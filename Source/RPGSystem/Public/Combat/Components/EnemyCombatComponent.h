// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CombatComponentBase.h"
#include "EnemyCombatComponent.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UEnemyCombatComponent : public UCombatComponentBase
{
	GENERATED_BODY()
public:
	UEnemyCombatComponent();

protected:
	// === Enemy 전용 설정 ===
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float AggroOnDamageMultiplier = 2.f;  // 데미지 × 배율 = 어그로

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	bool bShowHealthBar = true;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Combat")
	float HealthBarVisibleDuration = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Rewards")
	int32 ExperienceReward = 50;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|Rewards")
	TSubclassOf<AActor> DropTableClass;

	// === 히트 리액션 ===
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|HitReaction")
	bool bEnableHitReaction = true;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|HitReaction")
	float HitReactionThreshold = 10.f;  // 이 이상 데미지 시 리액션

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|HitReaction")
	float StaggerThreshold = 50.f;  // 이 이상 데미지 시 스태거

	// === 캐싱 ===
	// UPROPERTY()
	// TWeakObjectPtr<class UTargetingComponent> CachedTargetingComp;

	// 런타임
	float LastHealthBarShowTime = 0.f;

public:
	// === Enemy 전용 기능 ===
	UFUNCTION(BlueprintPure, Category = "Enemy|Combat")
	bool ShouldShowHealthBar() const;

	UFUNCTION(BlueprintPure, Category = "Enemy|Rewards")
	int32 GetExperienceReward() const { return ExperienceReward; }

	// === 델리게이트 ===
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitReaction, float, Damage);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStagger);

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnHitReaction OnHitReaction;

	UPROPERTY(BlueprintAssignable, Category = "Enemy|Events")
	FOnStagger OnStagger;

protected:
	virtual void BeginPlay() override;
	virtual void PostDamageReceived_Implementation(float FinalDamage, AActor* Instigator) override;
	virtual void HandleDeath_Implementation(AActor* Killer) override;

	void AddAggroFromDamage(AActor* Instigator, float Damage);
	void TriggerHitReaction(float Damage);
	void SpawnDrops();
	void GrantExperience(AActor* Killer);
};
