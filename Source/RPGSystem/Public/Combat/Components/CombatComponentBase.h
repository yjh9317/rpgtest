// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Combatable.h"
#include "Combat/CombatData.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "CombatComponentBase.generated.h"

class UStatsComponent;
class URPGEffect;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageReceived, float, Damage, AActor*, Instigator, const FDamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDamageDealt, float, Damage, AActor*, Target, const FDamageInfo&, DamageInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatDeath, AActor*, Killer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatStateChanged, bool, bInCombat);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPreDamageApplied, const FDamageInfo&, DamageInfo, AActor*, Instigator);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnPostDamageApplied, float, AppliedDamage, const FDamageInfo&, DamageInfo, AActor*, Instigator, bool, bKilledTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedFromCombat, float, OldHealth, float, NewHealth);

USTRUCT(BlueprintType)
struct FDamageEffectRule
{
	GENERATED_BODY()

	/** Optional damage type tag match (exact or child tag match via MatchesTag). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Effects", meta=(Categories="Damage.Type"))
	FGameplayTag RequiredDamageTypeTag;

	/** Optional damage tag set match (if empty, ignored). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Effects")
	FGameplayTagContainer RequiredDamageTags;

	/** Effect applied to the damaged target when rule matches. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Effects")
	TObjectPtr<URPGEffect> EffectToApply = nullptr;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UCombatComponentBase : public UActorComponent, public ICombatable
{
	GENERATED_BODY()

public:
	UCombatComponentBase();

protected:
	// === 캐싱 ===
	UPROPERTY()
	TWeakObjectPtr<UStatsComponent> CachedStatsComp;

	// === 상태 ===
	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
	bool bIsInCombat = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
	bool bIsDead = false;

	UPROPERTY(BlueprintReadOnly, Category = "Combat|State")
	float LastCombatTime = 0.f;

	// === 설정 ===
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Config")
	float CombatTimeout = 5.f;  // 전투 이탈 시간

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Config")
	bool bAutoLeaveCombat = true;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Config", meta=(Categories="Character.Stats"))
	FGameplayTag HealthStatTag;

	/** Damage tag/type -> effect application rules. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat|Config")
	TArray<FDamageEffectRule> DamageEffectRules;

public:
	// === 델리게이트 ===
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDamageReceived OnDamageReceived;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnDamageDealt OnDamageDealt;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnCombatDeath OnCombatDeath;

	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnCombatStateChanged OnCombatStateChanged;

	/** Fired before damage is resolved. Useful for hit-stop/camera/vfx hooks. */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnPreDamageApplied OnPreDamageApplied;

	/** Fired after damage has been resolved. */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnPostDamageApplied OnPostDamageApplied;

	/** Fired when combat damage changed health value. */
	UPROPERTY(BlueprintAssignable, Category = "Combat|Events")
	FOnHealthChangedFromCombat OnHealthChangedFromCombat;

	// === ICombatable 구현 ===
	virtual bool IsAlive() const override { return !bIsDead; }
	virtual float GetCurrentHealth() const override;
	virtual float GetMaxHealth() const override;
	virtual float ReceiveDamage(const FDamageInfo& DamageInfo) override;
	virtual float ApplyDamage(AActor* Target, const FDamageInfo& DamageInfo) override;
	virtual bool IsInCombat() const override { return bIsInCombat; }
	virtual void EnterCombat(AActor* Opponent) override;
	virtual void LeaveCombat() override;

	// === 공용 함수 ===
	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetInvulnerable(bool bNewInvulnerable);

	UFUNCTION(BlueprintPure, Category = "Combat")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Combat")
	AActor* GetCurrentTarget() const;
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
		FActorComponentTickFunction* ThisTickFunction) override;

	// === 오버라이드 포인트 ===
	// 데미지 계산 (Player/AI 다르게 구현 가능)
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	float CalculateFinalDamage(const FDamageInfo& DamageInfo);
	virtual float CalculateFinalDamage_Implementation(const FDamageInfo& DamageInfo);

	// 데미지 적용 후 처리
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void PostDamageReceived(float FinalDamage, AActor* Instigator);
	virtual void PostDamageReceived_Implementation(float FinalDamage, AActor* Instigator);

	// 사망 처리
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void HandleDeath(AActor* Killer);
	virtual void HandleDeath_Implementation(AActor* Killer);

	void CheckCombatTimeout(float DeltaTime);
	UStatsComponent* GetStatsComponent() const;
	void LogInvalidHealthStatTag(const TCHAR* CallerName) const;
	void ApplyDamageDrivenEffects(const FDamageInfo& DamageInfo);

private:
	bool bIsInvulnerable = false;
	UPROPERTY()
	TObjectPtr<AActor> TargetActor;
};
