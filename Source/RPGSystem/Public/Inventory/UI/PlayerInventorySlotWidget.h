// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Data/ItemDataStructure.h"
#include "UI/Base/RPGButtonWidget.h"
#include "PlayerInventorySlotWidget.generated.h"

class UItemDragAndDropWidget;
class USizeBox;
class UPlayerInventoryWidget;
class UImage;
class UTextBlock;
class UItemDefinition;
class UInventoryCoreComponent;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UPlayerInventorySlotWidget : public URPGButtonWidget
{
	GENERATED_BODY()

public:
	// UPlayerInventorySlotWidget();
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	
	void SetSlotSize(float NewSize);
	
protected:
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	UItemDragAndDropWidget* CreateDragVisualWidget();
	UDragDropOperation* CreateDragDropOperation(UItemDragAndDropWidget* DragVisual);
	
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UContextMenuWidget> ContextMenuClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "DragDrop")
	TSubclassOf<UItemDragAndDropWidget> DragVisualClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "DragDrop")
	TSubclassOf<UDragDropOperation> DragDropOperationClass;
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_ItemIcon;
    
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Quantity;
    
	UPROPERTY(BlueprintReadOnly, Category = "Slot")
	int32 SlotIndex = -1;
    
	UPROPERTY(BlueprintReadOnly, Category = "Slot")
	FInventorySlot CachedSlot;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;
private:
	UPROPERTY()
	TObjectPtr<UPlayerInventoryWidget> OwningInventoryWidget;
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	float LongPressDuration = 1.1f;

	FTimerHandle LongPressTimerHandle;
	
	void OnLongPressCompleted();
public:
	/** 슬롯 데이터로 UI 업데이트 */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	void UpdateSlot(const FInventorySlot& NewSlot);
    
	/** 슬롯 비우기 */
	UFUNCTION(BlueprintCallable, Category = "Slot")
	void ClearSlot();
    
	void SetSlotIndex(int32 InIndex) { SlotIndex = InIndex; }
	void SetOwningInventoryWidget(UPlayerInventoryWidget* InWidget);
};
