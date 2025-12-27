// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/Crafting/Data/CraftingRecipe.h"
#include "CraftingReagentWidget.generated.h"

class UImage;
class UTextBlock;
class UInventoryCoreComponent;

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UCraftingReagentWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 재료 슬롯 데이터 설정
	void InitReagent(const FCraftingIngredient& Ingredient, UInventoryCoreComponent* Inventory);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Name;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Quantity; // "5 / 10" 형태

	// 보유량이 충분한지 여부에 따라 텍스트 색상 변경 (Red/Green)
	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor Color_Available = FLinearColor::Green;

	UPROPERTY(EditDefaultsOnly, Category = "Style")
	FLinearColor Color_Insufficient = FLinearColor::Red;
};
