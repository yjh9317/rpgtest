// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatData.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class UStatsComponent;
class UActionComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDamageApplied, AActor*, Target, float, FinalDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UCombatComponent();

protected:
    // HP/방어력 등은 UStatsComponent에 있다고 가정
    UPROPERTY()
    TObjectPtr<UStatsComponent> CachedStats = nullptr;

    UPROPERTY()
    TObjectPtr<UActionComponent> CachedActionComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TWeakObjectPtr<AActor> CurrentTarget;

public:
    virtual void BeginPlay() override;

    void SetCurrentTarget(AActor* NewTarget);
    AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }
    bool HasTarget() const { return CurrentTarget.IsValid(); }
    float ApplyDamageToTarget(const FDamageInfo& DamageInfo);
    float ApplyDamage(AActor* Target, const FDamageInfo& DamageInfo);
    bool IsAlive() const;
    bool IsInCombat() const { return bInCombat; }

    void EnterCombat();
    void LeaveCombat();

    FOnDamageApplied OnDamageApplied;
    FOnDeath OnDeath;

protected:
    bool bInCombat = false;

    float CalculateFinalDamage(const FDamageInfo& DamageInfo, AActor* Target) const;
    void HandleDeath(AActor* Target);
};
