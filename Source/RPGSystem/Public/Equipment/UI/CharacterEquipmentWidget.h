// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "CharacterEquipmentWidget.generated.h"

class UItemInstance;
class UInventoryCoreComponent;
class UEquipmentComponent;
class UCharacterEquipmentSlotWidget;

/**
 * 6 vs 6 WoW Style Layout (Updated)
 */
UCLASS()
class RPGSYSTEM_API UCharacterEquipmentWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void InitializeWidget(UEquipmentComponent* InEquipComp);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void SynchronizeProperties() override;
	UFUNCTION()
	void OnEquipmentUpdated(FGameplayTag SlotTag, const UItemInstance* ItemInstance);

protected:
	// === 왼쪽 (Left Side - 6 Slots) ===
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Head;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Neck;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Shoulder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Chest;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Wrist;

	// === 오른쪽 (Right Side - 6 Slots) ===
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Hands;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Waist;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Legs;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Feet;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Ring;    

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_Trinket;

	// === 하단 (Weapons - 2 Slots) ===
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_MainHand;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCharacterEquipmentSlotWidget> Slot_OffHand;
	
	// 태그로 위젯을 빠르게 찾기 위한 룩업 테이블
	UPROPERTY()
	TMap<FGameplayTag, UCharacterEquipmentSlotWidget*> SlotWidgetMap;

	UPROPERTY()
	TObjectPtr<UEquipmentComponent> EquipmentComp;
};