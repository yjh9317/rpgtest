// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemDragAndDropWidget.generated.h"

class UImage;
class UTexture2D;

UCLASS()
class RPGSYSTEM_API UItemDragAndDropWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitWidget(UTexture2D* InImage, const FVector2D& InSize);
    
	void ShowDropState();
	void ShowSwapState();
	void ShowWrongState();
	void ClearState();
	
protected:
	virtual void NativeConstruct() override;

protected:
	// =========================================================
	// UI 컴포넌트 바인딩
	// =========================================================

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_DropState;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_SwapState;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_WrongState;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DragDrop")
	FVector2D IconSize = FVector2D(64.f, 64.f);
};
