#include "Inventory/Upgrading/UpgradingComponent.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Inventory/InventoryFunctionLibrary.h"
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/ItemInstance.h"
#include "Item/Data/Fragment/ItemFragment_Upgradable.h"
#include "Item/Data/Fragment/ItemFragment_Durability.h" // 내구도 이관용

UUpgradingComponent::UUpgradingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

bool UUpgradingComponent::GetAvailableUpgrades(UInventoryCoreComponent* Inventory, FGuid InventoryGuid, int32 SlotIndex, TArray<FUpgradeRecipe>& OutRecipes)
{
	OutRecipes.Empty();

	if (!Inventory) return false;

	FInventorySlot* Slot = Inventory->GetSlot(InventoryGuid, SlotIndex);
	if (!Slot || Slot->IsEmpty() || !Slot->ItemInstance)
	{
		return false;
	}

	const UItemFragment_Upgradable* UpgradeFragment = Slot->ItemInstance->FindFragmentByClass<UItemFragment_Upgradable>();
	if (UpgradeFragment)
	{
		OutRecipes = UpgradeFragment->UpgradeRecipes;
		return OutRecipes.Num() > 0;
	}

	return false;
}

void UUpgradingComponent::TryUpgradeItem(UInventoryCoreComponent* Inventory, FGuid InventoryGuid, int32 SlotIndex, int32 RecipeIndex)
{
	if (!Inventory || !Inventory->GetOwner()->HasAuthority())
	{
		OnUpgradeFailed.Broadcast(FText::FromString("Invalid Inventory or No Authority"));
		return;
	}

	// 1. 유효성 검사 및 데이터 확보
	TArray<FUpgradeRecipe> Recipes;
	if (!GetAvailableUpgrades(Inventory, InventoryGuid, SlotIndex, Recipes))
	{
		OnUpgradeFailed.Broadcast(FText::FromString("No upgrades available for this item"));
		return;
	}

	if (!Recipes.IsValidIndex(RecipeIndex))
	{
		OnUpgradeFailed.Broadcast(FText::FromString("Invalid recipe index"));
		return;
	}

	const FUpgradeRecipe& Recipe = Recipes[RecipeIndex];
	UItemDefinition* ResultDef = Recipe.ResultItemDef.LoadSynchronous(); // 동기 로드 필요

	if (!ResultDef)
	{
		OnUpgradeFailed.Broadcast(FText::FromString("Invalid result item definition"));
		return;
	}

	// 2. 재료 확인 InventoryCoreComponent의 조회 기능 활용
	if (!HasEnoughMaterials(Inventory, Recipe.Costs))
	{
		OnUpgradeFailed.Broadcast(FText::FromString("Not enough materials"));
		return;
	}

	// 3. 재료 소모
	ConsumeMaterials(Inventory, Recipe.Costs);

	// 4. 업그레이드 실행 (교체 로직)
	FInventorySlot* Slot = Inventory->GetSlot(InventoryGuid, SlotIndex);
	if (!Slot || Slot->IsEmpty()) return;

	// A. 기존 아이템 인스턴스 백업
	// RemoveItem 호출 후에도 이 포인터는 GC가 돌기 전까지(함수 내에서) 유효합니다.
	UItemInstance* OldInstance = Slot->ItemInstance;

	// B. 기존 아이템 제거
	Inventory->RemoveItem(InventoryGuid, SlotIndex, 1);

	// C. 새 아이템 생성 및 슬롯 할당 (리팩토링된 함수 사용)
	// [변경 1] AddItemToSlot(Def) -> AddNewItemToSlot(Def)
	// 빈 슬롯(방금 비운 자리)에 새 정의(ResultDef)로 만든 인스턴스를 넣습니다.
	bool bAdded = Inventory->AddNewItemToSlot(InventoryGuid, SlotIndex, ResultDef, 1);

	if (!bAdded)
	{
		// 원래 슬롯에 넣기 실패했다면(오류 등), 해당 인벤토리의 빈 곳에라도 넣습니다.
		// [변경 2] AddItem(Guid, Def) -> AddItemToInventory(Guid, Def)
		int32 AddedQty = Inventory->AddItemToInventory(InventoryGuid, ResultDef, 1);
		bAdded = (AddedQty > 0);
	}

	if (bAdded)
	{
		// 5. 상태 이관 (내구도, 강화수치 등)
		if (Recipe.bTransferItemState && OldInstance)
		{
			// 방금 추가된 새 아이템의 인스턴스를 찾습니다.
			// (대부분의 경우 SlotIndex 자리에 그대로 있습니다)
			FInventorySlot* NewSlot = Inventory->GetSlot(InventoryGuid, SlotIndex);
          
			// 슬롯 검증: 비어있지 않고, 방금 넣은 아이템 정의와 일치하는지
			if (NewSlot && !NewSlot->IsEmpty() && NewSlot->GetItemDefinition() == ResultDef)
			{
				TransferItemState(OldInstance, NewSlot->ItemInstance);
			}
			// 만약 AddNewItemToSlot은 실패하고 AddItemToInventory로 다른 곳에 들어갔다면?
			// (이 경우 새 위치를 찾기 까다롭지만, 업그레이드는 보통 제자리에 성공하므로 예외 처리 생략 가능)
		}

		OnUpgradeSuccess.Broadcast(ResultDef);
	}
	else
	{
		// 심각한 오류: 재료만 날아감 (실제 게임에선 우편으로 보내거나 재료 복구 필요)
		OnUpgradeFailed.Broadcast(FText::FromString("Inventory error: Failed to add upgraded item"));
	}
}

bool UUpgradingComponent::HasEnoughMaterials(UInventoryCoreComponent* Inventory, const TArray<FUpgradeCost>& Costs)
{
	for (const FUpgradeCost& Cost : Costs)
	{
		if (Cost.ItemDef.IsNull()) continue;
		
		UItemDefinition* MatDef = Cost.ItemDef.LoadSynchronous();
		// CountItemByDef 활용
		// 전체 인벤토리에서 검색하도록 구현 (필요시 특정 인벤토리만 검색하도록 수정)
		int32 OwnedCount = 0;
		for(const FGuid& Guid : Inventory->GetAllInventoryGuids())
		{
			OwnedCount += Inventory->CountItemByDef(Guid, MatDef);
		}

		if (OwnedCount < Cost.Quantity)
		{
			return false;
		}
	}
	return true;
}

void UUpgradingComponent::ConsumeMaterials(UInventoryCoreComponent* Inventory, const TArray<FUpgradeCost>& Costs)
{
	for (const FUpgradeCost& Cost : Costs)
	{
		if (Cost.ItemDef.IsNull()) continue;
		
		UItemDefinition* MatDef = Cost.ItemDef.LoadSynchronous();
		int32 RemainingToRemove = Cost.Quantity;

		// 모든 인벤토리를 순회하며 차감 RemoveItemByDef 활용
		TArray<FGuid> Guids = Inventory->GetAllInventoryGuids();
		for(const FGuid& Guid : Guids)
		{
			int32 CountInBag = Inventory->CountItemByDef(Guid, MatDef);
			if(CountInBag > 0)
			{
				int32 RemoveAmount = FMath::Min(RemainingToRemove, CountInBag);
				Inventory->RemoveItemByDef(Guid, MatDef, RemoveAmount);
				RemainingToRemove -= RemoveAmount;
			}

			if(RemainingToRemove <= 0) break;
		}
	}
}

void UUpgradingComponent::TransferItemState(UItemInstance* SourceInstance, UItemInstance* DestInstance)
{
	if (!SourceInstance || !DestInstance) return;

	// 예: 내구도 비율 유지
	const UItemFragment_Durability* OldDurability = SourceInstance->FindFragmentByClass<UItemFragment_Durability>();
	UItemFragment_Durability* NewDurability = DestInstance->FindMutableFragmentByClass<UItemFragment_Durability>();

	if (OldDurability && NewDurability)
	{
		float OldPercent = OldDurability->GetDurabilityPercent();
		NewDurability->CurrentDurability = NewDurability->MaxDurability * OldPercent;
	}

	// 여기에 인챈트나 강화 수치 등 추가 이관 로직 작성 가능
	// 예: UItemFragment_Enchantable 데이터 복사
}