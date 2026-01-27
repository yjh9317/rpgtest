// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RPGHUDWidget.generated.h"

class UStatsComponent;
class UProgressBarWidget;
class UStatsViewModel;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGHUDWidget : public UUserWidget
{
	GENERATED_BODY()
			
public:
	UFUNCTION(BlueprintCallable, Category = "Init")
	void InitializeHUD(APawn* OwningPawn);
	
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stats")
	TObjectPtr<UStatsViewModel> StatsViewModel;
	
	UFUNCTION(BlueprintCallable)
	void SetViewModel(UStatsViewModel* InViewModel);
protected:
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeConstruct() override;
	
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


// public:
// // --- 상시 표시 위젯들 ---
// UPROPERTY(meta = (BindWidget))
// TObjectPtr<UMinimapWidget> WBP_Minimap;
//
// UPROPERTY(meta = (BindWidget))
// TObjectPtr<UCompassWidget> WBP_Compass;
//
// UPROPERTY(meta = (BindWidget))
// TObjectPtr<UInteractionPromptWidget> WBP_InteractionPrompt;
//
// UPROPERTY(meta = (BindWidget))
// TObjectPtr<UCrosshairWidget> WBP_Crosshair;
//
// // --- 헬퍼 함수들 ---
// UFUNCTION(BlueprintCallable, Category = "HUD")
// void SetCrosshairVisibility(bool bVisible);
//
// UFUNCTION(BlueprintCallable, Category = "HUD")
// void ShowInteractionPrompt(const FText& InteractText);
//
// UFUNCTION(BlueprintCallable, Category = "HUD")
// void HideInteractionPrompt();