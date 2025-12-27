// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Context/ContextMenuEntryWidget.h"
// #include "UI/ContextMenuWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

void UContextMenuEntryWidget::SetupEntry(const FContextMenuItemData& InData)
{
	ItemData = InData;

	if (Switcher_Type)
	{
		Switcher_Type->SetActiveWidgetIndex((int32)InData.Type);
	}

	// 타입별 설정
	switch (InData.Type)
	{
	case EContextMenuItemType::Button:
		if (Txt_ButtonLabel) Txt_ButtonLabel->SetText(InData.Text);
		break;
	case EContextMenuItemType::Checkbox:
		if (Txt_CheckboxLabel) Txt_CheckboxLabel->SetText(InData.Text);
		if (CB_Toggle) CB_Toggle->SetIsChecked(InData.bIsChecked);
		break;
	case EContextMenuItemType::Header:
		if (Txt_HeaderLabel) Txt_HeaderLabel->SetText(InData.Text);
		break;
	case EContextMenuItemType::Separator:
		// 별도 텍스트 없음
		break;
	}
}

void UContextMenuEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Btn_Action) Btn_Action->OnClicked.AddDynamic(this, &UContextMenuEntryWidget::OnBtnClicked);
	if (CB_Toggle) CB_Toggle->OnCheckStateChanged.AddDynamic(this, &UContextMenuEntryWidget::OnCheckboxChanged);
}

void UContextMenuEntryWidget::OnBtnClicked()
{
	// 델리게이트 실행
	ItemData.OnClicked.ExecuteIfBound();

	// 부모 메뉴 닫기
	if (ParentMenu.IsValid())
	{
		// ParentMenu->CloseMenu();
	}
}

void UContextMenuEntryWidget::OnCheckboxChanged(bool bNewState)
{
	// 체크박스는 상태가 변하되 메뉴를 닫지 않는 것이 일반적입니다 (WoW 방식).
	// 원한다면 닫게 할 수도 있습니다.
	ItemData.OnClicked.ExecuteIfBound();
}
