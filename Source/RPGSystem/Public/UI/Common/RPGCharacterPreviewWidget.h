// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RPGCharacterPreviewWidget.generated.h"

class ARPGCharacterCaptureActor;
class UImage;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGCharacterPreviewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_RenderView;
	
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
	
protected:
	// 실제 월드 어딘가(지하 감옥 등)에 숨겨진 캡처 전용 액터를 참조
	UPROPERTY(EditAnywhere, Category = "Setup")
	TSubclassOf<ARPGCharacterCaptureActor> PreviewCaptureActorClass;
	
private:
	
};
