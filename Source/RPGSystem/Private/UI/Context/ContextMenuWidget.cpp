// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Context/ContextMenuWidget.h"
#include "UI/Context/ContextMenuEntryWidget.h"
#include "Components/VerticalBox.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void UContextMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (Btn_BackgroundClose)
	{
		Btn_BackgroundClose->OnClicked.AddDynamic(this, &UContextMenuWidget::OnBackgroundClicked);
	}
}

void UContextMenuWidget::AddMenuItem(const FContextMenuItemData& ItemData)
{
	if (!EntryWidgetClass || !VB_MenuList) return;

	UContextMenuEntryWidget* NewEntry = CreateWidget<UContextMenuEntryWidget>(this, EntryWidgetClass);
	if (NewEntry)
	{
		NewEntry->SetupEntry(ItemData);
		NewEntry->ParentMenu = this;
		VB_MenuList->AddChild(NewEntry);
	}
}

void UContextMenuWidget::ShowAtMousePosition()
{
	AddToViewport(100); // 높은 Z-Order

	// 마우스 위치 가져오기
	float LocationX, LocationY;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (PC->GetMousePosition(LocationX, LocationY))
		{
			// MenuContainer의 위치 설정 (CanvasPanel Slot 가정)
			if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MenuContainer->Slot))
			{
				// DPI 스케일 고려 필요 시 UWidgetLayoutLibrary::GetViewportScale(this) 활용
				CanvasSlot->SetPosition(FVector2D(LocationX, LocationY));
			}
		}
	}
}

void UContextMenuWidget::CloseMenu()
{
	RemoveFromParent();
}

void UContextMenuWidget::OnBackgroundClicked()
{
	CloseMenu();
}
