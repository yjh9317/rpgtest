// BaseAttackAction.h

#pragma once

#include "CoreMinimal.h"
#include "Combat/Action/BaseAction.h"
#include "BaseAttackAction.generated.h"

class UCombatComponent;

UCLASS(Blueprintable)
class RPGSYSTEM_API UBaseAttackAction : public UBaseAction
{
	GENERATED_BODY()

public:
	UBaseAttackAction(const FObjectInitializer& ObjectInitializer);

protected:
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float BaseDamage = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	FGameplayTagContainer DamageTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float BaseStaminaCost = 10.f;

	UPROPERTY()
	TObjectPtr<UCombatComponent> CachedCombatComponent = nullptr;

public:
	virtual void Initialize(AActor* NewActionOwner, UObject* NewSourceObject = nullptr) override;
	virtual bool CanExecute() const override;

protected:
	UFUNCTION(BlueprintCallable, Category = "Attack")
	virtual float ApplyAttackToCurrentTarget();

	UCombatComponent* GetCombatComponent() const;

	// 스태미너/리소스 체크용
	virtual bool HasAttackResources() const;
	virtual void ConsumeAttackResources();
};

// UPROPERTY(BlueprintReadOnly, Category = "Attack|State")
// bool bHitboxActive = false;          // 히트박스 활성 창
//
// UPROPERTY(BlueprintReadOnly, Category = "Attack|State")
// bool bComboWindowOpen = false;       // 콤보 입력 창
//
// UPROPERTY(BlueprintReadOnly, Category = "Attack|State")
// bool bMovementLocked = false;        // 이동 잠금
//
// UPROPERTY(BlueprintReadOnly, Category = "Attack|State")
// bool bInvincible = false;           // 무적 프레임

// UFUNCTION(BlueprintPure, Category = "Attack|State")
// bool IsHitboxActive() const { return bHitboxActive; }
//
// UFUNCTION(BlueprintPure, Category = "Attack|State")
// bool IsComboWindowOpen() const { return bComboWindowOpen; }
//
// UFUNCTION(BlueprintPure, Category = "Attack|State")
// bool IsMovementLocked() const { return bMovementLocked; }
//
// UFUNCTION(BlueprintPure, Category = "Attack|State")
// bool IsInvincible() const { return bInvincible; }