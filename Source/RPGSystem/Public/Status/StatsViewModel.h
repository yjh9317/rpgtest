// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "StatsViewModel.generated.h"

class UStatsComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStatTextChanged, const FText& /* NewText */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStatPercentChanged, float /* NewPercent */);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnStatColorChanged, const FLinearColor& /* NewColor */);

UCLASS()
class RPGSYSTEM_API UStatsViewModel : public UObject
{
	GENERATED_BODY()
	
public:
	UStatsViewModel();
	
	/**
	 * ViewModel 초기화
	 * @param InStatsComponent 연결할 StatsComponent
	 */
	void InitializeStats(UStatsComponent* InStatsComponent);
	
	/**
	 * ViewModel 정리 및 델리게이트 해제
	 */
	void Cleanup();

#pragma region Health_Delegates
	/** Health 텍스트 변경 (예: "100 / 500") */
	FOnStatTextChanged OnHealthTextChanged;
	/** Health 백분율 변경 (0.0 ~ 1.0) */
	FOnStatPercentChanged OnHealthPercentChanged;
	/** Health 바 색상 변경 */
	FOnStatColorChanged OnHealthColorChanged;
#pragma endregion

#pragma region Mana_Delegates
	/** Mana 텍스트 변경 (예: "80 / 150") */
	FOnStatTextChanged OnManaTextChanged;
	/** Mana 백분율 변경 (0.0 ~ 1.0) */
	FOnStatPercentChanged OnManaPercentChanged;
	/** Mana 바 색상 변경 */
	FOnStatColorChanged OnManaColorChanged;
#pragma endregion

#pragma region Stamina_Delegates
	/** Stamina 텍스트 변경 (예: "50 / 100") */
	FOnStatTextChanged OnStaminaTextChanged;
	/** Stamina 백분율 변경 (0.0 ~ 1.0) */
	FOnStatPercentChanged OnStaminaPercentChanged;
	/** Stamina 바 색상 변경 */
	FOnStatColorChanged OnStaminaColorChanged;
#pragma endregion

#pragma region Experience_Delegates
	/** Experience 텍스트 변경 (예: "750 / 1000") */
	FOnStatTextChanged OnExperienceTextChanged;
	/** Experience 백분율 변경 (0.0 ~ 1.0) */
	FOnStatPercentChanged OnExperiencePercentChanged;
#pragma endregion

#pragma region Level_Delegates
	/** Level 텍스트 변경 (예: "Lv.15") */
	FOnStatTextChanged OnLevelChanged;
#pragma endregion

#pragma region Getters
	// Health
	FORCEINLINE FText GetHealthText() const { return HealthText; }
	FORCEINLINE float GetHealthPercent() const { return HealthPercent; }
	FORCEINLINE FLinearColor GetHealthColor() const { return HealthColor; }

	// Mana
	FORCEINLINE FText GetManaText() const { return ManaText; }
	FORCEINLINE float GetManaPercent() const { return ManaPercent; }
	FORCEINLINE FLinearColor GetManaColor() const { return ManaColor; }

	// Stamina
	FORCEINLINE FText GetStaminaText() const { return StaminaText; }
	FORCEINLINE float GetStaminaPercent() const { return StaminaPercent; }
	FORCEINLINE FLinearColor GetStaminaColor() const { return StaminaColor; }

	// Experience
	FORCEINLINE FText GetExperienceText() const { return ExperienceText; }
	FORCEINLINE float GetExperiencePercent() const { return ExperiencePercent; }

	// Level
	FORCEINLINE FText GetLevelText() const { return LevelText; }
#pragma endregion

protected:
	/** 연결된 StatsComponent */
	UPROPERTY(Transient)
	TObjectPtr<UStatsComponent> StatsComponent;

	/** Gameplay Tags */
	UPROPERTY(EditDefaultsOnly, Category="Tag")
	FGameplayTag HealthTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Tag")
	FGameplayTag ManaTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Tag")
	FGameplayTag StaminaTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Tag")
	FGameplayTag EXPTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Tag")
	FGameplayTag LevelTag;

	/** 색상 계산을 위한 임계값 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float GoodThreshold = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WarningThreshold = 0.3f;

	/** 캐시된 UI 데이터 */
	UPROPERTY(Transient)
	FText HealthText;

	UPROPERTY(Transient)
	float HealthPercent = 0.0f;

	UPROPERTY(Transient)
	FLinearColor HealthColor = FLinearColor::White;

	UPROPERTY(Transient)
	FText ManaText;

	UPROPERTY(Transient)
	float ManaPercent = 0.0f;

	UPROPERTY(Transient)
	FLinearColor ManaColor = FLinearColor::White;

	UPROPERTY(Transient)
	FText StaminaText;

	UPROPERTY(Transient)
	float StaminaPercent = 0.0f;

	UPROPERTY(Transient)
	FLinearColor StaminaColor = FLinearColor::White;

	UPROPERTY(Transient)
	FText ExperienceText;

	UPROPERTY(Transient)
	float ExperiencePercent = 0.0f;

	UPROPERTY(Transient)
	FText LevelText;

private:
	/**
	 * StatsComponent 콜백 함수들
	 */
	UFUNCTION()
	void OnStatChangedCallback(const FGameplayTag& StatTag, float NewValue);

	UFUNCTION()
	void OnStatModifiedCallback(const FGameplayTag& StatTag, float OldValue, float NewValue);
	
	/**
	 * 각 스탯별 디스플레이 업데이트 함수
	 */
	void UpdateHealthDisplay();
	void UpdateManaDisplay();
	void UpdateStaminaDisplay();
	void UpdateEXPDisplay();
	void UpdateLevelDisplay();

	/**
	 * 유틸리티 함수들
	 */
	FLinearColor CalculateStatBarColor(float Percent) const;
	FText FormatStatValueText(float CurrentValue, float MaxValue) const;
	FText FormatStatText(float Value) const;

	/**
	 * Setter 함수들 - 값이 변경된 경우에만 델리게이트 브로드캐스트
	 */
	void SetHealthText(const FText& NewValue);
	void SetHealthPercent(float NewValue);
	void SetHealthColor(const FLinearColor& NewValue);

	void SetManaText(const FText& NewValue);
	void SetManaPercent(float NewValue);
	void SetManaColor(const FLinearColor& NewValue);

	void SetStaminaText(const FText& NewValue);
	void SetStaminaPercent(float NewValue);
	void SetStaminaColor(const FLinearColor& NewValue);

	void SetExperienceText(const FText& NewValue);
	void SetExperiencePercent(float NewValue);

	void SetLevelText(const FText& NewValue);
};
