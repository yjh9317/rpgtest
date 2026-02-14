// CharacterEquipmentSlotWidget.cpp
#include "Equipment/UI/CharacterEquipmentSlotWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Components/Button.h"
#include "Equipment/EquipmentComponent.h"
#include "Item/Data/ItemInstance.h"
#include "Item/Data/ItemDefinition.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Input/Reply.h"

// 만약 드래그 드롭 오퍼레이션 클래스가 따로 있다면 include
// #include "Inventory/UI/InventoryDragDropOperation.h" 

void UCharacterEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (Image_Icon)
	{
		// 중요: Hidden이 아니라 HitTestInvisible로 해야 보이지만 클릭은 뒤(버튼)로 넘어감
		Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UCharacterEquipmentSlotWidget::InitSlot(UEquipmentComponent* InEquipComp, FGameplayTag InSlotTag)
{
	EquipmentComp = InEquipComp;
	SlotTag = InSlotTag;
}

void UCharacterEquipmentSlotWidget::UpdateSlot(const UItemInstance* InItem)
{
	CachedItem = InItem;

	if (Image_Icon)
	{
		if (InItem && InItem->GetItemDef())
		{
			// 아이템이 있으면 아이콘 표시
			TSoftObjectPtr<UTexture2D> Icon = InItem->GetItemDef()->Icon;
			if (!Icon.IsNull())
			{
				Image_Icon->SetBrushFromSoftTexture(Icon);
				Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
		else
		{
			// 아이템이 없으면 숨김 (또는 투명/기본이미지)
			// Image_Icon->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}


bool UCharacterEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	// 드래그된 객체가 인벤토리 아이템인지 확인 (프로젝트에 맞는 DragDropOperation 캐스팅 필요)
	// UInventoryDragDropOperation* DragOp = Cast<UInventoryDragDropOperation>(InOperation);
	
	// 여기서는 예시로 Payload가 UItemInstance라고 가정합니다. (실제 구현에 맞춰 수정 필요)
	UObject* Payload = InOperation ? InOperation->Payload : nullptr;
	UItemInstance* DroppedItem = Cast<UItemInstance>(Payload);

	if (EquipmentComp && DroppedItem)
	{
		// 장착 시도 (EquipmentComponent가 내부적으로 슬롯 확인 및 교체 처리)
		return EquipmentComp->EquipItem(DroppedItem);
	}

	return false;
}

FReply UCharacterEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 우클릭 시 장착 해제
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (EquipmentComp && CachedItem)
		{
			EquipmentComp->UnequipItem(SlotTag);
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UCharacterEquipmentSlotWidget::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	if (Txt_Slot)
	{
		Txt_Slot->SetText(SlotText);
	}
}

void UCharacterEquipmentSlotWidget::OnSlotButtonClicked()
{
	
}
