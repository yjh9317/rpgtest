// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Crafting/UI/CraftingWindowWidget.h"
#include "Inventory/Crafting/UI/CraftingReagentWidget.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/Crafting/CraftingProcessorComponent.h"
#include "Inventory/InventoryFunctionLibrary.h"
#include "Item/Data/ItemDefinition.h" 

// [수정] UMG 컴포넌트 헤더 추가 (필수)
#include "Components/Button.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"

void UCraftingWindowWidget::InitializeCraftingWindow(UInventoryCoreComponent* Inventory, UCraftingProcessorComponent* Processor, const TArray<UCraftingRecipe*>& Recipes)
{
	InventoryComp = Inventory;
	ProcessorComp = Processor;

	if (ProcessorComp)
	{
		// 진행률 및 완료 이벤트 바인딩
		ProcessorComp->OnCraftingProgress.AddUObject(this, &UCraftingWindowWidget::UpdateCraftingProgress);
		ProcessorComp->OnJobFinished.AddUObject(this, &UCraftingWindowWidget::OnCraftingFinished);
	}

	PopulateRecipeList(Recipes);
	
	if (PB_CraftingProgress)
	{
		PB_CraftingProgress->SetPercent(0.0f);
	}
    
	if (Btn_Create)
	{
		Btn_Create->SetIsEnabled(false);
	}
}

void UCraftingWindowWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCraftingWindowWidget::PopulateRecipeList(const TArray<UCraftingRecipe*>& Recipes)
{
	RecipeListContainer->ClearChildren();

	// 실제 구현 시에는 CategoryTag 별로 그룹화하여 TreeView나 ExpandableArea를 사용합니다.
	// 여기서는 단순화를 위해 리스트로 나열합니다.
	for (UCraftingRecipe* Recipe : Recipes)
	{
		if (!Recipe) continue;

		// RecipeEntryWidget 생성 및 데이터 주입 (블루프린트에서 OnClicked 이벤트를 통해 OnRecipeSelected 호출하도록 설정)
		// 예: CreateWidget<URecipeEntry>(...)->Setup(Recipe, this);
		// RecipeListContainer->AddChild(...);
	}
}

void UCraftingWindowWidget::OnRecipeSelected(UCraftingRecipe* Recipe)
{
	SelectedRecipe = Recipe;
	if (!Recipe || Recipe->Outputs.IsEmpty()) return;

	// 1. 결과 아이템 정보 표시
	// Outputs[0]이 메인 결과물이라고 가정
	const FCraftingOutput& MainOutput = Recipe->Outputs[0];
	UItemDefinition* ResultDef = MainOutput.ItemDef.LoadSynchronous(); // 아이콘 로드

	if (ResultDef)
	{
		UInventoryFunctionLibrary::LoadItemIconAsync(ResultDef, Img_ResultIcon);
		if (Txt_ResultName)
		{
			Txt_ResultName->SetText(ResultDef->ItemName);
		}
	}

	// 2. 재료 리스트 갱신 (핵심)
	VB_ReagentsList->ClearChildren();
	bool bCanCraft = true;

	for (const FCraftingIngredient& Ingredient : Recipe->Ingredients)
	{
		if (!ReagentWidgetClass) continue;

		UCraftingReagentWidget* ReagentWidget = CreateWidget<UCraftingReagentWidget>(this, ReagentWidgetClass);
		if (ReagentWidget)
		{
			// 재료 위젯 초기화 (내부에서 InventoryComp를 조회하여 수량 체크)
			ReagentWidget->InitReagent(Ingredient, InventoryComp);
			VB_ReagentsList->AddChild(ReagentWidget);

			// 제작 가능 여부 체크
			UItemDefinition* MatDef = Ingredient.ItemDef.LoadSynchronous();
			int32 Owned = 0;
			// InventoryCoreComponent의 전체 인벤토리 검색 기능을 활용
			for(const FGuid& Guid : InventoryComp->GetAllInventoryGuids())
			{
				Owned += InventoryComp->CountItemByDef(Guid, MatDef);
			}

			if (Owned < Ingredient.Quantity)
			{
				bCanCraft = false;
			}
		}
	}

	// 3. 버튼 상태 업데이트
	Btn_Create->SetIsEnabled(bCanCraft && !ProcessorComp->IsActive()); // 이미 제작 중이면 비활성
}

void UCraftingWindowWidget::OnCreateButtonClicked()
{
	if (SelectedRecipe && InventoryComp && ProcessorComp)
	{
		// 제작 요청
		bool bStarted = ProcessorComp->RequestCrafting(SelectedRecipe, InventoryComp);
		
		if (bStarted)
		{
			Btn_Create->SetIsEnabled(false); // 중복 클릭 방지
		}
	}
}

void UCraftingWindowWidget::UpdateCraftingProgress(float Progress)
{
	if (PB_CraftingProgress)
	{
		PB_CraftingProgress->SetPercent(Progress);
	}
}

void UCraftingWindowWidget::OnCraftingFinished(const UCraftingRecipe* Recipe)
{
	// 제작 완료 후 UI 갱신 (재료가 소모되었으므로 재료 목록 다시 체크)
	OnRecipeSelected(SelectedRecipe);
	PB_CraftingProgress->SetPercent(0.0f);
}

void UCraftingWindowWidget::NativeDestruct()
{
	if (ProcessorComp)
	{
		ProcessorComp->OnCraftingProgress.RemoveAll(this);
		ProcessorComp->OnJobFinished.RemoveAll(this);
	}
	Super::NativeDestruct();
}