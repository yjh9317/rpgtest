// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/Data/Fragment/ItemFragment_Upgradable.h"
#include "UpgradingComponent.generated.h"

class UInventoryCoreComponent;
class UItemInstance;
class UItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeSuccess, const UItemDefinition*, NewItemDef);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeFailed, FText, Reason);

/**
 * 새로운 업그레이드 시스템 컴포넌트
 * InventoryCoreComponent와 상호작용하여 아이템을 업그레이드합니다.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSYSTEM_API UUpgradingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UUpgradingComponent(const FObjectInitializer& ObjectInitializer);
	
	FOnUpgradeSuccess OnUpgradeSuccess;
	FOnUpgradeFailed OnUpgradeFailed;
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Upgrade")
	float DefaultUpgradeTime = 5.0f;

public:
	// ========================================
	// 주요 기능
	// ========================================

	/** * 특정 인벤토리 슬롯의 아이템에 대해 가능한 업그레이드 레시피들을 반환합니다.
	 * ItemInstance와 Fragment 구조 활용
	 */
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	bool GetAvailableUpgrades(UInventoryCoreComponent* Inventory, FGuid InventoryGuid, int32 SlotIndex, TArray<FUpgradeRecipe>& OutRecipes);

	/**
	 * 업그레이드 시도
	 * @param Inventory        대상 인벤토리 컴포넌트
	 * @param InventoryGuid    인벤토리 GUID
	 * @param SlotIndex        대상 아이템 슬롯 인덱스
	 * @param RecipeIndex      선택한 레시피 인덱스 (GetAvailableUpgrades의 결과 기준)
	 */
	UFUNCTION(BlueprintCallable, Category = "Upgrade")
	void TryUpgradeItem(UInventoryCoreComponent* Inventory, FGuid InventoryGuid, int32 SlotIndex, int32 RecipeIndex);

private:
	bool HasEnoughMaterials(UInventoryCoreComponent* Inventory, const TArray<FUpgradeCost>& Costs);
	void ConsumeMaterials(UInventoryCoreComponent* Inventory, const TArray<FUpgradeCost>& Costs);
	void TransferItemState(UItemInstance* SourceInstance, UItemInstance* DestInstance);
};