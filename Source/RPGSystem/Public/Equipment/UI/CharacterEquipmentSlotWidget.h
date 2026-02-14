// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "Item/Data/ItemDataStructure.h"
#include "CharacterEquipmentSlotWidget.generated.h"


class UTextBlock;
class UButton;
class UImage;
class UEquipmentComponent;

UCLASS()
class RPGSYSTEM_API UCharacterEquipmentSlotWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitSlot(UEquipmentComponent* InEquipComp, FGameplayTag InSlotTag);
	void UpdateSlot(const UItemInstance* InItem);

	UFUNCTION()
	void OnSlotButtonClicked();
protected:
	virtual void NativeConstruct() override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void SynchronizeProperties() override;
protected:
	// [UI] 아이콘 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FGameplayTag SlotTag;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Txt_Slot;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FText SlotText;

	// 현재 표시 중인 아이템 (툴팁 등을 위해 캐싱)
	UPROPERTY(BlueprintReadOnly, Category = "State")
	TObjectPtr<const UItemInstance> CachedItem;

	UPROPERTY()
	TObjectPtr<UEquipmentComponent> EquipmentComp;

};
