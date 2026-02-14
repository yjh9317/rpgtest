// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/Components/CombatComponentBase.h"
#include "Engine/DamageEvents.h"
#include "Status/StatsComponent.h"
#include "Status/Effects/EffectComponent.h"

UCombatComponentBase::UCombatComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.5f;  // 전투 타임아웃만 체크하므로 느린 틱
	HealthStatTag = FGameplayTag::RequestGameplayTag(TEXT("Character.Stats.Health"), false);
}

void UCombatComponentBase::BeginPlay()
{
	Super::BeginPlay();

	// StatsComponent 캐싱
	if (AActor* Owner = GetOwner())
	{
		CachedStatsComp = Owner->FindComponentByClass<UStatsComponent>();
	}
}

void UCombatComponentBase::TickComponent(float DeltaTime, ELevelTick TickType, 
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutoLeaveCombat)
	{
		CheckCombatTimeout(DeltaTime);
	}
}

// === ICombatable 구현 ===
float UCombatComponentBase::GetCurrentHealth() const
{
	if (UStatsComponent* Stats = GetStatsComponent())
	{
		if (HealthStatTag.IsValid() && Stats->HasStat(HealthStatTag))
		{
			return Stats->GetStatValue(HealthStatTag);
		}

		LogInvalidHealthStatTag(TEXT("GetCurrentHealth"));
	}
	return 100.f;
}

float UCombatComponentBase::GetMaxHealth() const
{
	if (UStatsComponent* Stats = GetStatsComponent())
	{
		if (HealthStatTag.IsValid() && Stats->HasStat(HealthStatTag))
		{
			return Stats->GetMaxStatValue(HealthStatTag);
		}

		LogInvalidHealthStatTag(TEXT("GetMaxHealth"));
	}
	return 100.f;
}

float UCombatComponentBase::ReceiveDamage(const FDamageInfo& DamageInfo)
{
	if (bIsDead)
	{
		return 0.f;
	}

	if (bIsInvulnerable && !DamageInfo.bBypassInvulnerability)
	{
		return 0.f;
	}

	OnPreDamageApplied.Broadcast(DamageInfo, DamageInfo.SourceActor.Get());

	// 최종 데미지 계산
	const float FinalDamage = CalculateFinalDamage(DamageInfo);
	if (FinalDamage <= 0.f)
	{
		OnPostDamageApplied.Broadcast(0.f, DamageInfo, DamageInfo.SourceActor.Get(), false);
		return 0.f;
	}

	float OldHealth = 0.f;
	float NewHealth = 0.f;
	bool bHealthChanged = false;

	if (UStatsComponent* Stats = GetStatsComponent())
	{
		if (HealthStatTag.IsValid() && Stats->HasStat(HealthStatTag))
		{
			OldHealth = Stats->GetStatValue(HealthStatTag);
			Stats->ModifyStatValue(HealthStatTag, -FinalDamage);
			NewHealth = Stats->GetStatValue(HealthStatTag);
			bHealthChanged = true;
		}
		else
		{
			LogInvalidHealthStatTag(TEXT("ReceiveDamage"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponentBase::ReceiveDamage - StatsComponent not found on %s"),
			*GetNameSafe(GetOwner()));
	}

	if (bHealthChanged)
	{
		OnHealthChangedFromCombat.Broadcast(OldHealth, NewHealth);
	}

	// 전투 상태 진입
	if (DamageInfo.SourceActor.IsValid())
	{
		EnterCombat(DamageInfo.SourceActor.Get());
	}

	ApplyDamageDrivenEffects(DamageInfo);

	// 이벤트 발송
	OnDamageReceived.Broadcast(FinalDamage, DamageInfo.SourceActor.Get(), DamageInfo);

	// 후처리
	PostDamageReceived(FinalDamage, DamageInfo.SourceActor.Get());

	const bool bKilledTarget = GetCurrentHealth() <= 0.f;
	if (bKilledTarget)
	{
		HandleDeath(DamageInfo.SourceActor.Get());
	}

	OnPostDamageApplied.Broadcast(FinalDamage, DamageInfo, DamageInfo.SourceActor.Get(), bKilledTarget);

	return FinalDamage;
}

float UCombatComponentBase::ApplyDamage(AActor* Target, const FDamageInfo& DamageInfo)
{
	if (!Target) return 0.f;

	float AppliedDamage = 0.f;

	// ICombatable 구현체에 데미지 적용
	if (ICombatable* Combatable = Cast<ICombatable>(Target))
	{
		AppliedDamage = Combatable->ReceiveDamage(DamageInfo);
	}
	// CombatComponentBase를 직접 찾아서 적용
	else if (UCombatComponentBase* TargetCombat = Target->FindComponentByClass<UCombatComponentBase>())
	{
		AppliedDamage = TargetCombat->ReceiveDamage(DamageInfo);
	}
	// Unreal 기본 데미지 시스템
	else
	{
		AppliedDamage = Target->TakeDamage(DamageInfo.BaseDamage, FDamageEvent(), nullptr, GetOwner());
	}

	if (AppliedDamage > 0.f)
	{
		EnterCombat(Target);
		OnDamageDealt.Broadcast(AppliedDamage, Target, DamageInfo);
	}

	return AppliedDamage;
}

void UCombatComponentBase::EnterCombat(AActor* Opponent)
{
	LastCombatTime = GetWorld()->GetTimeSeconds();
	if (Opponent)
	{
		TargetActor = Opponent;
	}

	if (!bIsInCombat)
	{
		bIsInCombat = true;
		OnCombatStateChanged.Broadcast(true);
	}
}

void UCombatComponentBase::LeaveCombat()
{
	if (bIsInCombat)
	{
		bIsInCombat = false;
		OnCombatStateChanged.Broadcast(false);
	}

	TargetActor = nullptr;
}

void UCombatComponentBase::SetInvulnerable(bool bNewInvulnerable)
{
	bIsInvulnerable = bNewInvulnerable;
}

float UCombatComponentBase::GetHealthPercent() const
{
	const float MaxHP = GetMaxHealth();
	if (MaxHP <= 0.f) return 0.f;
	return GetCurrentHealth() / MaxHP;
}

AActor* UCombatComponentBase::GetCurrentTarget() const
{
	return TargetActor;
}

// === 오버라이드 포인트 ===
float UCombatComponentBase::CalculateFinalDamage_Implementation(const FDamageInfo& DamageInfo)
{
	float Damage = DamageInfo.BaseDamage;

	// TODO: 방어력, 저항 계산
	// 기본은 원본 데미지 그대로

	return FMath::Max(0.f, Damage);
}

void UCombatComponentBase::PostDamageReceived_Implementation(float FinalDamage, AActor* Instigator)
{
	// 기본 구현: 아무것도 안 함
	// Player: 히트 이펙트, 카메라 흔들림
	// AI: 히트 리액션, 어그로 추가
}

void UCombatComponentBase::HandleDeath_Implementation(AActor* Killer)
{
	if (bIsDead) return;

	bIsDead = true;
	LeaveCombat();

	OnCombatDeath.Broadcast(Killer);
}

void UCombatComponentBase::CheckCombatTimeout(float DeltaTime)
{
	if (!bIsInCombat || bIsDead) return;

	const float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastCombatTime >= CombatTimeout)
	{
		LeaveCombat();
	}
}

void UCombatComponentBase::ApplyDamageDrivenEffects(const FDamageInfo& DamageInfo)
{
	if (DamageEffectRules.IsEmpty())
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	UEffectComponent* EffectComponent = OwnerActor->FindComponentByClass<UEffectComponent>();
	if (!EffectComponent)
	{
		return;
	}

	for (const FDamageEffectRule& Rule : DamageEffectRules)
	{
		if (!Rule.EffectToApply)
		{
			continue;
		}

		if (Rule.RequiredDamageTypeTag.IsValid() &&
			!DamageInfo.DamageTypeTag.MatchesTag(Rule.RequiredDamageTypeTag))
		{
			continue;
		}

		if (!Rule.RequiredDamageTags.IsEmpty() &&
			!DamageInfo.DamageTags.HasAny(Rule.RequiredDamageTags))
		{
			continue;
		}

		FEffectContext EffectContext;
		EffectContext.SourceActor = DamageInfo.SourceActor;
		EffectContext.TargetActor = OwnerActor;
		EffectContext.SourceLocation = DamageInfo.SourceActor.IsValid()
			? DamageInfo.SourceActor->GetActorLocation()
			: OwnerActor->GetActorLocation();
		EffectContext.ApplicationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

		EffectComponent->ApplyEffect(Rule.EffectToApply, EffectContext);
	}
}

UStatsComponent* UCombatComponentBase::GetStatsComponent() const
{
	if (CachedStatsComp.IsValid())
	{
		return CachedStatsComp.Get();
	}

	if (AActor* Owner = GetOwner())
	{
		UCombatComponentBase* MutableThis = const_cast<UCombatComponentBase*>(this);
		MutableThis->CachedStatsComp = Owner->FindComponentByClass<UStatsComponent>();
		return MutableThis->CachedStatsComp.Get();
	}

	return nullptr;
}

void UCombatComponentBase::LogInvalidHealthStatTag(const TCHAR* CallerName) const
{
	UE_LOG(LogTemp, Warning,
		TEXT("UCombatComponentBase::%s - Invalid or missing HealthStatTag '%s' on %s"),
		CallerName,
		*HealthStatTag.ToString(),
		*GetNameSafe(GetOwner()));
}




