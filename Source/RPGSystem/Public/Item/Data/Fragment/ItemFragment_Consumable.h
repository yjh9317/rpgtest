// ItemFragment_Consumable.h (개선된 버전)
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemFragment_Usable.h"
#include "ItemFragment_Consumable.generated.h"

UENUM(BlueprintType)
enum class EConsumableEffectType : uint8
{
    Instant         UMETA(DisplayName = "Instant"),      // 즉시 적용
    OverTime        UMETA(DisplayName = "Over Time"),    // 시간에 걸쳐
    Duration        UMETA(DisplayName = "Duration"),     // 지속 효과
    Buff            UMETA(DisplayName = "Buff")          // 버프 부여
};

USTRUCT(BlueprintType)
struct FConsumableEffect
{
    GENERATED_BODY()
    
    /** 영향을 받는 스탯 태그 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect", 
        meta = (Categories = "Character.Stat"))
    FGameplayTag StatTag;
    
    /** 효과 타입 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
    EConsumableEffectType EffectType = EConsumableEffectType::Instant;
    
    /** 효과 값 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
    float Value = 50.0f;
    
    /** 지속 시간 (OverTime, Duration용) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect",
        meta = (EditCondition = "EffectType != EConsumableEffectType::Instant"))
    float Duration = 0.0f;
    
    /** 틱 횟수 (OverTime용) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect",
        meta = (EditCondition = "EffectType == EConsumableEffectType::OverTime"))
    int32 NumberOfTicks = 1;
};

/**
 * 소비 아이템 Fragment
 * 음식, 포션 등의 사용 가능한 아이템
 */
UCLASS(meta = (DisplayName = "Consumable"))
class RPGSYSTEM_API UItemFragment_Consumable : public UItemFragment_Usable
{
    GENERATED_BODY()

public:
    /** 적용할 효과 목록 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
    TArray<FConsumableEffect> Effects;
    
    /** 사용 시 소모 여부 (보통 true) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Consumable")
    bool bConsumeOnUse = true;

public:
    //~UItemFragment_Usable interface
    virtual bool CanUse_Implementation(AActor* User) const override;
    virtual bool UseItem_Implementation(AActor* User) override;
    virtual void OnItemUsed_Implementation(AActor* User) override;
    //~End of UItemFragment_Usable interface

private:
    /** 효과 적용 */
    void ApplyEffects(AActor* User);
    // /** 즉시 효과 */
    // void ApplyInstantEffect(class UStatsComponent* StatsComp, const FConsumableEffect& Effect);
    // /** 지속 효과 */
    // void ApplyDurationEffect(class UStatsComponent* StatsComp, AActor* User, const FConsumableEffect& Effect);
    // /** 틱 효과 */
    // void ApplyOverTimeEffect(class UStatsComponent* StatsComp, AActor* User, const FConsumableEffect& Effect);
};