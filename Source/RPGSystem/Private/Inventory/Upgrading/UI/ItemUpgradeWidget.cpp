// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/Upgrading/UI/ItemUpgradeWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/RichTextBlock.h" 
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/InventoryFunctionLibrary.h"
#include "Item/Data/ItemInstance.h"
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/Fragment/ItemFragment_Stats.h"
#include "Item/Data/Fragment/ItemFragment_Upgradable.h"

void UItemUpgradeWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UItemUpgradeWidget::SetupUpgradeWindow(UInventoryCoreComponent* InInventory, UUpgradingComponent* InUpgradingComp, FGuid InInventoryGuid, int32 InSlotIndex)
{
	InventoryComp = InInventory;
	UpgradingComp = InUpgradingComp;
	TargetInventoryGuid = InInventoryGuid;
	TargetSlotIndex = InSlotIndex;

	if (UpgradingComp)
	{
		UpgradingComp->OnUpgradeSuccess.AddDynamic(this, &UItemUpgradeWidget::HandleUpgradeSuccess);
	}

	RefreshUI();
}


void UItemUpgradeWidget::RefreshUI()
{
	if (!InventoryComp || !UpgradingComp) return;

	FInventorySlot* InventorySlot = InventoryComp->GetSlot(TargetInventoryGuid, TargetSlotIndex);
	if (!InventorySlot || InventorySlot->IsEmpty()) 
	{
		// 창 닫기 처리
		RemoveFromParent();
		return;
	}

	UItemInstance* CurrentItem = InventorySlot->ItemInstance;
	
	// 1. 아이콘 및 이름 설정
	if (CurrentItem->GetItemDef())
	{
		UInventoryFunctionLibrary::LoadItemIconAsync(CurrentItem->GetItemDef(), Img_ItemIcon);
		Txt_ItemName->SetText(CurrentItem->GetItemDef()->ItemName);
	}

	// 2. 레시피 가져오기
	TArray<FUpgradeRecipe> Recipes;
	bool bHasUpgrade = UpgradingComp->GetAvailableUpgrades(InventoryComp, TargetInventoryGuid, TargetSlotIndex, Recipes);

	if (bHasUpgrade && Recipes.IsValidIndex(SelectedRecipeIndex))
	{
		const FUpgradeRecipe& Recipe = Recipes[SelectedRecipeIndex];
		
		// 결과 아이템 로드 (동기 로드 주의, 필요시 비동기로 변경)
		UItemDefinition* NextItemDef = Recipe.ResultItemDef.LoadSynchronous();

		// 스탯 비교 UI 업데이트
		UpdateStatComparison(CurrentItem, NextItemDef);

		// 비용 UI 업데이트
		UpdateCostList(Recipe.Costs);

		Btn_Upgrade->SetIsEnabled(true);
	}
	else
	{
		// 업그레이드 불가 상태 (Max Level 등)
		VB_NextStats->ClearChildren();
		VB_CostList->ClearChildren();
		Btn_Upgrade->SetIsEnabled(false);
		// "Max Level" 텍스트 표시 등 처리
	}
}

void UItemUpgradeWidget::UpdateStatComparison(const UItemInstance* CurrentItem, const UItemDefinition* NextItemDef)
{
	VB_CurrentStats->ClearChildren();
	VB_NextStats->ClearChildren();

	if (!CurrentItem || !NextItemDef) return;

	// 현재 스탯 가져오기 (Instance의 런타임 데이터)
	const UItemFragment_Stats* CurrentStats = CurrentItem->FindFragmentByClass<UItemFragment_Stats>();
	
	// 다음 스탯 가져오기 (Definition의 기본 데이터)
	// 주의: Definition에는 런타임 변동치(인챈트 등)가 없을 수 있으므로, 기본 BaseValue를 비교합니다.
	const UItemFragment_Stats* NextStatsDef = NextItemDef->FindFragmentByClass<UItemFragment_Stats>();

	// 비교를 위해 모든 스탯 태그 수집
	TSet<FGameplayTag> AllStatTags;
	if (CurrentStats)
	{
		for (const FItemStat& Stat : CurrentStats->Stats) AllStatTags.Add(Stat.StatTag);
	}
	if (NextStatsDef)
	{
		for (const FItemStat& Stat : NextStatsDef->Stats) AllStatTags.Add(Stat.StatTag);
	}

	// 각 스탯별로 UI 행 생성
	for (const FGameplayTag& Tag : AllStatTags)
	{
		float ValCurrent = CurrentStats ? CurrentStats->GetStatValue(Tag) : 0.0f;
		float ValNext = NextStatsDef ? NextStatsDef->GetStatValue(Tag) : 0.0f;
		float Diff = ValNext - ValCurrent;

		// [Left Panel] 현재 값 표시
		UTextBlock* CurrentText = NewObject<UTextBlock>(this);
		CurrentText->SetText(FText::Format(FText::FromString("{0}: {1}"), FText::FromName(Tag.GetTagName()), FText::AsNumber(ValCurrent)));
		VB_CurrentStats->AddChild(CurrentText);

		// [Right Panel] 미리보기 값 표시 (RichText로 색상 처리)
		// 예: "<StatName>Agility</> 150 <Green>(+22)</>"
		FString ComparisonStr;
		if (Diff > 0)
		{
			// 상승 (초록색)
			ComparisonStr = FString::Printf(TEXT("%s %.0f <Green>(+%.0f)</>"), *Tag.GetTagName().ToString(), ValNext, Diff);
		}
		else if (Diff < 0)
		{
			// 하락 (빨간색)
			ComparisonStr = FString::Printf(TEXT("%s %.0f <Red>(%.0f)</>"), *Tag.GetTagName().ToString(), ValNext, Diff);
		}
		else
		{
			// 동일 (흰색)
			ComparisonStr = FString::Printf(TEXT("%s %.0f"), *Tag.GetTagName().ToString(), ValNext);
		}

		// RichTextBlock 생성 (RichTextStyle 데이터 테이블이 설정되어 있어야 함)
		// 편의상 여기서는 UTextBlock을 사용하고 색상만 변경하거나, 실제 구현 시 URichTextBlock 사용 권장
		UTextBlock* NextText = NewObject<UTextBlock>(this);
		NextText->SetText(FText::FromString(ComparisonStr)); // 실제로는 RichText 파싱 필요
		if(Diff > 0) NextText->SetColorAndOpacity(FLinearColor::Green);
		VB_NextStats->AddChild(NextText);
	}
}

void UItemUpgradeWidget::UpdateCostList(const TArray<FUpgradeCost>& Costs)
{
	VB_CostList->ClearChildren();

	for (const FUpgradeCost& Cost : Costs)
	{
		if (Cost.ItemDef.IsNull()) continue;

		UItemDefinition* MatDef = Cost.ItemDef.LoadSynchronous();
		
		// 보유량 확인 (InventoryCoreComponent의 기능 활용)
		int32 OwnedAmount = 0;
		for(const FGuid& Guid : InventoryComp->GetAllInventoryGuids())
		{
			OwnedAmount += InventoryComp->CountItemByDef(Guid, MatDef);
		}

		// UI 생성 (아이콘 + 보유량/필요량)
		// 예: [Icon] Iron Ingot 5 / 10
		FString CostStr = FString::Printf(TEXT("%s: %d / %d"), *MatDef->ItemName.ToString(), OwnedAmount, Cost.Quantity);
		
		UTextBlock* CostText = NewObject<UTextBlock>(this);
		CostText->SetText(FText::FromString(CostStr));
		
		// 재료 부족 시 빨간색 표시
		if (OwnedAmount < Cost.Quantity)
		{
			CostText->SetColorAndOpacity(FLinearColor::Red);
			Btn_Upgrade->SetIsEnabled(false); // 재료 부족하면 버튼 비활성
		}
		else
		{
			CostText->SetColorAndOpacity(FLinearColor::White);
		}

		VB_CostList->AddChild(CostText);
	}
}

void UItemUpgradeWidget::OnUpgradeButtonClicked()
{
	if (UpgradingComp && InventoryComp)
	{
		// 업그레이드 시도
		UpgradingComp->TryUpgradeItem(InventoryComp, TargetInventoryGuid, TargetSlotIndex, SelectedRecipeIndex);
	}
}

void UItemUpgradeWidget::HandleUpgradeSuccess(const UItemDefinition* NewItemDef)
{
	// 업그레이드 성공 시 UI 갱신 (또는 닫기)
	// RefreshUI를 호출하여 변경된 아이템(결과물)을 다시 로드하여 다음 업그레이드를 보여줄 수도 있음
	RefreshUI();
	
	// 사운드 재생 등 추가 연출
}

void UItemUpgradeWidget::NativeDestruct()
{
	if (UpgradingComp)
	{
		UpgradingComp->OnUpgradeSuccess.RemoveDynamic(this, &UItemUpgradeWidget::HandleUpgradeSuccess);
	}
	Super::NativeDestruct();
}
