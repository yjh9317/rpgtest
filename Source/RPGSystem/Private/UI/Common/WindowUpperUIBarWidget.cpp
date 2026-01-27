// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/WindowUpperUIBarWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"

void UWindowUpperUIBarWidget::NativePreConstruct()
{
	if (SB_Root)
	{
		SB_Root->SetWidthOverride(BarWidth);
		SB_Root->SetHeightOverride(BarHeight);
	}

	SetActiveTab(EWindowTabType::None);
}

void UWindowUpperUIBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Inventory) Btn_Inventory->OnClicked.AddDynamic(this, &UWindowUpperUIBarWidget::OnBtnInvClicked);
	if (Btn_Crafting) Btn_Crafting->OnClicked.AddDynamic(this, &UWindowUpperUIBarWidget::OnBtnCraftClicked);
	if (Btn_Ability) Btn_Ability->OnClicked.AddDynamic(this, &UWindowUpperUIBarWidget::OnBtnAbilClicked);
	if (Btn_WorldMap) Btn_WorldMap->OnClicked.AddDynamic(this, &UWindowUpperUIBarWidget::OnBtnMapClicked);
	if (Btn_Quest) Btn_Quest->OnClicked.AddDynamic(this, &UWindowUpperUIBarWidget::OnBtnQuestClicked);

	if (Btn_ArrowLeft) Btn_ArrowLeft->OnClicked.AddDynamic(this, &UWindowUpperUIBarWidget::OnBtnPrevClicked);
	if (Btn_ArrowRight) Btn_ArrowRight->OnClicked.AddDynamic(this, &UWindowUpperUIBarWidget::OnBtnNextClicked);
    
    // 호버 이벤트는 블루프린트에서 Visual State로 처리하거나, 
    // 필요하다면 여기서 OnHovered 델리게이트를 추가로 바인딩해도 됩니다.
    // 여기서는 클릭(선택) 상태 로직 위주로 구현합니다.
}

void UWindowUpperUIBarWidget::SetActiveTab(EWindowTabType NewTab)
{
	CurrentTab = NewTab;

	UpdateButtonState(Btn_Inventory, Txt_Inventory, CurrentTab == EWindowTabType::Inventory, false);
	UpdateButtonState(Btn_Crafting, Txt_Crafting, CurrentTab == EWindowTabType::Crafting, false);
	UpdateButtonState(Btn_Ability, Txt_Ability, CurrentTab == EWindowTabType::Ability, false);
	UpdateButtonState(Btn_WorldMap, Txt_WorldMap, CurrentTab == EWindowTabType::WorldMap, false);
	UpdateButtonState(Btn_Quest, Txt_Quest, CurrentTab == EWindowTabType::Quest, false);
}

void UWindowUpperUIBarWidget::UpdateButtonState(UButton* InBtn, UTextBlock* InTxt, bool bIsSelected, bool bIsHovered)
{
	if (!InBtn || !InTxt) return;

	FLinearColor TargetColor = Color_Normal;
    
	if (bIsSelected)
	{
		TargetColor = Color_Selected;
	}
	else if (bIsHovered)
	{
		TargetColor = Color_Hover;
	}

	InBtn->SetBackgroundColor(TargetColor);

	FLinearColor TargetTextColor = bIsSelected ? TextColor_Selected : TextColor_Normal;
	InTxt->SetColorAndOpacity(FSlateColor(TargetTextColor));
    
    // 버튼 비활성화 처리
    InBtn->SetIsEnabled(!bIsSelected); 
}

void UWindowUpperUIBarWidget::OnBtnInvClicked()
{
	SetActiveTab(EWindowTabType::Inventory);
}

void UWindowUpperUIBarWidget::OnBtnCraftClicked()
{
	SetActiveTab(EWindowTabType::Crafting);
}

void UWindowUpperUIBarWidget::OnBtnAbilClicked()
{
	SetActiveTab(EWindowTabType::Ability);
}

void UWindowUpperUIBarWidget::OnBtnMapClicked()
{
	SetActiveTab(EWindowTabType::WorldMap);
}

void UWindowUpperUIBarWidget::OnBtnQuestClicked()
{
	SetActiveTab(EWindowTabType::Quest);
}

void UWindowUpperUIBarWidget::OnBtnPrevClicked()
{
	if (CurrentTab == EWindowTabType::None)
	{
		SetActiveTab((EWindowTabType)0);
		return;
	}

	uint8 CurrentIndex = (uint8)CurrentTab;

	// (CurrentIndex + 1)을 전체 개수(Count)로 나눈 나머지(%)를 구하면 
	// 자동으로 0 ~ (Count-1) 사이를 순환
	uint8 MaxCount = (uint8)EWindowTabType::Count;
	uint8 NextIndex = (CurrentIndex + 1) % MaxCount;

	SetActiveTab((EWindowTabType)NextIndex);
}

void UWindowUpperUIBarWidget::OnBtnNextClicked()
{
	uint8 MaxCount = (uint8)EWindowTabType::Count;
    
	if (CurrentTab == EWindowTabType::None)
	{
		SetActiveTab((EWindowTabType)(MaxCount - 1));
		return;
	}

	uint8 CurrentIndex = (uint8)CurrentTab;

	// 0번일 때 빼면 음수가 되므로, 삼항 연산자로 처리
	uint8 PrevIndex = (CurrentIndex == 0) ? (MaxCount - 1) : (CurrentIndex - 1);
	SetActiveTab((EWindowTabType)PrevIndex);
}
