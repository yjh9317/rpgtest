// ItemDataStructure.cpp

#include "Item/Data/ItemDataStructure.h"
#include "Inventory/InventoryCoreComponent.h"

// ========================================
// FInventoryList 구현
// ========================================

void FInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
    // 클라이언트에서 인벤토리가 추가되었을 때
    if (!OwnerComponent)
    {
        return;
    }

    for (int32 Index : AddedIndices)
    {
        if (Entries.IsValidIndex(Index))
        {
            const FInventoryEntry& Entry = Entries[Index];
            
            UE_LOG(LogTemp, Log, TEXT("[Client] Inventory added: %s"), 
                *Entry.MetaData.InventoryName.ToString());
            
            // 전체 인벤토리 갱신 델리게이트 (-1은 전체 갱신)
            OwnerComponent->OnInventoryChanged.Broadcast(Entry.InventoryGuid, -1);
        }
    }
}

void FInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
    // 클라이언트에서 인벤토리가 변경되었을 때
    if (!OwnerComponent)
    {
        return;
    }

    for (int32 Index : ChangedIndices)
    {
        if (Entries.IsValidIndex(Index))
        {
            const FInventoryEntry& Entry = Entries[Index];
            
            // 전체 인벤토리 갱신
            OwnerComponent->OnInventoryChanged.Broadcast(Entry.InventoryGuid, -1);
        }
    }
}

void FInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
    // 클라이언트에서 인벤토리가 제거되기 전
    if (!OwnerComponent)
    {
        return;
    }

    for (int32 Index : RemovedIndices)
    {
        if (Entries.IsValidIndex(Index))
        {
            const FInventoryEntry& Entry = Entries[Index];
            
            UE_LOG(LogTemp, Log, TEXT("[Client] Inventory removed: %s"), 
                *Entry.MetaData.InventoryName.ToString());
            
            OwnerComponent->OnInventoryCleared.Broadcast(
                Entry.InventoryGuid, 
                Entry.MetaData.InventoryName);
        }
    }
}

FInventoryEntry* FInventoryList::FindInventoryByGuid(const FGuid& InGuid)
{
    for (FInventoryEntry& Entry : Entries)
    {
        if (Entry.InventoryGuid == InGuid)
        {
            return &Entry;
        }
    }
    return nullptr;
}

const FInventoryEntry* FInventoryList::FindInventoryByGuid(const FGuid& InGuid) const
{
    for (const FInventoryEntry& Entry : Entries)
    {
        if (Entry.InventoryGuid == InGuid)
        {
            return &Entry;
        }
    }
    return nullptr;
}

FInventoryEntry* FInventoryList::FindInventoryByName(FName InName)
{
    for (FInventoryEntry& Entry : Entries)
    {
        if (Entry.MetaData.InventoryName == InName)
        {
            return &Entry;
        }
    }
    return nullptr;
}

const FInventoryEntry* FInventoryList::FindInventoryByName(FName InName) const
{
    for (const FInventoryEntry& Entry : Entries)
    {
        if (Entry.MetaData.InventoryName == InName)
        {
            return &Entry;
        }
    }
    return nullptr;
}

FInventoryEntry& FInventoryList::AddInventory(const FGuid& InGuid, const FInventoryMetaData& InMetaData, const TArray<FInventorySlot>& InSlots)
{
    FInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
    NewEntry.InventoryGuid = InGuid;
    NewEntry.MetaData = InMetaData;
    NewEntry.Slots = InSlots;
    
    // Fast Array에 변경 마킹
    MarkItemDirty(NewEntry);
    
    return NewEntry;
}

bool FInventoryList::RemoveInventory(const FGuid& InGuid)
{
    for (int32 i = 0; i < Entries.Num(); ++i)
    {
        if (Entries[i].InventoryGuid == InGuid)
        {
            Entries.RemoveAt(i);
            MarkArrayDirty();
            return true;
        }
    }
    return false;
}

TArray<FName> FInventoryList::GetAllInventoryNames() const
{
    TArray<FName> Names;
    Names.Reserve(Entries.Num());
        
    for (const FInventoryEntry& Entry : Entries)
    {
        Names.Add(Entry.MetaData.InventoryName);
    }
        
    return Names;
}

TArray<FGuid> FInventoryList::GetAllInventoryGuids() const
{
    TArray<FGuid> Guids;
    Guids.Reserve(Entries.Num());
        
    for (const FInventoryEntry& Entry : Entries)
    {
        Guids.Add(Entry.InventoryGuid);
    }
        
    return Guids;
}

int32 FInventoryList::GetInventoryCount() const
{
    return Entries.Num();
}

bool FInventoryList::IsEmpty() const
{
    return Entries.Num() == 0;
}
