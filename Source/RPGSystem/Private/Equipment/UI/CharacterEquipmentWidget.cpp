// Fill out your copyright notice in the Description page of Project Settings.

#include "Equipment/UI/CharacterEquipmentWidget.h"
#include "Equipment/EquipmentComponent.h"
#include "Equipment/UI/CharacterEquipmentSlotWidget.h"
#include "RPGSystemGameplayTags.h" // ⭐ 이 헤더를 꼭 포함해야 합니다!

void UCharacterEquipmentWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (EquipmentComp)
	{
		EquipmentComp->OnEquipmentUpdated.RemoveAll(this);
	}
}

void UCharacterEquipmentWidget::NativeDestruct()
{
	if (EquipmentComp)
	{
		EquipmentComp->OnEquipmentUpdated.RemoveAll(this);
	}
	Super::NativeDestruct();
}
void UCharacterEquipmentWidget::InitializeWidget(UEquipmentComponent* InEquipComp)
{
	EquipmentComp = InEquipComp;
	if (!EquipmentComp) return;
	
	SlotWidgetMap.Empty();

	// ⭐ 네임스페이스를 사용해 태그에 직접 접근 (Tags.XXX 아님)
	using namespace RPGGameplayTags; 

	// === Left Side (6) ===
	if(Slot_Head)     SlotWidgetMap.Add(Equipment_Slot_Head,     Slot_Head);
	if(Slot_Neck)     SlotWidgetMap.Add(Equipment_Slot_Neck,     Slot_Neck); 
	if(Slot_Shoulder) SlotWidgetMap.Add(Equipment_Slot_Shoulder, Slot_Shoulder);
	if(Slot_Back)     SlotWidgetMap.Add(Equipment_Slot_Back,     Slot_Back);
	if(Slot_Chest)    SlotWidgetMap.Add(Equipment_Slot_Chest,    Slot_Chest);
	if(Slot_Wrist)    SlotWidgetMap.Add(Equipment_Slot_Wrist,    Slot_Wrist);

	// === Right Side (6) ===
	if(Slot_Hands)    SlotWidgetMap.Add(Equipment_Slot_Hands,    Slot_Hands);
	if(Slot_Waist)    SlotWidgetMap.Add(Equipment_Slot_Waist,    Slot_Waist);
	if(Slot_Legs)     SlotWidgetMap.Add(Equipment_Slot_Legs,     Slot_Legs);
	if(Slot_Feet)     SlotWidgetMap.Add(Equipment_Slot_Feet,     Slot_Feet);
	if(Slot_Ring)     SlotWidgetMap.Add(Equipment_Slot_Ring,     Slot_Ring);
	if(Slot_Trinket)  SlotWidgetMap.Add(Equipment_Slot_Trinket,  Slot_Trinket);

	// === Weapons ===
	if(Slot_MainHand) SlotWidgetMap.Add(Equipment_Slot_MainHand, Slot_MainHand);
	if(Slot_OffHand)  SlotWidgetMap.Add(Equipment_Slot_OffHand,  Slot_OffHand);

	for (auto& Pair : SlotWidgetMap)
	{
		FGameplayTag Tag = Pair.Key;
		UCharacterEquipmentSlotWidget* Widget = Pair.Value;

		if (Widget)
		{
			Widget->InitSlot(EquipmentComp, Tag);
			UItemInstance* EquippedItem = EquipmentComp->GetEquippedItem(Tag);
			Widget->UpdateSlot(EquippedItem);
		}
	}

	EquipmentComp->OnEquipmentUpdated.RemoveAll(this);
	EquipmentComp->OnEquipmentUpdated.AddDynamic(this, &UCharacterEquipmentWidget::OnEquipmentUpdated);
}

void UCharacterEquipmentWidget::OnEquipmentUpdated(FGameplayTag SlotTag, const UItemInstance* ItemInstance)
{
	// 해당 태그를 가진 슬롯 위젯을 찾아서 업데이트
	if (UCharacterEquipmentSlotWidget** FoundWidgetPtr = SlotWidgetMap.Find(SlotTag))
	{
		UCharacterEquipmentSlotWidget* Widget = *FoundWidgetPtr;
		if (Widget)
		{
			// ItemInstance가 nullptr이면 빈 슬롯(아이콘 숨김) 처리됨
			Widget->UpdateSlot(ItemInstance);
		}
	}
}