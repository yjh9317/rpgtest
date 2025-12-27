// Fill out your copyright notice in the Description page of Project Settings.


#include "Status/StatsComponent.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Status/DataAsset_StatConfig.h"


UStatsComponent::UStatsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	
	SetIsReplicatedByDefault(true);
}

void FStatArray::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (OwnerComponent)
	{
		OwnerComponent->OnStatsReplicated();
	}
}

void FStatArray::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (OwnerComponent)
	{
		OwnerComponent->OnStatsReplicated();
	}
}

void UStatsComponent::OnStatsReplicated()
{
	RebuildStatCache();
	OnStatChanged.Broadcast(FGameplayTag(), 0.0f);
}


void UStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatsComponent, StatArray);
}

void UStatsComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (GetOwnerRole() == ROLE_Authority)
	{
		InitializeStats();
	}
	
	RebuildStatCache();
}



void UStatsComponent::BeginDestroy()
{
    OnStatChanged.Clear();
    OnStatModified.Clear();
    OnStatMaxValueChanged.Clear();
    RefreshStats.Clear();

    // 모든 타이머 정리
    if (GetWorld())
    {
        FTimerManager& TimerManager = GetWorld()->GetTimerManager();
        
        // 재계산 타이머
        TimerManager.ClearTimer(StatRecalculationTimer);
        
        // 모든 Modifier 타이머 정리
        for (auto& Pair : ModifierTimers)
        {
            TimerManager.ClearTimer(Pair.Value);
        }
        ModifierTimers.Empty();
    }

    // 캐시 정리
    StatIndexCache.Empty();
    
    Super::BeginDestroy();
}

void UStatsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 스탯 재계산이 필요한 경우
	if (bNeedsStatRecalculation)
	{
		// RecalculateAllStats();
		bNeedsStatRecalculation = false;
	}
}

#pragma region Core_Stat_Functions

float UStatsComponent::GetBaseStatValue(const FGameplayTag& StatTag) const
{
	if (const int32* Index = StatIndexCache.Find(StatTag))
	{
		return StatArray.Items[*Index].BaseValue;
	}
	return 0.0f;
}

float UStatsComponent::GetStatValue(const FGameplayTag& StatTag) const
{
	if (const int32* Index = StatIndexCache.Find(StatTag))
	{
		return StatArray.Items[*Index].CurrentValue;
	}
	return 0.0f;
}

float UStatsComponent::GetMaxStatValue(const FGameplayTag& StatTag) const
{
	if (const int32* Index = StatIndexCache.Find(StatTag))
	{
		return StatArray.Items[*Index].MaxValue;
	}
	return 0.0f;
}

float UStatsComponent::GetStatPercentage(const FGameplayTag& StatTag) const
{
	float CurrentValue = GetStatValue(StatTag);
	float MaxValue = GetMaxStatValue(StatTag);
	
	if (MaxValue <= 0.0f)
	{
		return 0.0f;
	}
	
	return FMath::Clamp(CurrentValue / MaxValue, 0.0f, 1.0f);
}

void UStatsComponent::SetStatValue(const FGameplayTag& StatTag, float Value)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	if (!StatTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SetStatValue called with invalid StatTag"));
		return;
	}

	if (int32* Index = StatIndexCache.Find(StatTag))
	{
		FStatEntry& Entry = StatArray.Items[*Index];
		float OldValue = Entry.CurrentValue;
		float NewValue = ClampStatValue(StatTag, Value);
        
		// 값이 실제로 변경된 경우에만 업데이트
		if (!FMath::IsNearlyEqual(OldValue, NewValue, 0.01f))
		{
			Entry.CurrentValue = NewValue;
			StatArray.MarkItemDirty(Entry);
            
			// 두 델리게이트 모두 브로드캐스트
			OnStatChanged.Broadcast(StatTag, NewValue);
			OnStatModified.Broadcast(StatTag, OldValue, NewValue);
            
			UE_LOG(LogTemp, VeryVerbose, TEXT("Stat Updated - Tag: %s, Old: %.2f, New: %.2f"),
				*StatTag.ToString(), OldValue, NewValue);
		}
	}
	// 새 스탯 추가
	else
	{
		float ClampedValue = FMath::Max(0.0f, Value);
		FStatEntry NewEntry(StatTag, ClampedValue, ClampedValue);
        
		int32 NewIndex = StatArray.Items.Add(NewEntry);
		StatIndexCache.Add(StatTag, NewIndex);
		StatArray.MarkArrayDirty();
        
		// 새로 추가된 스탯은 OnStatChanged만 브로드캐스트
		OnStatChanged.Broadcast(StatTag, ClampedValue);
        
		UE_LOG(LogTemp, Log, TEXT("New Stat Added - Tag: %s, Value: %.2f"), 
			*StatTag.ToString(), ClampedValue);
	}
}

void UStatsComponent::SetMaxStatValue(const FGameplayTag& StatTag, float MaxValue)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	int32* Index = StatIndexCache.Find(StatTag);
	if (!Index)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetMaxStatValue: Stat not found - %s"), 
			*StatTag.ToString());
		return;
	}

	FStatEntry& Entry = StatArray.Items[*Index];
	float OldValue = Entry.CurrentValue;
	Entry.MaxValue = MaxValue;
    
	// CurrentValue가 MaxValue를 초과하면 조정
	if (Entry.CurrentValue > Entry.MaxValue)
	{
		Entry.CurrentValue = Entry.MaxValue;
        
		StatArray.MarkItemDirty(Entry);
		OnStatMaxValueChanged.Broadcast(StatTag);
        
		// CurrentValue가 변경되었으므로 알림
		OnStatChanged.Broadcast(StatTag, Entry.CurrentValue);
		OnStatModified.Broadcast(StatTag, OldValue, Entry.CurrentValue);
	}
	else
	{
		StatArray.MarkItemDirty(Entry);
		OnStatMaxValueChanged.Broadcast(StatTag);
	}
}

bool UStatsComponent::HasStat(const FGameplayTag& StatTag) const
{
	return StatIndexCache.Contains(StatTag);
}

TArray<FGameplayTag> UStatsComponent::GetAllStatTags() const
{
	TArray<FGameplayTag> Result;
	StatIndexCache.GetKeys(Result);
	return Result;
}


#pragma endregion


void UStatsComponent::InitializeStats()
{
	checkf(DataAsset_StatConfig,TEXT("UStatsComponent::InitializeStats : DataAsset_StatConfig is not valid "));
	
	StatArray.OwnerComponent = this;
		
	for (const auto& Config : DataAsset_StatConfig->StatConfigs)
	{
		const float Base = FMath::Max(0.0f, Config.BaseValue);
		const float Max = (Config.MaxValue > 0.0f) ? Config.MaxValue : Base;
		FStatEntry NewEntry(Config.StatTag, Base, Max);
		int32 NewIndex = StatArray.Items.Add(NewEntry);
		StatIndexCache.Add(Config.StatTag, NewIndex);
	}
    
	StatArray.MarkArrayDirty();
	RefreshStats.Broadcast();
}


#pragma region Internal_Functions

float UStatsComponent::ClampStatValue(const FGameplayTag& StatTag, float Value) const
{
	float MaxValue = GetMaxStatValue(StatTag);
	return FMath::Clamp(Value, 0, MaxValue);
}

void UStatsComponent::OnStatValueChanged(const FGameplayTag& StatTag, float OldValue, float NewValue)
{
	if (!FMath::IsNearlyEqual(OldValue, NewValue, 0.01f))
	{
		OnStatChanged.Broadcast(StatTag, NewValue);
		OnStatModified.Broadcast(StatTag, OldValue, NewValue);
	}
}

void UStatsComponent::RebuildStatCache()
{
	StatIndexCache.Empty();
    
	for (int32 i = 0; i < StatArray.Items.Num(); ++i)
	{
		StatIndexCache.Add(StatArray.Items[i].StatTag, i);
	}
}

#pragma endregion

void UStatsComponent::AddStatModifier(const FGameplayTag& StatTag, const FStatModifier& Modifier)
{
	if (GetOwnerRole() != ROLE_Authority) return;
	if (!StatTag.IsValid() || !Modifier.ModifierTag.IsValid()) return;

	int32* Index = StatIndexCache.Find(StatTag);
	if (!Index) return;

	FStatEntry& Entry = StatArray.Items[*Index];

	Entry.Modifiers.RemoveAll([&](const FStatModifier& Mod)
	{
		return Mod.ModifierTag == Modifier.ModifierTag;
	});

	FStatModifier NewModifier = Modifier;
	NewModifier.StartTime = GetWorld()->GetTimeSeconds();
	Entry.Modifiers.Add(NewModifier);

	Entry.RecalculateValue();
    
	// 네트워크 복제
	StatArray.MarkItemDirty(Entry);

	if (Modifier.Duration > 0.0f)
	{
		SetupModifierTimer(StatTag, Modifier.Duration);
	}
	
	OnStatModified.Broadcast(StatTag, Entry.BaseValue, Entry.CurrentValue);
}

void UStatsComponent::RemoveStatModifier(const FGameplayTag& StatTag, const FGameplayTag& ModifierTag)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	int32* Index = StatIndexCache.Find(StatTag);
	if (!Index) return;

	FStatEntry& Entry = StatArray.Items[*Index];
	float OldValue = Entry.CurrentValue;

	int32 RemovedCount = Entry.Modifiers.RemoveAll([&](const FStatModifier& Mod)
	{
		return Mod.ModifierTag == ModifierTag;
	});

	if (RemovedCount > 0)
	{
		Entry.RecalculateValue();
		StatArray.MarkItemDirty(Entry);
		OnStatModified.Broadcast(StatTag, OldValue, Entry.CurrentValue);
	}
}

void UStatsComponent::RemoveAllModifiers(const FGameplayTag& StatTag)
{
	if (GetOwnerRole() != ROLE_Authority) return;

	int32* Index = StatIndexCache.Find(StatTag);
	if (!Index) return;

	FStatEntry& Entry = StatArray.Items[*Index];
    
	if (Entry.Modifiers.Num() > 0)
	{	
		float OldValue = Entry.CurrentValue;
		Entry.Modifiers.Empty();
		Entry.RecalculateValue();
		StatArray.MarkItemDirty(Entry);
		OnStatModified.Broadcast(StatTag, OldValue, Entry.CurrentValue);
	}

	// 타이머 정리
	if (FTimerHandle* Timer = ModifierTimers.Find(StatTag))
	{
		GetWorld()->GetTimerManager().ClearTimer(*Timer);
		ModifierTimers.Remove(StatTag);
	}
}

TArray<FStatModifier> UStatsComponent::GetStatModifiers(const FGameplayTag& StatTag) const
{
	if (const int32* Index = StatIndexCache.Find(StatTag))
	{
		return StatArray.Items[*Index].Modifiers;
	}
	return TArray<FStatModifier>();
}

void UStatsComponent::AddItemStatBonus(const FGameplayTag& StatTag, const FGameplayTag& ItemID, float BonusValue,
	EStatModifierType ModifierType)
{
	FStatModifier Modifier(ItemID, BonusValue, ModifierType, -1.0f);  // 영구
	AddStatModifier(StatTag, Modifier);
}

void UStatsComponent::RemoveItemStatBonus(const FGameplayTag& StatTag, const FGameplayTag& ItemID)
{
	RemoveStatModifier(StatTag, ItemID);
}

void UStatsComponent::AddTemporaryStatBuff(const FGameplayTag& StatTag, const FGameplayTag& BuffID, float BonusValue,
	float Duration, EStatModifierType ModifierType)
{
	FStatModifier Modifier(BuffID, BonusValue, ModifierType, Duration);
	AddStatModifier(StatTag, Modifier);
}

void UStatsComponent::RemoveStatBuff(const FGameplayTag& StatTag, const FGameplayTag& BuffID)
{
	RemoveStatModifier(StatTag, BuffID);
}


void UStatsComponent::SetupModifierTimer(const FGameplayTag& StatTag, float Duration)
{
	if (FTimerHandle* ExistingTimer = ModifierTimers.Find(StatTag))
	{
		GetWorld()->GetTimerManager().ClearTimer(*ExistingTimer);
	}

	FTimerHandle NewTimer;
	GetWorld()->GetTimerManager().SetTimer(
		NewTimer,
		FTimerDelegate::CreateUObject(this, &UStatsComponent::OnModifierExpired, StatTag),
		Duration,
		false
	);

	ModifierTimers.Add(StatTag, NewTimer);
}

void UStatsComponent::OnModifierExpired(FGameplayTag StatTag)
{
	CleanupExpiredModifiers();
}

void UStatsComponent::CleanupExpiredModifiers()
{
	if (GetOwnerRole() != ROLE_Authority) return;

	float CurrentTime = GetWorld()->GetTimeSeconds();

	for (FStatEntry& Entry : StatArray.Items)
	{
		int32 RemovedCount = Entry.Modifiers.RemoveAll([CurrentTime](const FStatModifier& Mod)
		{
			return Mod.IsExpired(CurrentTime);
		});

		if (RemovedCount > 0)
		{
			float OldValue = Entry.CurrentValue;
			Entry.RecalculateValue();
			StatArray.MarkItemDirty(Entry);
			OnStatModified.Broadcast(Entry.StatTag, OldValue, Entry.CurrentValue);
		}
	}
}


float UStatsComponent::ModifyStatValue(const FGameplayTag& StatTag, float Delta)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("ModifyStatValue called on non-authority. Tag: %s"), 
			*StatTag.ToString());
		return 0.0f;
	}

	if (!StatTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ModifyStatValue called with invalid StatTag"));
		return 0.0f;
	}

	float CurrentValue = GetStatValue(StatTag);
	float MaxValue = GetMaxStatValue(StatTag);
	float NewValue = CurrentValue + Delta;
	float ClampedValue = FMath::Clamp(NewValue, 0.0f, MaxValue);
	
	SetStatValue(StatTag, ClampedValue);
	float ActualDelta = ClampedValue - CurrentValue;

	// 로그 (Verbose는 성능에 영향 없음)
	UE_LOG(LogTemp, VeryVerbose, TEXT("ModifyStatValue - Tag: %s, Delta: %.2f, Actual: %.2f, Current: %.2f -> %.2f"), 
		*StatTag.ToString(), Delta, ActualDelta, CurrentValue, ClampedValue);

	return ActualDelta;
}

float UStatsComponent::ModifyStatByPercentage(const FGameplayTag& StatTag, float Percentage)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("ModifyStatByPercentage called on non-authority. Tag: %s"), 
			*StatTag.ToString());
		return 0.0f;
	}

	if (!StatTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ModifyStatByPercentage called with invalid StatTag"));
		return 0.0f;
	}

	// 최대값 기준으로 변경량 계산
	float MaxValue = GetMaxStatValue(StatTag);
	float Delta = MaxValue * Percentage;

	float ActualDelta = ModifyStatValue(StatTag, Delta);

	UE_LOG(LogTemp, Verbose, TEXT("ModifyStatByPercentage - Tag: %s, Percentage: %.2f%%, MaxValue: %.2f, Delta: %.2f, Actual: %.2f"), 
		*StatTag.ToString(), Percentage * 100.0f, MaxValue, Delta, ActualDelta);

	return ActualDelta;
}

void UStatsComponent::RestoreStatToMax(const FGameplayTag& StatTag)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("RestoreStatToMax called on non-authority. Tag: %s"), 
			*StatTag.ToString());
		return;
	}

	if (!StatTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("RestoreStatToMax called with invalid StatTag"));
		return;
	}

	float MaxValue = GetMaxStatValue(StatTag);
	
	if (MaxValue <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("RestoreStatToMax - MaxValue is 0 or negative. Tag: %s"), 
			*StatTag.ToString());
		return;
	}

	SetStatValue(StatTag, MaxValue);

	UE_LOG(LogTemp, Log, TEXT("RestoreStatToMax - Tag: %s restored to %.2f"), 
		*StatTag.ToString(), MaxValue);
}

void UStatsComponent::RestoreMultipleStatsToMax(const TArray<FGameplayTag>& StatTags)
{
	if (GetOwnerRole() != ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("RestoreMultipleStatsToMax called on non-authority"));
		return;
	}

	for (const FGameplayTag& StatTag : StatTags)
	{
		if (StatTag.IsValid())
		{
			RestoreStatToMax(StatTag);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("RestoreMultipleStatsToMax - Restored %d stats"), StatTags.Num());
}

bool UStatsComponent::IsStatBelowThreshold(const FGameplayTag& StatTag, float Threshold) const
{
	if (!StatTag.IsValid())
	{
		return false;
	}

	float Percentage = GetStatPercentage(StatTag);
	return Percentage <= Threshold;
}

bool UStatsComponent::IsStatFull(const FGameplayTag& StatTag) const
{
	if (!StatTag.IsValid())
	{
		return false;
	}

	float CurrentValue = GetStatValue(StatTag);
	float MaxValue = GetMaxStatValue(StatTag);
	
	return FMath::IsNearlyEqual(CurrentValue, MaxValue, 0.01f);
}

bool UStatsComponent::IsStatEmpty(const FGameplayTag& StatTag) const
{
	if (!StatTag.IsValid())
	{
		return false;
	}

	float CurrentValue = GetStatValue(StatTag);
	return FMath::IsNearlyZero(CurrentValue, 0.01f);
}
