// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatData.h"
#include "UObject/Interface.h"
#include "Combatable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UCombatable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API ICombatable
{
	GENERATED_BODY()

public:
	// === 필수 구현 ===
	virtual bool IsAlive() const = 0;
	virtual float GetCurrentHealth() const = 0;
	virtual float GetMaxHealth() const = 0;

	// === 데미지 처리 ===
	virtual float ReceiveDamage(const FDamageInfo& DamageInfo) = 0;
	virtual float ApplyDamage(AActor* Target, const FDamageInfo& DamageInfo) = 0;

	// === 선택적 구현 (기본 구현 제공) ===
	virtual bool CanReceiveDamage() const { return IsAlive(); }
	virtual bool IsInvulnerable() const { return false; }
	virtual float GetDamageMultiplier() const { return 1.f; }

	// === 전투 상태 ===
	virtual void EnterCombat(AActor* Opponent) {}
	virtual void LeaveCombat() {}
	virtual bool IsInCombat() const { return false; }
};
