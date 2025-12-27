// CraftingRecipe.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CraftingRecipe.generated.h"

class UItemDefinition;

/** 재료 정보 구조체 */
USTRUCT(BlueprintType)
struct FCraftingIngredient
{
	GENERATED_BODY()

	/** 특정 아이템 정의가 필요할 경우 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemDefinition> ItemDef;

	/** 혹은 특정 태그를 가진 아이템(예: "Material.Wood")이면 될 경우 (고급 기능) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTagContainer InputTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Quantity = 1;
};

/** 제작 결과물 구조체 */
USTRUCT(BlueprintType)
struct FCraftingOutput
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UItemDefinition> ItemDef;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Quantity = 1;
};

/**
 * 제작 레시피 정의 (DataAsset)
 */
UCLASS(BlueprintType, Const)
class RPGSYSTEM_API UCraftingRecipe : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** UI 표시 이름 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FText DisplayName;

	/** 제작 카테고리 (Weapon, Armor, Potion 등) - Enum 대신 GameplayTag 권장 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta=(Categories="Crafting.Category"))
	FGameplayTag CategoryTag;

	/** 필요한 제작대 유형 (Forge, Workbench 등) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta=(Categories="Crafting.Station"))
	FGameplayTag StationTag;

	/** 제작 소요 시간 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float CraftingTime = 2.0f;

	/** 재료 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ingredients")
	TArray<FCraftingIngredient> Ingredients;

	/** 결과물 목록 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Outputs")
	TArray<FCraftingOutput> Outputs;
};