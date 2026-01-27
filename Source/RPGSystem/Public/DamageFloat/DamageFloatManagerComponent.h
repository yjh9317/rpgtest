// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DamageFloatManagerComponent.generated.h"

USTRUCT(BlueprintType)
struct FDamageFloatData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Damage Float")
    float DamageAmount = 0.0f;

    /** 
     * CRITICAL: 공격받은 "그 순간"의 월드 위치 (고정값!)
     * 몬스터가 이동해도 이 값은 변하지 않음
     */
    UPROPERTY(BlueprintReadWrite, Category = "Damage Float")
    FVector SpawnWorldPosition = FVector::ZeroVector;

    /** 애니메이션용 수직 오프셋 (SpawnWorldPosition 기준) */
    float VerticalOffset = 0.0f;

    /** 수평 랜덤 오프셋 (겹침 방지) */
    UPROPERTY(BlueprintReadWrite, Category = "Damage Float")
    FVector2D HorizontalOffset = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Damage Float")
    float RemainingLifetime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Damage Float")
    bool bIsCritical = false;

    UPROPERTY(BlueprintReadWrite, Category = "Damage Float")
    FLinearColor TextColor = FLinearColor::White;

    /** Spawn 시간 (디버깅용) */
    float SpawnTime = 0.0f;

    FDamageFloatData() = default;

    FDamageFloatData(float InDamage, const FVector& InWorldPosition, bool bInCritical = false)
        : DamageAmount(InDamage)
        , SpawnWorldPosition(InWorldPosition) // 이 위치는 이후 고정됨!
        , RemainingLifetime(2.0f)
        , bIsCritical(bInCritical)
    {
        TextColor = bInCritical ? FLinearColor::Red : FLinearColor::White;
        
        // 같은 위치에서 연속 hit 시 겹침 방지
        HorizontalOffset = FVector2D(
            FMath::RandRange(-20.0f, 20.0f),
            FMath::RandRange(-20.0f, 20.0f)
        );
    }

    /** 현재 렌더링 위치 (월드 좌표) */
    FVector GetCurrentWorldPosition() const
    {
        return FVector(
            SpawnWorldPosition.X + HorizontalOffset.X,
            SpawnWorldPosition.Y + HorizontalOffset.Y,
            SpawnWorldPosition.Z + VerticalOffset
        );
    }
};

/**
 * Damage Float Manager Component
 * 플레이어 컨트롤러나 HUD에 부착하여 사용
 */

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UDamageFloatManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UDamageFloatManagerComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                               FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * Spawn damage float at EXACT world position
     * @param DamageAmount - Damage value
     * @param WorldPosition - 공격받은 순간의 정확한 월드 위치 (이후 고정됨!)
     * @param bIsCritical - Critical hit flag
     */
    UFUNCTION(BlueprintCallable, Category = "Damage Float")
    void SpawnDamageFloat(float DamageAmount, FVector WorldPosition, bool bIsCritical = false);

    /**
     * Convenience function: Spawn at actor's location + offset
     * @param TargetActor - 피격당한 액터 (nullptr 체크 포함)
     * @param HeightOffset - 액터 중심에서 위로 얼마나 띄울지
     */
    UFUNCTION(BlueprintCallable, Category = "Damage Float")
    void SpawnDamageFloatAtActor(AActor* TargetActor, float DamageAmount, float HeightOffset = 100.0f, bool bIsCritical = false);

    UFUNCTION(BlueprintPure, Category = "Damage Float")
    const TArray<FDamageFloatData>& GetActiveDamageFloats() const { return ActiveFloats; }

    UFUNCTION(BlueprintPure, Category = "Damage Float")
    int32 GetActiveDamageFloatCount() const { return ActiveFloats.Num(); }

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "10", ClampMax = "500"))
    int32 MaxActiveDamageFloats = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float DamageFloatLifetime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float VerticalFloatSpeed = 100.0f;

    /** Off-screen floats를 culling할지 여부 (최적화) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bCullOffScreenFloats = true;

    /** Off-screen으로 간주할 거리 (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization", meta = (EditCondition = "bCullOffScreenFloats"))
    float MaxDistanceFromCamera = 5000.0f;

private:
    UPROPERTY()
    TArray<FDamageFloatData> ActiveFloats;

    void PreallocatePool();
    void CullDistantFloats();

    /** 현재 월드 시간 캐싱 (디버깅용) */
    float CurrentWorldTime = 0.0f;
};
