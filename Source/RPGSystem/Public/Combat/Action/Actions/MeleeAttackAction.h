// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Action/Actions/BaseAttackAction.h"
#include "MeleeAttackAction.generated.h"

/**
 * 
 */

class UCommandBufferComponent;

USTRUCT(BlueprintType)
struct FMeleeAttackData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TArray<UAnimMontage*> ComboMontages;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	bool bCanMoveWhileAttacking = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float MovementSpeedWhileAttacking = 0.3f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	float ComboTimeoutDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	float InputQueueDuration = 0.3f;

	bool IsValid() const { return ComboMontages.Num() > 0; }
};


UCLASS()
class RPGSYSTEM_API UMeleeAttackAction : public UBaseAttackAction
{
    GENERATED_BODY()

public:
    UMeleeAttackAction(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee Attack")
    FMeleeAttackData MeleeData;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Combo State")
    int32 CurrentComboIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Combo State")
    bool bComboWindowOpen = false;

    UPROPERTY(BlueprintReadOnly, Category = "Combo State")
    bool bMovementCancelable = false;

    float LastAttackTime = 0.0f;
    float BlendOutTime = 0.2f;

    UPROPERTY()
    TArray<AActor*> AlreadyHitActors;

    UPROPERTY()
    TObjectPtr<UAnimMontage> CurrentAnimMontage;

    UPROPERTY()
    TObjectPtr<UCommandBufferComponent> CachedCommandBuffer;

public:
    // 콤보 윈도우
    void SetComboWindowOpen(bool bOpen) { bComboWindowOpen = bOpen; }
    bool IsComboWindowOpen() const { return bComboWindowOpen; }
    void OnComboWindowClosed();
    void OnAttackMontageEnded();

	virtual bool ProcessInput() override;
    // 이동 캔슬
    void CancelToMovement(const FVector& Direction);

    // 콤보 처리
    UFUNCTION(BlueprintCallable, Category = "Combo")
    bool TryProcessComboInput();

    UFUNCTION(BlueprintPure, Category = "Combat")
    int32 GetCurrentComboIndex() const { return CurrentComboIndex; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    int32 GetMaxComboCount() const { return MeleeData.ComboMontages.Num(); }

protected:
    virtual void Initialize(AActor* NewActionOwner, UObject* NewSourceObject = nullptr) override;
    virtual bool CanExecute() const override;
	
	virtual void OnExecute_Implementation() override;
	virtual void OnComplete_Implementation() override;
	virtual void OnInterrupt_Implementation() override;


	
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttack();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool PlayAttackMontage();

    UFUNCTION(BlueprintCallable, Category = "Combo")
    void ResetCombo();

    bool ValidateExecution() const;
    float GetStaminaCostForCombo() const;

	
};
