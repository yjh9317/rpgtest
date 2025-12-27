// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Crafting/Data/CraftingRecipe.h"
#include "CraftingWindowWidget.generated.h"

class UInventoryCoreComponent;
class UCraftingProcessorComponent;
class UTreeView;
class UButton;
class UProgressBar;
class UEditableTextBox;
class UVerticalBox;
class UImage;      
class UTextBlock;  
class UPanelWidget;

/**
 * WoW 스타일 제작 메인 윈도우
 */
UCLASS()
class RPGSYSTEM_API UCraftingWindowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// UI 초기화 (Open 시 호출)
	void InitializeCraftingWindow(UInventoryCoreComponent* Inventory, UCraftingProcessorComponent* Processor, const TArray<UCraftingRecipe*>& Recipes);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// --- Logic ---
	// 레시피 목록 생성 (카테고리별 분류)
	void PopulateRecipeList(const TArray<UCraftingRecipe*>& Recipes);

	// 레시피 선택 시 상세 정보 업데이트
	UFUNCTION()
	void OnRecipeSelected(UCraftingRecipe* Recipe);

	// 제작 버튼 클릭
	UFUNCTION()
	void OnCreateButtonClicked();

	// 제작 진행 상황 업데이트 (Processor 델리게이트 바인딩)
	UFUNCTION()
	void UpdateCraftingProgress(float Progress);

	UFUNCTION()
	void OnCraftingFinished(const UCraftingRecipe* Recipe);

protected:
	// --- Dependencies ---
	UPROPERTY()
	TObjectPtr<UInventoryCoreComponent> InventoryComp;

	UPROPERTY()
	TObjectPtr<UCraftingProcessorComponent> ProcessorComp;

	UPROPERTY()
	TObjectPtr<UCraftingRecipe> SelectedRecipe;

	// --- Widgets (Left Panel) ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> RecipeListContainer; // ScrollBox 권장
	
	// --- Widgets (Center/Detail Panel) ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_ResultIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_ResultName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> VB_ReagentsList; // 재료 위젯들이 들어갈 곳

	// --- Widgets (Bottom/Action Panel) ---
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> PB_CraftingProgress;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Create;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> Input_Quantity;

	// 재료 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Crafting UI")
	TSubclassOf<class UCraftingReagentWidget> ReagentWidgetClass;
	
	// 레시피 리스트 아이템 위젯 클래스 (버튼 역할)
	UPROPERTY(EditDefaultsOnly, Category = "Crafting UI")
	TSubclassOf<UUserWidget> RecipeEntryWidgetClass; 
};