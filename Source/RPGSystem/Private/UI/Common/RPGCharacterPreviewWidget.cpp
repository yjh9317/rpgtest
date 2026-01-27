// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Common/RPGCharacterPreviewWidget.h"

FReply URPGCharacterPreviewWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FVector2D MouseDelta = InMouseEvent.GetCursorDelta();

		if (MouseDelta.X != 0.0f)
		{
			// GetOwningPlayer()->
		}
		return FReply::Handled();
	}

	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply URPGCharacterPreviewWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
                                                           const FPointerEvent& InMouseEvent)
{
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void URPGCharacterPreviewWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
}
