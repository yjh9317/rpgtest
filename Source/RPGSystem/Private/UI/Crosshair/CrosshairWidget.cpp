// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Crosshair/CrosshairWidget.h"

#include "Components/Overlay.h"

void UCrosshairWidget::CreateCrosshairWidget()
{
}

void UCrosshairWidget::UpdateCrosshair()
{
}

void UCrosshairWidget::RemoveCrosshair()
{
}

void UCrosshairWidget::AddCrosshair()
{
}

void UCrosshairWidget::HitFeedback()
{
}

void UCrosshairWidget::Recoil()
{
}

void UCrosshairWidget::FindCrosshair()
{
}

UWidget* UCrosshairWidget::GetCurrentCrosshair()
{
	if (UWidget* ChildWidget = OV_Content->GetChildAt(0))
	{
		return ChildWidget;
	}
	return nullptr;
}
