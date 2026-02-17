// CharacterEquipmentSlotWidget.cpp
#include "Equipment/UI/CharacterEquipmentSlotWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Equipment/EquipmentComponent.h"
#include "Input/Reply.h"
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/ItemInstance.h"

namespace
{
	FText BuildSlotDisplayText(const FGameplayTag& InSlotTag)
	{
		if (!InSlotTag.IsValid())
		{
			return FText::GetEmpty();
		}

		const FString TagName = InSlotTag.GetTagName().ToString();
		int32 LastDotIndex = INDEX_NONE;
		if (!TagName.FindLastChar(TEXT('.'), LastDotIndex))
		{
			return FText::FromString(TagName);
		}

		const FString RawName = TagName.Mid(LastDotIndex + 1);
		FString PrettyName;
		PrettyName.Reserve(RawName.Len() + 4);

		for (int32 Index = 0; Index < RawName.Len(); ++Index)
		{
			const TCHAR Current = RawName[Index];
			const bool bInsertSpace = (Index > 0) && FChar::IsUpper(Current) && FChar::IsLower(RawName[Index - 1]);
			if (bInsertSpace)
			{
				PrettyName.AppendChar(TEXT(' '));
			}
			PrettyName.AppendChar(Current);
		}

		return FText::FromString(PrettyName);
	}
}

void UCharacterEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Image_Icon)
	{
		Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	if (Txt_Slot)
	{
		if (SlotText.IsEmpty() && SlotTag.IsValid())
		{
			SlotText = BuildSlotDisplayText(SlotTag);
		}
		Txt_Slot->SetText(SlotText);
	}
}

void UCharacterEquipmentSlotWidget::InitSlot(UEquipmentComponent* InEquipComp, FGameplayTag InSlotTag)
{
	EquipmentComp = InEquipComp;
	SlotTag = InSlotTag;
	SlotText = BuildSlotDisplayText(SlotTag);

	if (Txt_Slot)
	{
		Txt_Slot->SetText(SlotText);
	}
}

void UCharacterEquipmentSlotWidget::UpdateSlot(const UItemInstance* InItem)
{
	CachedItem = InItem;

	if (!Image_Icon)
	{
		return;
	}

	if (InItem && InItem->GetItemDef())
	{
		const TSoftObjectPtr<UTexture2D> Icon = InItem->GetItemDef()->Icon;
		if (!Icon.IsNull())
		{
			Image_Icon->SetBrushFromSoftTexture(Icon);
			Image_Icon->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
	}
}

bool UCharacterEquipmentSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	UObject* Payload = InOperation ? InOperation->Payload : nullptr;
	UItemInstance* DroppedItem = Cast<UItemInstance>(Payload);

	if (EquipmentComp && DroppedItem)
	{
		return EquipmentComp->EquipItem(DroppedItem);
	}

	return false;
}

FReply UCharacterEquipmentSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
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
		if (SlotText.IsEmpty() && SlotTag.IsValid())
		{
			SlotText = BuildSlotDisplayText(SlotTag);
		}
		Txt_Slot->SetText(SlotText);
	}
}

void UCharacterEquipmentSlotWidget::OnSlotButtonClicked()
{
}