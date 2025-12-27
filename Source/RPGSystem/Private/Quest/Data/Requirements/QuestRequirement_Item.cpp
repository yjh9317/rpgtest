#include "Quest/Data/Requirements/QuestRequirement_Item.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Player/RPGPlayerController.h"
#include "Item/Data/ItemDefinition.h"

bool UQuestRequirement_Item::IsMet(const UQuestManagerComponent* Manager, const ARPGPlayerController* Player) const
{
	if (!IsValid(Player) || !IsValid(RequiredItem)) return false;

	APawn* Pawn = Player->GetPawn();
	if (!IsValid(Pawn)) return false;

	UInventoryCoreComponent* Inventory = Pawn->FindComponentByClass<UInventoryCoreComponent>();
	if (Inventory)
	{
		// 여기서는 모든 인벤토리를 다 합쳐서 체크하는 방식의 예시입니다.
		// 만약 특정 인벤토리만 체크해야 한다면 InventoryCoreComponent에 함수 추가 필요
		TArray<FGuid> AllInventories = Inventory->GetAllInventoryGuids();
		int32 TotalCount = 0;
		for(const FGuid& Guid : AllInventories)
		{
			TotalCount += Inventory->CountItemByDef(Guid, RequiredItem);
		}
		
		return TotalCount >= Quantity;
	}

	return false;
}

FText UQuestRequirement_Item::GetFailReason() const
{
	FString ItemName = IsValid(RequiredItem) ? RequiredItem->GetName() : TEXT("Unknown Item");
	return FText::Format(NSLOCTEXT("Quest", "FailItem", "{0} 아이템이 {1}개 필요합니다."), FText::FromString(ItemName), Quantity);
}
