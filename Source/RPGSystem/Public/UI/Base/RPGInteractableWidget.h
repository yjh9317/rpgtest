// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RPGInteractableWidget.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGInteractableWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Style")
	FButtonStyle WidgetStyle;

	UPROPERTY(meta = (BindWidget))
	class UBorder* Border_Background;

protected:
	void UpdateVisualStyle();

	bool bIsHovered = false;
	bool bIsPressed = false;

	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
};
