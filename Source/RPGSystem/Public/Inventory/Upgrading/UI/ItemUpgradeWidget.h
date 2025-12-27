// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Upgrading/UpgradingComponent.h"
#include "ItemUpgradeWidget.generated.h"

class UImage;
class UTextBlock;
class UButton;
class UVerticalBox;
class UInventoryCoreComponent;
class UItemInstance;
class UItemDefinition;

/**
 * 아이템 업그레이드 윈도우 UI
 */
UCLASS()
class RPGSYSTEM_API UItemUpgradeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// 위젯 초기화 (인벤토리에서 아이템 우클릭 시 호출)
	UFUNCTION(BlueprintCallable, Category = "Upgrade UI")
	void SetupUpgradeWindow(UInventoryCoreComponent* InInventory, UUpgradingComponent* InUpgradingComp, FGuid InInventoryGuid, int32 InSlotIndex);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// UI 갱신 메인 함수
	void RefreshUI();
	
	// 스탯 비교 텍스트 생성 (예: "Strength +5 (+2)")
	void UpdateStatComparison(const UItemInstance* CurrentItem, const UItemDefinition* NextItemDef);

	// 재료 비용 UI 생성
	void UpdateCostList(const TArray<FUpgradeCost>& Costs);

	UFUNCTION()
	void OnUpgradeButtonClicked();

	// 업그레이드 성공/실패 델리게이트 핸들러
	UFUNCTION()
	void HandleUpgradeSuccess(const UItemDefinition* NewItemDef);

protected:
	// --- Data ---
	UPROPERTY()
	TObjectPtr<UInventoryCoreComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UUpgradingComponent> UpgradingComp;

	FGuid TargetInventoryGuid;
	int32 TargetSlotIndex = -1;
	int32 SelectedRecipeIndex = 0; // 여러 분기가 있다면 선택 필요, 기본은 0

	// --- Widgets (BindWidget) ---
	
	// 1. Header (Icon)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ItemName;

	// 2. Comparison (Left: Current, Right: Next)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_CurrentStats;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_NextStats;

	// 3. Footer (Cost & Action)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_CostList;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Upgrade;
};
