// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RPGHUDWidget.generated.h"

class UStatsViewModel;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
	
	
public:
	void InitializeHUD(APawn* OwningPawn);
	void BindStatsModel(UStatsViewModel* InStatsViewModel);
	
protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	void DropItemToWorld(FGuid InventoryGuid, int32 SlotIndex);
protected:
	// UPROPERTY(meta = (BindWidget))
	// TObjectPtr<UMinimapWidget> WB_Minimap;
	
	// UPROPERTY(meta = (BindWidget))
	// TObjectPtr<UCompassWidget> WB_Compass;
	
	// UPROPERTY(meta = (BindWidget))
	// TObjectPtr<UStatBarWidget> WB_StatBars;
	
private:
	// 캐싱된 폰 참조 (Tick에서 위치 업데이트용)
	UPROPERTY()
	TObjectPtr<APawn> CachedPawn;
};
