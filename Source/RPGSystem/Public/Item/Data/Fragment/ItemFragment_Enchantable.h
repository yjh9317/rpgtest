// ItemFragment_Enchantable.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "ItemFragment_Enchantable.generated.h"

/**
 * 인챈트 데이터
 * Fragment가 복잡한 런타임 데이터도 자유롭게 관리할 수 있음을 보여주는 예시
 */

USTRUCT(BlueprintType)
struct FEnchantmentData
{
	GENERATED_BODY()

	/** 인챈트 식별 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EnchantmentTag;

	/** 인챈트 레벨 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;

	/** 인챈트 강도 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Magnitude = 1.0f;

	/** 남은 지속시간 (0 = 영구) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float RemainingDuration = 0.0f;
};

UCLASS(meta = (DisplayName = "Enchantable"))
class RPGSYSTEM_API UItemFragment_Enchantable : public UItemFragment
{
	GENERATED_BODY()
	
public:
	//~UObject interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	//~End of UObject interface

public:
	/** 최대 인챈트 슬롯 수 (정적 설정) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enchantment")
	int32 MaxEnchantmentSlots = 3;

	/** 허용되는 인챈트 타입 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enchantment", meta = (Categories = "Enchantment"))
	FGameplayTagContainer AllowedEnchantmentTypes;

	/** 현재 적용된 인챈트 목록 (런타임 가변, 복제됨) */
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Enchantment")
	TArray<FEnchantmentData> ActiveEnchantments;

public:
	/** 인챈트 추가 */
	UFUNCTION(BlueprintCallable, Category = "Enchantment")
	bool AddEnchantment(const FEnchantmentData& NewEnchantment);

	/** 인챈트 제거 */
	UFUNCTION(BlueprintCallable, Category = "Enchantment")
	bool RemoveEnchantment(const FGameplayTag& EnchantmentTag);

	/** 특정 인챈트 보유 여부 */
	UFUNCTION(BlueprintPure, Category = "Enchantment")
	bool HasEnchantment(const FGameplayTag& EnchantmentTag) const;

	/** 인챈트 슬롯 가득 찼는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Enchantment")
	bool IsEnchantmentSlotsFull() const;
};