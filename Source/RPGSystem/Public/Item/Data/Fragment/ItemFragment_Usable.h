#pragma once

#include "CoreMinimal.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "ItemFragment_Usable.generated.h"

/**
 * 사용 가능한 아이템의 베이스 Fragment
 * Consumable, Tool 등이 이를 상속
 */

UCLASS(Abstract, meta = (DisplayName = "Usable"))
class RPGSYSTEM_API UItemFragment_Usable : public UItemFragment
{
    GENERATED_BODY()

public:
    /** 아이템 사용 가능 여부 체크 */
    UFUNCTION(BlueprintNativeEvent, Category = "Item Usage")
    bool CanUse(AActor* User) const;
    virtual bool CanUse_Implementation(AActor* User) const { return true; }


    /** 아이템 사용 */
    UFUNCTION(BlueprintNativeEvent, Category = "Item Usage")
    bool UseItem(AActor* User);
    virtual bool UseItem_Implementation(AActor* User) { return false; }


    /** 사용 후 처리 (수량 감소, 파괴 등) */
    UFUNCTION(BlueprintNativeEvent, Category = "Item Usage")
    void OnItemUsed(AActor* User);
    virtual void OnItemUsed_Implementation(AActor* User) {}


    /** 사용 시 애니메이션 재생 여부 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
    bool bPlayUseAnimation = true;


    /** 사용 애니메이션 몽타주 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
    TSoftObjectPtr<UAnimMontage> UseAnimationMontage;


    /** 사용 시 사운드 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
    TSoftObjectPtr<USoundBase> UseSound;

    /** 사용 시 파티클 이펙트 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
    TSoftObjectPtr<UParticleSystem> UseEffect;

    /** 사용 쿨다운 시간 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Usage")
    float CooldownDuration = 0.0f;

    /** 마지막 사용 시간 (런타임) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Usage")
    float LastUsedTime = 0.0f;

public:
    //~UObject interface
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    //~End of UObject interface

    UFUNCTION(BlueprintPure, Category = "Item Usage")
    bool IsOnCooldown(const UWorld* World) const;

    UFUNCTION(BlueprintPure, Category = "Item Usage")
    float GetRemainingCooldown(const UWorld* World) const;

};