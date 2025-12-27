// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "HitBoxComponent.generated.h"

class ARPGPlayerCharacter;

// 상수 정의
namespace HitBoxConstants
{
    static constexpr float DEFAULT_TRACE_RADIUS = 50.0f;
    static constexpr float DEFAULT_DEBUG_DURATION = 2.0f;
    static constexpr float DEFAULT_MULTI_HIT_DELAY = 0.1f;
    static constexpr float DEFAULT_HIT_STOP_DURATION = 0.1f;
}

UENUM(BlueprintType)
enum class ETraceType : uint8
{
    LineTrace    UMETA(DisplayName = "Line Trace"),
    SphereTrace  UMETA(DisplayName = "Sphere Trace"),
    BoxTrace     UMETA(DisplayName = "Box Trace"),
    CapsuleTrace UMETA(DisplayName = "Capsule Trace")
};

UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Physical    UMETA(DisplayName = "Physical"),
    Fire        UMETA(DisplayName = "Fire"),
    Ice         UMETA(DisplayName = "Ice"),
    Lightning   UMETA(DisplayName = "Lightning"),
    Dark        UMETA(DisplayName = "Dark"),
    Holy        UMETA(DisplayName = "Holy"),
    Pure        UMETA(DisplayName = "Pure") // 방어 무시
};

UENUM(BlueprintType)
enum class EHitBoxState : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Cooldown    UMETA(DisplayName = "Cooldown")
};

USTRUCT(BlueprintType)
struct FHitBoxDamageEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> HitActor = nullptr;

	UPROPERTY(BlueprintReadOnly)
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly)
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly)
	FName BoneName = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	float FinalDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	bool bWasCritical = false;

	UPROPERTY(BlueprintReadOnly)
	EDamageType DamageType = EDamageType::Physical;

	UPROPERTY(BlueprintReadOnly)
	float DamageMultiplier = 1.0f;

	UPROPERTY(BlueprintReadOnly)
	int32 ComboIndex = 0;

    FHitBoxDamageEvent()
    {
        Reset();
    }

    void Reset()
    {
        HitActor = nullptr;
        HitLocation = FVector::ZeroVector;
        HitNormal = FVector::UpVector;
        BoneName = NAME_None;
        FinalDamage = 0.0f;
        bWasCritical = false;
        DamageType = EDamageType::Physical;
        DamageMultiplier = 1.0f;
        ComboIndex = 0;
    }
};

// 히트박스 설정을 위한 구조체
USTRUCT(BlueprintType)
struct FHitBoxSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETraceType TraceType = ETraceType::SphereTrace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "1.0"))
    float TraceRadius = HitBoxConstants::DEFAULT_TRACE_RADIUS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector BoxExtent = FVector(50.0f, 50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CapsuleRadius = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CapsuleHalfHeight = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FName> TraceSocketNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bTraceComplex = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIgnoreOwner = true;

    FHitBoxSettings()
    {
        TraceSocketNames.Add(TEXT("WeaponSocket"));
        TraceSocketNames.Add(TEXT("WeaponTip"));
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitBoxActivated, UHitBoxComponent*, HitBox);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitBoxDeactivated, UHitBoxComponent*, HitBox);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitBoxHit, const FHitBoxDamageEvent&, HitEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHitBoxStateChanged, EHitBoxState, OldState, EHitBoxState, NewState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UHitBoxComponent : public UActorComponent
{
	GENERATED_BODY()


public:
	UHitBoxComponent();

	// === 델리게이트 ===
	UPROPERTY(BlueprintAssignable)
	FOnHitBoxActivated OnHitBoxActivated;

	UPROPERTY(BlueprintAssignable)
	FOnHitBoxDeactivated OnHitBoxDeactivated;

	UPROPERTY(BlueprintAssignable)
	FOnHitBoxHit OnHitBoxHit;

	UPROPERTY(BlueprintAssignable)
	FOnHitBoxStateChanged OnHitBoxStateChanged;

    // === 트레이스 설정 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Trace")
    FHitBoxSettings HitBoxSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Debug")
    bool bDebugDraw = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Debug", meta = (ClampMin = "0.1"))
    float DebugDrawDuration = HitBoxConstants::DEFAULT_DEBUG_DURATION;

    // === 데미지 설정 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Damage")
    float BaseDamage = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Damage")
    EDamageType DamageType = EDamageType::Physical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Damage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CriticalChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Damage", meta = (ClampMin = "1.0"))
    float CriticalMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Damage")
    TMap<FName, float> BodyPartDamageMultipliers;

    // === 다단히트 설정 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|MultiHit")
    bool bAllowMultiHit = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|MultiHit", meta = (ClampMin = "0.01"))
    float MultiHitDelay = HitBoxConstants::DEFAULT_MULTI_HIT_DELAY;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|MultiHit", meta = (ClampMin = "1"))
    int32 MaxHitsPerActivation = 1;

    // === 히트 효과 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Effects")
    TObjectPtr<UParticleSystem> HitEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Effects")
    TObjectPtr<USoundBase> HitSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Effects", meta = (ClampMin = "0.0"))
    float HitStopDuration = HitBoxConstants::DEFAULT_HIT_STOP_DURATION;

    // === 넉백/경직 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Physics")
    float KnockbackForce = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Physics")
    float HitStunDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Physics")
    bool bApplyKnockback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Physics")
    bool bApplyHitStun = true;

    // === 필터링 ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Filter")
    TArray<TSubclassOf<AActor>> TargetClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Filter")
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HitBox|Filter")
    FGameplayTagContainer IgnoreTags;

protected:
    virtual void BeginPlay() override;
    virtual void BeginDestroy() override;

    UPROPERTY(Transient)
    TObjectPtr<ARPGPlayerCharacter> OwnerCharacter;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> OwnerMesh;

    // === 히트박스 상태 ===
    UPROPERTY(BlueprintReadOnly, Category = "HitBox|State")
    EHitBoxState CurrentState = EHitBoxState::Inactive;

    // === 히트 기록 ===
    UPROPERTY()
    TArray<TObjectPtr<AActor>> HitActors;

    UPROPERTY()
    TMap<TObjectPtr<AActor>, float> LastHitTimes;

    UPROPERTY(BlueprintReadOnly, Category = "HitBox|Stats")
    int32 CurrentHitCount = 0;

    // === 타이머 ===
    FTimerHandle MultiHitTimer;
    FTimerHandle CooldownTimer;

    // === 트레이스 캐시 ===
    TArray<FVector> PreviousTracePositions;
    FCollisionQueryParams CachedQueryParams;

private:
	// === 내부 함수들 ===
	void PerformTrace();
	void SetHitBoxState(EHitBoxState NewState);
	
	// 트레이스 함수들
	bool PerformLineTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits);
	bool PerformSphereTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits);
	bool PerformBoxTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits);
	bool PerformCapsuleTrace(const FVector& Start, const FVector& End, TArray<FHitResult>& OutHits);
    
	// 히트 처리
	void ProcessHit(const FHitResult& Hit);
	bool CanHitActor(AActor* Actor) const;
	float CalculateFinalDamage(const FHitResult& Hit, bool& bOutWasCritical);
	void ApplyDamageToActor(AActor* HitActor, const FHitResult& Hit);
	
	// 이펙트
	void PlayHitEffects(const FVector& Location, const FVector& Normal);
	void ApplyHitStop();
	void ApplyKnockback(AActor* HitActor, const FVector& Direction);
	
	// 유틸리티
	void UpdateQueryParams();
	void CacheTracePositions();

public:
	// === 메인 함수들 ===
	UFUNCTION(BlueprintCallable, Category = "HitBox|Trace")
	FHitResult MainTrace(const TArray<FName>& SocketsName,ETraceType TraceTypeOverride = ETraceType::SphereTrace);

	UFUNCTION(BlueprintCallable, Category = "HitBox|Control")
	void ActivateHitBox();

	UFUNCTION(BlueprintCallable, Category = "HitBox|Control")
	void DeactivateHitBox();

	UFUNCTION(BlueprintCallable, Category = "HitBox|Control")
	void ResetCombo();

	UFUNCTION(BlueprintCallable, Category = "HitBox|Control")
	void SetDamageMultiplier(float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "HitBox|Control")
	void ClearHitActors();

	UFUNCTION(BlueprintCallable, Category = "HitBox|Control")
	void SetBaseDamage(float NewBaseDamage) { BaseDamage = NewBaseDamage; }

	// === 상태 쿼리 ===
	UFUNCTION(BlueprintPure, Category = "HitBox|State")
	bool IsHitBoxActive() const { return CurrentState == EHitBoxState::Active; }

	UFUNCTION(BlueprintPure, Category = "HitBox|State")
	EHitBoxState GetHitBoxState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "HitBox|Stats")
	int32 GetHitCount() const { return CurrentHitCount; }

	UFUNCTION(BlueprintPure, Category = "HitBox|Stats")
	int32 GetMaxHits() const { return MaxHitsPerActivation; }

	UFUNCTION(BlueprintPure, Category = "HitBox|Stats")
	bool CanHitMore() const { return CurrentHitCount < MaxHitsPerActivation; }

	// === 설정 변경 ===
	UFUNCTION(BlueprintCallable, Category = "HitBox|Settings")
	void SetTraceSettings(const FHitBoxSettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category = "HitBox|Settings")
	void SetDamageType(EDamageType NewDamageType) { DamageType = NewDamageType; }

	UFUNCTION(BlueprintCallable, Category = "HitBox|Settings")
	void SetCriticalSettings(float NewCritChance, float NewCritMultiplier);

	UFUNCTION(BlueprintCallable, Category = "HitBox|Settings")
	void AddBodyPartMultiplier(FName BodyPart, float Multiplier);

	UFUNCTION(BlueprintCallable, Category = "HitBox|Settings")
	void RemoveBodyPartMultiplier(FName BodyPart);

	// === 틱 기반 연속 트레이스 ===
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
		FActorComponentTickFunction* ThisTickFunction) override;

	// === 콤보 시스템 연동 ===
	UPROPERTY(BlueprintReadOnly, Category = "HitBox|Combo")
	int32 CurrentComboCount = 0;

	UFUNCTION(BlueprintCallable, Category = "HitBox|Combo")
	void SetComboCount(int32 NewComboCount) { CurrentComboCount = NewComboCount; }

	// === 유틸리티 함수 ===
	UFUNCTION(BlueprintPure, Category = "HitBox|Utility")
	TArray<AActor*> GetHitActors() const;

	UFUNCTION(BlueprintCallable, Category = "HitBox|Utility")
	void AddIgnoreActor(AActor* ActorToIgnore);

	UFUNCTION(BlueprintCallable, Category = "HitBox|Utility")
	void RemoveIgnoreActor(AActor* ActorToRemove);

	UFUNCTION(BlueprintCallable, Category = "HitBox|Debug")
	void SetDebugDraw(bool bEnabled, float Duration = -1.0f);

private:
	// === 내부 변수 ===
	float CurrentDamageMultiplier = 1.0f;
	TArray<TObjectPtr<AActor>> IgnoreActors;
	
	// === 내부 함수 ===
	void OnMultiHitTimerComplete();
	void OnCooldownComplete();
		
};
