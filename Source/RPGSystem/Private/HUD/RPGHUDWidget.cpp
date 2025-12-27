// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RPGHUDWidget.h"

#include "Inventory/DragDrop/InventoryDragDropOperation.h"

void URPGHUDWidget::InitializeHUD(APawn* OwningPawn)
{
	CachedPawn = OwningPawn;
}

void URPGHUDWidget::BindStatsModel(UStatsViewModel* InStatsViewModel)
{
	// 여기서 하위 StatBar 위젯들에게 ViewModel을 연결해줍니다.
	// if (WB_StatBars && InStatsViewModel)
	// {
	//     WB_StatBars->Setup(InStatsViewModel);
	// }
}

bool URPGHUDWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UInventoryDragDropOperation* RPGOp = Cast<UInventoryDragDropOperation>(InOperation);
	if (RPGOp)
	{
		// UI가 아닌 곳(땅)에 드랍함 -> 아이템 버리기 처리
		DropItemToWorld(RPGOp->SourceInventoryGuid, RPGOp->SourceSlotIndex);
		return true;
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void URPGHUDWidget::DropItemToWorld(FGuid InventoryGuid, int32 SlotIndex)
{
	// PlayerController->ServerRequestDropItem(InventoryGuid, SlotIndex);
}
