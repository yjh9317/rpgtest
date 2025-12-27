// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "Item/Data/ItemDataStructure.h"
#include "InventoryDragDropOperation.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UInventoryDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
public:
	// 드래그 중인 아이템의 원본 정보
	UPROPERTY(BlueprintReadWrite, Category = "DragDrop")
	FGuid SourceInventoryGuid;

	UPROPERTY(BlueprintReadWrite, Category = "DragDrop")
	int32 SourceSlotIndex;

	UPROPERTY(BlueprintReadWrite, Category = "DragDrop")
	FInventorySlot SlotData;
    
	// 비주얼용 위젯 (드래그 따라다니는 아이콘)
	UPROPERTY(BlueprintReadWrite, Category = "DragDrop")
	UUserWidget* VisualWidget;
};
