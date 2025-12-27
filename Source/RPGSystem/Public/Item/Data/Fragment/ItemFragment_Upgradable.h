#pragma once

#include "CoreMinimal.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "GameplayTagContainer.h"
#include "ItemFragment_Upgradable.generated.h"

class UItemDefinition;

/** 업그레이드 비용 (재료) */
USTRUCT(BlueprintType)
struct FUpgradeCost
{
	GENERATED_BODY()

	/** 필요한 재료 아이템 (화폐 포함) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemDefinition> ItemDef;

	/** 필요 수량 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Quantity = 1;
};

/** 업그레이드 레시피 정보 */
USTRUCT(BlueprintType)
struct FUpgradeRecipe
{
	GENERATED_BODY()

	/** 이 레시피의 이름 (UI 표시용) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	/** 업그레이드 결과물 아이템 정의 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemDefinition> ResultItemDef;

	/** 필요한 재료 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FUpgradeCost> Costs;

	/** 업그레이드 성공 확률 (0.0 ~ 1.0) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float SuccessRate = 1.0f;
    
	/** 기존 아이템의 상태(내구도, 인챈트 등)를 유지할지 여부 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bTransferItemState = true;
};

/**
 * 아이템에 "업그레이드 가능" 속성을 부여하는 Fragment
 */
UCLASS(meta = (DisplayName = "Upgradable"))
class RPGSYSTEM_API UItemFragment_Upgradable : public UItemFragment
{
	GENERATED_BODY()

public:
	/** 가능한 업그레이드 경로 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Upgrade")
	TArray<FUpgradeRecipe> UpgradeRecipes;
};