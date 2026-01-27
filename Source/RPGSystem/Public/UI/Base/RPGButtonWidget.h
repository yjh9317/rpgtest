// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RPGButtonWidget.generated.h"

class USizeBox;
class UImage;
class UTextBlock;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRPGButtonClicked);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRPGButtonHovered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRPGButtonUnhovered);

UCLASS()
class RPGSYSTEM_API URPGButtonWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Background;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UTextBlock> Txt_Label;

	UPROPERTY(meta = (BindWidget, OptionalWidget = true))
	TObjectPtr<UImage> Img_Icon;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UTexture2D* BackgroundTexture;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UTexture2D* ButtonIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FText ButtonText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	bool bShowIcon;

	UPROPERTY(EditAnywhere, Category = "Config | Style")
	FLinearColor NormalColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "Config | Style")
	FLinearColor HoveredColor = FLinearColor(0.8f, 0.8f, 0.8f, 1.0f);

	UPROPERTY(EditAnywhere, Category = "Config | Style")
	FLinearColor PressedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Size")
	float ButtonWidth = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Size")
	float ButtonHeight = 0.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Style")
	FSlateFontInfo ButtonFont;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config | Style")
	FLinearColor TextColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> ClickSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	TObjectPtr<USoundBase> HoverSound;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRPGButtonClicked OnClicked;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRPGButtonHovered OnHovered;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRPGButtonUnhovered OnUnhovered;

protected:
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
	virtual void SynchronizeProperties() override;
	void SetTextLabel();
	void SetRootSizeBox();
	void UpdateWidgetLayout();
	virtual void NativePreConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Control")
	void SetButtonText(FText InText);
	
	UFUNCTION(BlueprintCallable, Category = "Control")
	void SetButtonIcon(UTexture2D* InIcon);

	UFUNCTION(BlueprintCallable, Category = "Control")
	void SetBackgroundTexture(UTexture2D* InIcon);
	
	UFUNCTION(BlueprintNativeEvent, Category = "DragDrop")
	UDragDropOperation* CreateDragDropOperation();
	virtual UDragDropOperation* CreateDragDropOperation_Implementation() { return nullptr; }

private:

	bool bIsPressed = false;
	bool bIsHovered = false;

	void UpdateButtonStyle();
};
