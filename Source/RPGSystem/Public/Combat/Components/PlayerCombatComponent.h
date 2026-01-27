// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatComponentBase.h"
#include "PlayerCombatComponent.generated.h"


UENUM(BlueprintType)
enum class ECombatStance : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),      // 맨손
	Melee       UMETA(DisplayName = "Melee"),        // 근접 무기 (검, 도끼 등)
	Aiming      UMETA(DisplayName = "Aiming"),       // 원거리 조준 (활, 총)
	Blocking    UMETA(DisplayName = "Blocking"),     // 방어 자세
	Charging    UMETA(DisplayName = "Charging"),     // 차징 공격 준비 중
	Casting     UMETA(DisplayName = "Casting")       // 마법 시전 중
};
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UPlayerCombatComponent : public UCombatComponentBase
{
	GENERATED_BODY()
public:
	UPlayerCombatComponent();

protected:
	// === Player 전용 설정 ===
	UPROPERTY(EditDefaultsOnly, Category = "Player|Combat")
	float InvincibilityDuration = 0.5f;  // 피격 후 무적 시간

	UPROPERTY(EditDefaultsOnly, Category = "Player|Combat")
	TSubclassOf<UCameraShakeBase> HitCameraShake;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Combat")
	float ControllerVibrationIntensity = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Combat")
	bool bEnableAutoRecovery = false;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Combat", meta = (EditCondition = "bEnableAutoRecovery"))
	float AutoRecoveryDelay = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "Player|Combat", meta = (EditCondition = "bEnableAutoRecovery"))
	float AutoRecoveryRate = 5.f;  // HP/초

	// === 런타임 ===
	FTimerHandle InvincibilityTimerHandle;
	FTimerHandle AutoRecoveryTimerHandle;
	float LastDamageTime = 0.f;

public:
	// === Player 전용 기능 ===
	UFUNCTION(BlueprintCallable, Category = "Player|Combat")
	void TriggerInvincibility(float Duration = -1.f);

	UFUNCTION(BlueprintPure, Category = "Player|Combat")
	bool IsInvincibilityActive() const;

	UFUNCTION(BlueprintCallable, Category = "Player|Combat")
	void StartAutoRecovery();

	UFUNCTION(BlueprintCallable, Category = "Player|Combat")
	void StopAutoRecovery();

protected:
	virtual void PostDamageReceived_Implementation(float FinalDamage, AActor* Instigator) override;
	virtual void HandleDeath_Implementation(AActor* Killer) override;

	void ApplyHitFeedback(float Damage);
	void OnInvincibilityEnd();
	void PerformAutoRecovery();
};
