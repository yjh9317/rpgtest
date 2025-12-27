// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/DragDrop/ItemDragAndDropWidget.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"

void UItemDragAndDropWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시 모든 상태 아이콘 숨김
	ClearState();
}

void UItemDragAndDropWidget::InitWidget(UTexture2D* InImage, const FVector2D& InSize)
{
	IconSize = InSize;

	if (Image_ItemIcon)
	{
		if (InImage)
		{
			Image_ItemIcon->SetBrushFromTexture(InImage);
			Image_ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
        
		// 아이콘 크기 설정 (원하는 경우 Brush 사이즈를 강제할 수 있음)
		// FSlateBrush Brush = Image_ItemIcon->GetBrush();
		// Brush.ImageSize = IconSize;
		// Image_ItemIcon->SetBrush(Brush);
	}
}

void UItemDragAndDropWidget::ShowDropState()
{
	ClearState();
	if (Image_DropState)
	{
		Image_DropState->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UItemDragAndDropWidget::ShowSwapState()
{
	ClearState();
	if (Image_SwapState)
	{
		Image_SwapState->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UItemDragAndDropWidget::ShowWrongState()
{
	ClearState();
	if (Image_WrongState)
	{
		Image_WrongState->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}
}

void UItemDragAndDropWidget::ClearState()
{
	if (Image_DropState) Image_DropState->SetVisibility(ESlateVisibility::Collapsed);
	if (Image_SwapState) Image_SwapState->SetVisibility(ESlateVisibility::Collapsed);
	if (Image_WrongState) Image_WrongState->SetVisibility(ESlateVisibility::Collapsed);
}
