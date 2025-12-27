// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Base/RPGInteractableWidget.h"

#include "Components/Border.h"

void URPGInteractableWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	bIsHovered = true;
	UpdateVisualStyle(); // 상태 변경 시 갱신
}

void URPGInteractableWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	bIsHovered = false;
	bIsPressed = false;
	
	if (Border_Background)
	{
		Border_Background->SetBrush(WidgetStyle.Normal);
	}
}

FReply URPGInteractableWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭일 때만 눌림 처리 (원하는 대로 커스텀 가능)
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = true;
		UpdateVisualStyle();
        
		// 여기서 DetectDrag를 반환하면 드래그 시작
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply URPGInteractableWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = false;
		UpdateVisualStyle();
		// 여기서 클릭 동작 실행 (OnClicked)
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void URPGInteractableWidget::UpdateVisualStyle()
{
	if (!Border_Background) return;

	FSlateBrush* TargetBrush = &WidgetStyle.Normal;

	if (bIsPressed)
	{
		TargetBrush = &WidgetStyle.Pressed;
	}
	else if (bIsHovered)
	{
		TargetBrush = &WidgetStyle.Hovered;
	}

	Border_Background->SetBrush(*TargetBrush);
}