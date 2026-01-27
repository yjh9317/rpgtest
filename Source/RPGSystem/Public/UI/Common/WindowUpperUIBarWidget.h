// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WindowUpperUIBarWidget.generated.h"

class UImage;
class UButton;
class UTextBlock;
class USizeBox;
/**
 * 
 */
UENUM(BlueprintType)
enum class EWindowTabType : uint8
{
	Inventory = 0,
	Crafting,
	Ability,
	WorldMap,
	Quest,
	Count, 
    
	None
};

UCLASS()
class RPGSYSTEM_API UWindowUpperUIBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativePreConstruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "UI Action")
	void SetActiveTab(EWindowTabType NewTab);
	
protected:
	void UpdateButtonState(UButton* InBtn, UTextBlock* InTxt, bool bIsSelected, bool bIsHovered);

	UFUNCTION() void OnBtnInvClicked();
	UFUNCTION() void OnBtnCraftClicked();
	UFUNCTION() void OnBtnAbilClicked();
	UFUNCTION() void OnBtnMapClicked();
	UFUNCTION() void OnBtnQuestClicked();

	UFUNCTION() void OnBtnPrevClicked();
	UFUNCTION() void OnBtnNextClicked();
	
public:
	UPROPERTY(EditAnywhere, Category = "UI Config")
	float BarWidth = 0.0f;

	UPROPERTY(EditAnywhere, Category = "UI Config")
	float BarHeight = 80.0f;
	
	UPROPERTY(EditAnywhere, Category = "UI Config")
	FLinearColor Color_Normal = FLinearColor(0.f, 0.f, 0.f, 0.f); // 투명

	UPROPERTY(EditAnywhere, Category = "UI Config")
	FLinearColor Color_Hover = FLinearColor(0.2f, 0.2f, 0.2f, 0.8f); // 반투명 회색

	UPROPERTY(EditAnywhere, Category = "UI Config")
	FLinearColor Color_Selected = FLinearColor(1.0f, 0.8f, 0.2f, 1.0f); // 강조색 (금색 등)

	UPROPERTY(EditAnywhere, Category = "UI Config")
	FLinearColor TextColor_Normal = FLinearColor::White;

	UPROPERTY(EditAnywhere, Category = "UI Config")
	FLinearColor TextColor_Selected = FLinearColor::Black;
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SB_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_ArrowLeft;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_ArrowRight;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Inventory;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Inventory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Crafting;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Crafting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Ability;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Ability;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_WorldMap;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_WorldMap;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Quest;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Quest;
private:
	EWindowTabType CurrentTab = EWindowTabType::None;
};
