// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/RPGHUDWidget.h"

#include "Inventory/DragDrop/InventoryDragDropOperation.h"
#include "Status/StatsViewModel.h"


void URPGHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (!StatsViewModel)
	{
		StatsViewModel = NewObject<UStatsViewModel>(this);
	}
}

void URPGHUDWidget::InitializeHUD(APawn* OwningPawn)
{
	CachedPawn = OwningPawn;
	
}

void URPGHUDWidget::SetViewModel(UStatsViewModel* InViewModel)
{
	StatsViewModel = InViewModel;
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
