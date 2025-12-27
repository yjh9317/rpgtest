// EquipmentComponent.cpp
#include "Equipment/EquipmentComponent.h"

#include "Combat/Action/BaseAction.h"
#include "Combat/Action/Components/ActionComponent.h"
#include "Item/Data/ItemInstance.h"
#include "Item/Data/Fragment/ItemFragment_Equippable.h"
#include "Net/UnrealNetwork.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Item/Data/Fragment/ItemFragment_Actions.h"

// =========================================================
// FEquipmentList 구현 (Fast Array)
// =========================================================

void FEquipmentList::AddOrUpdateEntry(FGameplayTag SlotTag, UItemInstance* Item)
{
	// 이미 존재하는 슬롯이면 업데이트
	for (FEquipmentEntry& Entry : Entries)
	{
		if (Entry.SlotTag == SlotTag)
		{
			Entry.ItemInstance = Item;
			MarkItemDirty(Entry); // 변경 알림
			return;
		}
	}

	// 없으면 새로 추가
	FEquipmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.SlotTag = SlotTag;
	NewEntry.ItemInstance = Item;
	MarkItemDirty(NewEntry);
	MarkArrayDirty();
}

bool FEquipmentList::RemoveEntry(FGameplayTag SlotTag)
{
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].SlotTag == SlotTag)
		{
			Entries.RemoveAt(i);
			MarkArrayDirty();
			return true;
		}
	}
	return false;
}

UItemInstance* FEquipmentList::FindItem(FGameplayTag SlotTag) const
{
	for (const FEquipmentEntry& Entry : Entries)
	{
		if (Entry.SlotTag == SlotTag)
		{
			return Entry.ItemInstance;
		}
	}
	return nullptr;
}

bool FEquipmentList::Contains(FGameplayTag SlotTag) const
{
	for (const FEquipmentEntry& Entry : Entries)
	{
		if (Entry.SlotTag == SlotTag) return true;
	}
	return false;
}

// === 복제 콜백 (클라이언트에서 호출됨) ===

void FEquipmentList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	if (!OwnerComponent) return;
	for (int32 Idx : AddedIndices)
	{
		const FEquipmentEntry& Entry = Entries[Idx];
		// 클라이언트 비주얼/UI 업데이트 트리거
		OwnerComponent->BroadcastEquipmentUpdate(Entry.SlotTag, Entry.ItemInstance);
	}
}

void FEquipmentList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	if (!OwnerComponent) return;
	for (int32 Idx : ChangedIndices)
	{
		const FEquipmentEntry& Entry = Entries[Idx];
		OwnerComponent->BroadcastEquipmentUpdate(Entry.SlotTag, Entry.ItemInstance);
	}
}

void FEquipmentList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	if (!OwnerComponent) return;
	for (int32 Idx : RemovedIndices)
	{
		const FEquipmentEntry& Entry = Entries[Idx];
		// 제거됨 알림 (ItemInstance = nullptr로 처리)
		OwnerComponent->BroadcastEquipmentUpdate(Entry.SlotTag, nullptr);
	}
}

// =========================================================
// UEquipmentComponent 구현
// =========================================================

UEquipmentComponent::UEquipmentComponent()
{
	SetIsReplicatedByDefault(true);
}

void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	EquippedItems.OwnerComponent = this;
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UEquipmentComponent, EquippedItems);
}

UItemInstance* UEquipmentComponent::GetEquippedItem(FGameplayTag SlotTag) const
{
	return EquippedItems.FindItem(SlotTag);
}

void UEquipmentComponent::InitializeEquipment(UInventoryCoreComponent* InInventoryComp)
{
	InventoryCore = InInventoryComp;
	if (InventoryCore)
	{
		EquipmentInventoryGUID = InventoryCore->FindInventoryGuid(EquipmentInventoryName);
	}
}

bool UEquipmentComponent::EquipItem(UItemInstance* ItemToEquip)
{
	if (!ItemToEquip || !InventoryCore) return false;

    const UItemFragment_Equippable* EquipFrag = ItemToEquip->FindFragmentByClass<UItemFragment_Equippable>();
    if (!EquipFrag) return false;

    FGameplayTag SlotTag = EquipFrag->EquipmentSlotTag;

    // 2. [교체 로직] 이미 장착된 아이템이 있다면 해제
    if (EquippedItems.Contains(SlotTag))
    {
        UItemInstance* OldItem = EquippedItems.FindItem(SlotTag);
        
        // 2-1. 기존 스탯/비주얼 해제
        UnapplyEquipmentStats(OldItem);
        UnapplyEquipmentVisuals(SlotTag);
        
        // 2-2. 기존 액션 해제 (ActionComponent)
        if (AActor* Owner = GetOwner())
        {
            if (UActionComponent* ActionComp = Owner->FindComponentByClass<UActionComponent>())
            {
                if (const UItemFragment_Actions* OldActionFrag = OldItem->FindFragmentByClass<UItemFragment_Actions>())
                {
                    // [수정] TArray<FGrantedActionDef> 순회
                    for (const FGrantedActionDef& ActionDef : OldActionFrag->GrantedActions)
                    {
                        if (ActionDef.SlotTag.IsValid())
                        {
                            ActionComp->UnregisterAction(ActionDef.SlotTag);
                        }
                    }
                }
            }
        }

        EquippedItems.RemoveEntry(SlotTag);
        InventoryCore->LootItemInstance(OldItem, 1);
    }

    // 3. 인벤토리에서 제거
    bool bRemoved = InventoryCore->RemoveItemFromSpecificInventory(EquipmentInventoryGUID, ItemToEquip);
    if (!bRemoved)
    {
        UE_LOG(LogTemp, Warning, TEXT("EquipItem: Item not found in Equipment Inventory."));
        return false; 
    }

    // 4. [등록 로직] 새 아이템의 액션 등록
    if (AActor* Owner = GetOwner())
    {
        if (UActionComponent* ActionComp = Owner->FindComponentByClass<UActionComponent>())
        {
            if (const UItemFragment_Actions* ActionFrag = ItemToEquip->FindFragmentByClass<UItemFragment_Actions>())
            {
                // [수정] TArray<FGrantedActionDef> 순회
                for (const FGrantedActionDef& ActionDef : ActionFrag->GrantedActions)
                {
                    if (ActionDef.SlotTag.IsValid() && ActionDef.ActionClass)
                    {
                        // 구조체의 필드(SlotTag, ActionClass)를 사용
                        ActionComp->RegisterAction(ActionDef.SlotTag, ActionDef.ActionClass);
                    }
                }
            }
        }
    }

    // 5. 장착 처리 마무리
    ItemToEquip->Rename(nullptr, this);
    EquippedItems.AddOrUpdateEntry(SlotTag, ItemToEquip);

    ApplyEquipmentVisuals(SlotTag, ItemToEquip);
    ApplyEquipmentStats(ItemToEquip);

    BroadcastEquipmentUpdate(SlotTag, ItemToEquip);
    
    return true;
}

bool UEquipmentComponent::UnequipItem(FGameplayTag SlotTag)
{
	if (!EquippedItems.Contains(SlotTag)) return false;
	if (!InventoryCore || !EquipmentInventoryGUID.IsValid()) return false;

	UItemInstance* ItemToUnequip = EquippedItems.FindItem(SlotTag);
	if (!ItemToUnequip) return false;

	// 2. 인벤토리 공간 확인
	int32 EmptySlot = InventoryCore->GetEmptySlotIndex(EquipmentInventoryGUID);
    
	if (EmptySlot >= 0)
	{
		// 3. 액션 해제 (인벤토리 이동 확정 후 실행)
		if (AActor* Owner = GetOwner())
		{
			if (UActionComponent* ActionComp = Owner->FindComponentByClass<UActionComponent>())
			{
				if (const UItemFragment_Actions* ActionFrag = ItemToUnequip->FindFragmentByClass<UItemFragment_Actions>())
				{
					// [수정] TArray<FGrantedActionDef> 순회
					for (const FGrantedActionDef& ActionDef : ActionFrag->GrantedActions)
					{
						if (ActionDef.SlotTag.IsValid())
						{
							ActionComp->UnregisterAction(ActionDef.SlotTag);
						}
					}
				}
			}
		}

		// 4. 인벤토리로 이동
		InventoryCore->AddItemToSlot(EquipmentInventoryGUID, EmptySlot, ItemToUnequip, 1);
    
		// 5. 장착 해제 마무리
		UnapplyEquipmentStats(ItemToUnequip);
		UnapplyEquipmentVisuals(SlotTag);
		EquippedItems.RemoveEntry(SlotTag);
       
		BroadcastEquipmentUpdate(SlotTag, nullptr);
        
		return true;
	}
    
	return false;
}

void UEquipmentComponent::BroadcastEquipmentUpdate(FGameplayTag SlotTag, UItemInstance* ItemInstance)
{
	// 1. UI 델리게이트 전파
	OnEquipmentUpdated.Broadcast(SlotTag, ItemInstance);

	// 2. (클라이언트) 만약 복제되어서 들어온 경우라면 비주얼 업데이트 수행
	// 서버는 EquipItem에서 직접 호출하지만, 클라이언트는 여기서 처리해야 함
	if (GetOwner()->GetLocalRole() != ROLE_Authority)
	{
		if (ItemInstance)
		{
			ApplyEquipmentVisuals(SlotTag, ItemInstance);
			// ApplyEquipmentStats(ItemInstance); // 스탯은 보통 서버 권한이라 클라 적용 X (UI 표시는 별도)
		}
		else
		{
			UnapplyEquipmentVisuals(SlotTag);
		}
	}
}

void UEquipmentComponent::ApplyEquipmentVisuals(FGameplayTag SlotTag, UItemInstance* Item)
{
	const UItemDefinition* Def = Item->GetItemDef();
	if (!Def) return;

	const UItemFragment_Equippable* EquipFrag = Def->FindFragmentByClass<UItemFragment_Equippable>();
	if (!EquipFrag) return;

	// [이전 답변의 RefreshVisuals 로직 활용]
	// StaticMesh(무기) -> Actor Spawn & Attach
	// SkeletalMesh(방어구) -> SetMasterPoseComponent or SetSkeletalMesh
    
	// 생성된 액터/메시를 SpawnedEquipmentActors 맵에 저장하여 추적
}

void UEquipmentComponent::UnapplyEquipmentVisuals(FGameplayTag SlotTag)
{
	// 저장해둔 액터/메시 제거
	if (SpawnedEquipmentActors.Contains(SlotTag))
	{
		if (AActor* Actor = SpawnedEquipmentActors[SlotTag])
		{
			Actor->Destroy();
		}
		SpawnedEquipmentActors.Remove(SlotTag);
	}
    
	// 갑옷의 경우 기본 메시로 복구하는 로직이 필요할 수 있음
}

void UEquipmentComponent::ApplyEquipmentStats(UItemInstance* Item)
{
	// if (!CachedStats) return;
	// ItemFragment_Stats 데이터를 읽어서 StatsComponent에 Modifier 추가
	// ModifierTag로 ItemInstance의 GUID나 SlotTag를 사용하여 나중에 제거할 수 있게 함
}

void UEquipmentComponent::UnapplyEquipmentStats(UItemInstance* Item)
{
	// if (!CachedStats) return;
	// 해당 아이템(혹은 슬롯)에 해당하는 Modifier 제거
}
