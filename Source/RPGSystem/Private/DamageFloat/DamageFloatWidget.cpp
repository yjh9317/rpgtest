// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageFloat/DamageFloatWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "DamageFloat/DamageFloatManagerComponent.h"

void UDamageFloatWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	APlayerController* PC = GetOwningPlayer();
	if (!IsValid(PC)) return;

	UDamageFloatManagerComponent* Manager = PC->FindComponentByClass<UDamageFloatManagerComponent>();
	if (!IsValid(Manager)) return;

	// Clear previous frame's widgets
	CP_DamageFloat->ClearChildren();

	const TArray<FDamageFloatData>& ActiveFloats = Manager->GetActiveDamageFloats();

	for (const FDamageFloatData& FloatData : ActiveFloats)
	{
		// ✅ GetCurrentWorldPosition()이 SpawnWorldPosition + Offsets 반환
		FVector CurrentWorldPos = FloatData.GetCurrentWorldPosition();
        
		FVector2D ScreenPosition;
		if (PC->ProjectWorldLocationToScreen(CurrentWorldPos, ScreenPosition))
		{
			// Create text widget
			UTextBlock* DamageText = NewObject<UTextBlock>(this);
			FText DamageDisplayText = FText::AsNumber(FMath::RoundToInt(FloatData.DamageAmount));
            
			DamageText->SetText(DamageDisplayText);
			DamageText->SetColorAndOpacity(FloatData.TextColor);
            
			// Critical hits: larger font
			if (FloatData.bIsCritical)
			{
				FSlateFontInfo CritFont = DamageText->GetFont();
				CritFont.Size = 48;
				DamageText->SetFont(CritFont);
			}

			// Add to canvas
			UCanvasPanelSlot* CPSlot = CP_DamageFloat->AddChildToCanvas(DamageText);
			CPSlot->SetPosition(ScreenPosition);
			CPSlot->SetAlignment(FVector2D(0.5f, 0.5f)); // Center pivot
		}
	}
}
