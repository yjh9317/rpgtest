// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerInventoryWidget.generated.h"

class USizeBox;
class UItemDefinition;
class UUniformGridPanel;
class UPlayerInventorySlotWidget;
class UInventoryCoreComponent;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UPlayerInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void InitializeInventory(UInventoryCoreComponent* InComponent, FGuid InInventoryGuid);
	void InitializeInventory(UInventoryCoreComponent* InComponent, FName InInventoryName);
	
	virtual void SynchronizeProperties() override;
	
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FGuid CurrentInventoryGuid;
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void BindDelegateFunc();

	void OnInventoryRefresh(FGuid InventoryGuid);
	void OnSlotChanged(FGuid InventoryGuid, int32 SlotIndex);
	void OnItemAddedToInventory(FGuid InventoryGuid, int32 SlotIndex, const UItemDefinition* ItemDef);
	void OnItemRemovedFromInventory(FGuid InventoryGuid, int32 SlotIndex, const UItemDefinition* ItemDef);
	void OnWeightUpdated(FGuid InventoryGuid, float CurrentWeight);


private:
	// ========================================
	// 내부 갱신 함수들
	// ========================================
	void CreateSlotWidgets(int32 SlotCount);
	void RefreshAllSlots();
	void RefreshSingleSlot(int32 SlotIndex);
	void UpdateWeightDisplay();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TObjectPtr<UInventoryCoreComponent> InventoryComponent;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> SlotGrid;
	
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
	TSubclassOf<UPlayerInventorySlotWidget> SlotWidgetClass;
    
	UPROPERTY()
	TArray<TObjectPtr<UPlayerInventorySlotWidget>> SlotWidgets;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> Sizebox_Root;

	// [기존 변수]
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float InventoryTotalWidth = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	int32 ColumnsPerRow = 6;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	float SlotPadding = 5.0f;
	
public:
	FORCEINLINE UInventoryCoreComponent* GetInventoryComponent() const { return InventoryComponent; }
};
