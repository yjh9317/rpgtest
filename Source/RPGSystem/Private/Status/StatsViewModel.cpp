// Fill out your copyright notice in the Description page of Project Settings.


#include "Status/StatsViewModel.h"
#include "Status/StatsComponent.h"

UStatsViewModel::UStatsViewModel()
{
}

void UStatsViewModel::Initialize(UStatsComponent* InStatsComponent)
{
	if (!InStatsComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UStatsViewModel::Initialize - StatsComponent is nullptr"));
		return;
	}

	StatsComponent = InStatsComponent;

	StatsComponent->OnStatChanged.AddDynamic(this, &UStatsViewModel::OnStatChangedCallback);
	StatsComponent->OnStatModified.AddDynamic(this, &UStatsViewModel::OnStatModifiedCallback);

	UpdateHealthDisplay();
	UpdateManaDisplay();
	UpdateStaminaDisplay();
	UpdateEXPDisplay();
	UpdateLevelDisplay();
}

void UStatsViewModel::Cleanup()
{
	if (StatsComponent)
	{
		StatsComponent->OnStatChanged.RemoveDynamic(this, &UStatsViewModel::OnStatChangedCallback);
		StatsComponent->OnStatModified.RemoveDynamic(this, &UStatsViewModel::OnStatModifiedCallback);
		StatsComponent = nullptr;
	}

	OnHealthTextChanged.Clear();
	OnHealthPercentChanged.Clear();
	OnHealthColorChanged.Clear();
	
	OnManaTextChanged.Clear();
	OnManaPercentChanged.Clear();
	OnManaColorChanged.Clear();
	
	OnStaminaTextChanged.Clear();
	OnStaminaPercentChanged.Clear();
	OnStaminaColorChanged.Clear();
	
	OnExperienceTextChanged.Clear();
	OnExperiencePercentChanged.Clear();
	
	OnLevelChanged.Clear();
}

#pragma region Callback_Functions

void UStatsViewModel::OnStatChangedCallback(const FGameplayTag& StatTag, float NewValue)
{
	if (StatTag.MatchesTag(HealthTag))
	{
		UpdateHealthDisplay();
	}
	else if (StatTag.MatchesTag(ManaTag))
	{
		UpdateManaDisplay();
	}
	else if (StatTag.MatchesTag(StaminaTag))
	{
		UpdateStaminaDisplay();
	}
	else if (StatTag.MatchesTag(EXPTag))
	{
		UpdateEXPDisplay();
	}
	else if (StatTag.MatchesTag(LevelTag))
	{
		UpdateLevelDisplay();
	}
}

void UStatsViewModel::OnStatModifiedCallback(const FGameplayTag& StatTag, float OldValue, float NewValue)
{
	// Health 변경 시 추가 효과 처리
	if (StatTag.MatchesTag(HealthTag))
	{
		// 데미지를 받았을 때
		if (NewValue < OldValue)
		{
			// TODO: 데미지 효과 트리거 (화면 흔들림, 피격 이펙트 등)
			UE_LOG(LogTemp, Log, TEXT("Health Damaged: %f -> %f"), OldValue, NewValue);
		}
		// 회복했을 때
		else if (NewValue > OldValue)
		{
			// TODO: 회복 효과 트리거 (힐 이펙트 등)
			UE_LOG(LogTemp, Log, TEXT("Health Restored: %f -> %f"), OldValue, NewValue);
		}
	}
}

#pragma endregion

#pragma region Update_Display_Functions

void UStatsViewModel::UpdateHealthDisplay()
{
	if (!StatsComponent) return;

	const float CurrentHP = StatsComponent->GetStatValue(HealthTag);
	const float MaxHP = StatsComponent->GetMaxStatValue(HealthTag);
	const float Percent = StatsComponent->GetStatPercentage(HealthTag);

	// "100 / 500" 형태로 포맷팅
	SetHealthText(FormatStatValueText(CurrentHP, MaxHP));
	SetHealthPercent(Percent);
	SetHealthColor(CalculateStatBarColor(Percent));
}

void UStatsViewModel::UpdateManaDisplay()
{
	if (!StatsComponent) return;

	const float CurrentMana = StatsComponent->GetStatValue(ManaTag);
	const float MaxMana = StatsComponent->GetMaxStatValue(ManaTag);
	const float Percent = StatsComponent->GetStatPercentage(ManaTag);

	SetManaText(FormatStatValueText(CurrentMana, MaxMana));
	SetManaPercent(Percent);
	// Mana는 고정 색상 사용 (파란색 계열)
	SetManaColor(FLinearColor(0.1f, 0.4f, 1.0f));
}

void UStatsViewModel::UpdateStaminaDisplay()
{
	if (!StatsComponent) return;

	const float CurrentStamina = StatsComponent->GetStatValue(StaminaTag);
	const float MaxStamina = StatsComponent->GetMaxStatValue(StaminaTag);
	const float Percent = StatsComponent->GetStatPercentage(StaminaTag);

	SetStaminaText(FormatStatValueText(CurrentStamina, MaxStamina));
	SetStaminaPercent(Percent);
	// Stamina는 고정 색상 사용 (노란색 계열)
	SetStaminaColor(FLinearColor(1.0f, 0.9f, 0.2f));
}

void UStatsViewModel::UpdateEXPDisplay()
{
	if (!StatsComponent) return;

	const float CurrentEXP = StatsComponent->GetStatValue(EXPTag);
	const float MaxEXP = StatsComponent->GetMaxStatValue(EXPTag);
	const float Percent = StatsComponent->GetStatPercentage(EXPTag);

	SetExperienceText(FormatStatValueText(CurrentEXP, MaxEXP));
	SetExperiencePercent(Percent);
}

void UStatsViewModel::UpdateLevelDisplay()
{
	if (!StatsComponent) return;

	const float Level = StatsComponent->GetStatValue(LevelTag);
	
	SetLevelText(FText::Format(
		FText::FromString(TEXT("Lv.{0}")),
		FText::AsNumber(FMath::RoundToInt(Level))
	));
}

#pragma endregion

#pragma region Utility_Functions

FLinearColor UStatsViewModel::CalculateStatBarColor(float Percent) const
{
	// 백분율에 따라 색상 결정
	if (Percent > GoodThreshold)
	{
		return FLinearColor::Green;
	}
	else if (Percent > WarningThreshold)
	{
		return FLinearColor::Yellow;
	}
	else
	{
		return FLinearColor::Red;
	}
}

FText UStatsViewModel::FormatStatValueText(float CurrentValue, float MaxValue) const
{
	// "100 / 500" 형식으로 포맷팅
	return FText::Format(
		FText::FromString(TEXT("{0} / {1}")),
		FText::AsNumber(FMath::RoundToInt(CurrentValue)),
		FText::AsNumber(FMath::RoundToInt(MaxValue))
	);
}

FText UStatsViewModel::FormatStatText(float Value) const
{
	return FText::AsNumber(FMath::RoundToInt(Value));
}

#pragma endregion

#pragma region Setter_Functions

void UStatsViewModel::SetHealthText(const FText& NewValue)
{
	if (!HealthText.EqualTo(NewValue))
	{
		HealthText = NewValue;
		OnHealthTextChanged.Broadcast(HealthText);
	}
}

void UStatsViewModel::SetHealthPercent(float NewValue)
{
	// 부동소수점 비교 시 작은 오차 허용
	if (!FMath::IsNearlyEqual(HealthPercent, NewValue, 0.001f))
	{
		HealthPercent = NewValue;
		OnHealthPercentChanged.Broadcast(HealthPercent);
	}
}

void UStatsViewModel::SetHealthColor(const FLinearColor& NewValue)
{
	if (HealthColor != NewValue)
	{
		HealthColor = NewValue;
		OnHealthColorChanged.Broadcast(HealthColor);
	}
}

void UStatsViewModel::SetManaText(const FText& NewValue)
{
	if (!ManaText.EqualTo(NewValue))
	{
		ManaText = NewValue;
		OnManaTextChanged.Broadcast(ManaText);
	}
}

void UStatsViewModel::SetManaPercent(float NewValue)
{
	if (!FMath::IsNearlyEqual(ManaPercent, NewValue, 0.001f))
	{
		ManaPercent = NewValue;
		OnManaPercentChanged.Broadcast(ManaPercent);
	}
}

void UStatsViewModel::SetManaColor(const FLinearColor& NewValue)
{
	if (ManaColor != NewValue)
	{
		ManaColor = NewValue;
		OnManaColorChanged.Broadcast(ManaColor);
	}
}

void UStatsViewModel::SetStaminaText(const FText& NewValue)
{
	if (!StaminaText.EqualTo(NewValue))
	{
		StaminaText = NewValue;
		OnStaminaTextChanged.Broadcast(StaminaText);
	}
}

void UStatsViewModel::SetStaminaPercent(float NewValue)
{
	if (!FMath::IsNearlyEqual(StaminaPercent, NewValue, 0.001f))
	{
		StaminaPercent = NewValue;
		OnStaminaPercentChanged.Broadcast(StaminaPercent);
	}
}

void UStatsViewModel::SetStaminaColor(const FLinearColor& NewValue)
{
	if (StaminaColor != NewValue)
	{
		StaminaColor = NewValue;
		OnStaminaColorChanged.Broadcast(StaminaColor);
	}
}

void UStatsViewModel::SetExperienceText(const FText& NewValue)
{
	if (!ExperienceText.EqualTo(NewValue))
	{
		ExperienceText = NewValue;
		OnExperienceTextChanged.Broadcast(ExperienceText);
	}
}

void UStatsViewModel::SetExperiencePercent(float NewValue)
{
	if (!FMath::IsNearlyEqual(ExperiencePercent, NewValue, 0.001f))
	{
		ExperiencePercent = NewValue;
		OnExperiencePercentChanged.Broadcast(ExperiencePercent);
	}
}

void UStatsViewModel::SetLevelText(const FText& NewValue)
{
	if (!LevelText.EqualTo(NewValue))
	{
		LevelText = NewValue;
		OnLevelChanged.Broadcast(LevelText);
	}
}

#pragma endregion