// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageFloatWidget.generated.h"

class UCanvasPanel;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UDamageFloatWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CP_DamageFloat;
};
