// Source/RPGSystem/Private/Inventory/InventoryCoreComponent.cpp

#include "Inventory/InventoryCoreComponent.h"
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/ItemInstance.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DataTable.h"
#include "Inventory/InventoryInitializer.h"

UInventoryCoreComponent::UInventoryCoreComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    InventoryInitializer = CreateDefaultSubobject<UInventoryInitializer>(TEXT("InventoryInitializer"));
}

void UInventoryCoreComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeInventory();
}

void UInventoryCoreComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInventoryCoreComponent, InventoryList);
}

// ========================================
// 초기화 / 생성 / 삭제
// ========================================

void UInventoryCoreComponent::InitializeInventory()
{
    if (GetOwnerRole() == ROLE_Authority && InventoryInitializer)
    {
        InventoryInitializer->Initialize(this);
    }
}

FGuid UInventoryCoreComponent::CreateInventory(const FInventoryCreateConfig& Config)
{
    FGuid NewGuid = FGuid::NewGuid();
    TArray<FInventorySlot> NewSlots;
    NewSlots.SetNum(Config.SlotCount);
    
    for (int32 i = 0; i < NewSlots.Num(); ++i)
    {
        NewSlots[i].SlotIndex = i;
    }
    
    FInventoryMetaData NewMetaData;
    NewMetaData.InventoryName = Config.InventoryName;
    NewMetaData.DisplayName = Config.DisplayName;
    NewMetaData.MaxSlots = Config.MaxSlots;
    NewMetaData.bUseWeight = Config.bUseWeight;
    NewMetaData.MaxWeight = Config.MaxWeight;
    NewMetaData.CurrentWeight = 0.0f;
    NewMetaData.AllowedTypes = Config.AllowedTypes;
    
    InventoryList.AddInventory(NewGuid, NewMetaData, NewSlots);
    
    UE_LOG(LogTemp, Log, TEXT("Created inventory '%s' with GUID: %s"),*Config.InventoryName.ToString(), *NewGuid.ToString());
    
    return NewGuid;
}

bool UInventoryCoreComponent::DestroyInventory(FGuid InventoryGuid)
{
    if (!InventoryGuid.IsValid() || !HasInventory(InventoryGuid)) return false;
    
    FName InventoryName = NAME_None;
    if (const FInventoryEntry* Entry = InventoryList.FindInventoryByGuid(InventoryGuid))
    {
        InventoryName = Entry->MetaData.InventoryName;
    }
    
    if (InventoryList.RemoveInventory(InventoryGuid))
    {
        OnInventoryCleared.Broadcast(InventoryGuid, InventoryName);
        return true;
    }
    return false;
}

bool UInventoryCoreComponent::DestroyInventoryByName(FName InventoryName)
{
    const FGuid InventoryGuid = FindInventoryGuid(InventoryName);
    return DestroyInventory(InventoryGuid);
}

// ========================================
// 조회 (Query)
// ========================================

TArray<FInventorySlot>* UInventoryCoreComponent::GetInventory(FGuid InventoryGuid)
{
    FInventoryEntry* Entry = InventoryList.FindInventoryByGuid(InventoryGuid);
    return Entry ? &Entry->Slots : nullptr;
}

const TArray<FInventorySlot>* UInventoryCoreComponent::GetInventory(FGuid InventoryGuid) const
{
    const FInventoryEntry* Entry = InventoryList.FindInventoryByGuid(InventoryGuid);
    return Entry ? &Entry->Slots : nullptr;
}

FInventoryMetaData* UInventoryCoreComponent::GetMetaData(FGuid InventoryGuid)
{
    FInventoryEntry* Entry = InventoryList.FindInventoryByGuid(InventoryGuid);
    return Entry ? &Entry->MetaData : nullptr;
}

const FInventoryMetaData* UInventoryCoreComponent::GetMetaData(FGuid InventoryGuid) const
{
    const FInventoryEntry* Entry = InventoryList.FindInventoryByGuid(InventoryGuid);
    return Entry ? &Entry->MetaData : nullptr;
}

FInventorySlot* UInventoryCoreComponent::GetSlot(FGuid InventoryGuid, int32 SlotIndex)
{
    TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    return (Inventory && Inventory->IsValidIndex(SlotIndex)) ? &(*Inventory)[SlotIndex] : nullptr;
}

const FInventorySlot* UInventoryCoreComponent::GetSlot(FGuid InventoryGuid, int32 SlotIndex) const
{
    const TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    return (Inventory && Inventory->IsValidIndex(SlotIndex)) ? &(*Inventory)[SlotIndex] : nullptr;
}

bool UInventoryCoreComponent::HasInventory(FGuid InventoryGuid) const
{
    return InventoryGuid.IsValid() && InventoryList.FindInventoryByGuid(InventoryGuid) != nullptr;
}

bool UInventoryCoreComponent::IsInventoryFull(FGuid InventoryGuid) const
{
    const TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    if (!Inventory) return true;
    
    for (const FInventorySlot& Slot : *Inventory)
    {
        if (Slot.IsEmpty()) return false;
    }
    return true;
}

int32 UInventoryCoreComponent::GetEmptySlotIndex(FGuid InventoryGuid) const
{
    const TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    if (!Inventory) return -1;
    
    for (int32 i = 0; i < Inventory->Num(); ++i)
    {
        if ((*Inventory)[i].IsEmpty()) return i;
    }
    return -1;
}

FGuid UInventoryCoreComponent::FindInventoryGuid(FName InventoryName) const
{
    const FInventoryEntry* Entry = InventoryList.FindInventoryByName(InventoryName);
    return Entry ? Entry->InventoryGuid : FGuid();
}

bool UInventoryCoreComponent::GetInventoryByName(FName InventoryName, TArray<FInventorySlot>& OutSlots) const
{
    const FGuid Guid = GetGuidByName_NoLog(InventoryName);
    if (!Guid.IsValid()) return false;

    const TArray<FInventorySlot>* Slots = GetInventory(Guid);
    if (!Slots) return false;

    OutSlots = *Slots;
    return true;
}

TArray<FName> UInventoryCoreComponent::GetAllInventoryNames() const
{
    return InventoryList.GetAllInventoryNames();
}

TArray<FGuid> UInventoryCoreComponent::GetAllInventoryGuids() const
{
    return InventoryList.GetAllInventoryGuids();
}

// =============================================================
// [메인 API] 스마트 루팅 (Looting)
// =============================================================

int32 UInventoryCoreComponent::LootNewItem(const UItemDefinition* ItemDef, int32 Quantity)
{
    if (!ItemDef || Quantity <= 0) return Quantity;

    int32 Remaining = Quantity;

    // 1. Main 인벤토리 우선 시도
    FGuid MainGuid = FindInventoryGuid(FName("Main"));
    if (MainGuid.IsValid())
    {
        int32 Added = InsertItem_Internal(MainGuid, ItemDef, Remaining, nullptr);
        Remaining -= Added;
    }

    // 2. 남았다면 다른 인벤토리 순회 (타입 필터에 맞는 곳 찾기)
    if (Remaining > 0)
    {
        for (const FInventoryEntry& Entry : InventoryList.Entries)
        {
            if (Entry.InventoryGuid == MainGuid) continue; // Main은 이미 했음

            int32 Added = InsertItem_Internal(Entry.InventoryGuid, ItemDef, Remaining, nullptr);
            Remaining -= Added;

            if (Remaining <= 0) break;
        }
    }

    return Remaining; // 0이면 성공, 남았으면 인벤토리 꽉 참
}

bool UInventoryCoreComponent::LootItemInstance(UItemInstance* InInstance, int32 Quantity)
{
    if (!InInstance || Quantity <= 0) return false;
    
    const UItemDefinition* Def = InInstance->GetItemDef();
    if (!Def) return false;

    // 1. "Main" 인벤토리 우선 시도
    FGuid MainGuid = FindInventoryGuid(FName("Main"));
    if (MainGuid.IsValid())
    {
        // InsertItem_Internal은 실제로 넣은 개수를 반환함
        int32 Added = InsertItem_Internal(MainGuid, Def, Quantity, InInstance);
        
        // 다 넣었으면 성공
        if (Added == Quantity) return true;
        
        // 일부만 들어갔다면(가방 꽉 참 등), 남은 수량 계산 필요
        // (복잡해지므로 여기서는 "전부 성공" 아니면 "실패"로 단순화하거나, 
        //  남은 수량을 처리하는 루프를 돌려야 함. 스마트 루팅은 보통 루프를 돕니다.)
    }

    // 2. 다른 인벤토리 순회 (필터 맞는 곳 찾기)
    for (const FInventoryEntry& Entry : InventoryList.Entries)
    {
        if (Entry.InventoryGuid == MainGuid) continue;

        int32 Added = InsertItem_Internal(Entry.InventoryGuid, Def, Quantity, InInstance);
        if (Added == Quantity) return true;
    }

    return false; // 넣을 곳이 없음 (인벤토리 가득 참)
}

// =============================================================
// [관리 API]
// =============================================================

int32 UInventoryCoreComponent::AddItemToInventory(FGuid InventoryGuid, const UItemDefinition* ItemDef, int32 Quantity)
{
    return InsertItem_Internal(InventoryGuid, ItemDef, Quantity, nullptr);
}

bool UInventoryCoreComponent::AddItemToSlot(FGuid InventoryGuid, int32 SlotIndex, UItemInstance* InInstance, int32 Quantity)
{
    if (!IsValidSlotIndex(InventoryGuid, SlotIndex) || !InInstance || Quantity <= 0) return false;
    
    FInventorySlot* Slot = GetSlot(InventoryGuid, SlotIndex);
    if (!Slot || !Slot->IsEmpty()) return false; 

    // 무게 체크
    FInventoryMetaData* Meta = GetMetaData(InventoryGuid);
    if (Meta && Meta->bUseWeight && WeightPolicy == EInventoryWeightPolicy::BlockOverflow)
    {
        float AddWeight = GetItemWeight(InInstance->GetItemDef(), Quantity);
        if (Meta->CurrentWeight + AddWeight > Meta->MaxWeight) return false;
    }

    // 소유권 이전
    InInstance->Rename(nullptr, this);
    Slot->ItemInstance = InInstance;
    Slot->Quantity = Quantity;
    
    HandleInventoryChanged(InventoryGuid, SlotIndex, EInventoryRefreshType::SingleSlot, InInstance->GetItemDef(), true);
    return true;
}

bool UInventoryCoreComponent::AddNewItemToSlot(FGuid InventoryGuid, int32 SlotIndex, const UItemDefinition* ItemDef, int32 Quantity)
{
    if (!ItemDef || Quantity <= 0) return false;
    
    UItemInstance* NewInst = CreateItemInstance(ItemDef);
    return AddItemToSlot(InventoryGuid, SlotIndex, NewInst, Quantity);
}

bool UInventoryCoreComponent::RemoveItem(FGuid InventoryGuid, int32 SlotIndex, int32 Quantity)
{
    if (!IsValidSlotIndex(InventoryGuid, SlotIndex) || Quantity <= 0) return false;
    
    FInventorySlot* Slot = GetSlot(InventoryGuid, SlotIndex);
    if (!Slot || Slot->IsEmpty()) return false;
    
    const UItemDefinition* ItemDef = Slot->GetItemDefinition();
    
    if (Slot->Quantity <= Quantity)
    {
        Slot->Clear(); // 전체 제거
        HandleInventoryChanged(InventoryGuid, SlotIndex, EInventoryRefreshType::SingleSlot, ItemDef, false);
    }
    else
    {
        Slot->Quantity -= Quantity; // 부분 제거
        HandleInventoryChanged(InventoryGuid, SlotIndex, EInventoryRefreshType::SingleSlot, nullptr, false);
    }
    
    UpdateWeight(InventoryGuid);
    return true;
}

bool UInventoryCoreComponent::RemoveItemByDef(FGuid InventoryGuid, const UItemDefinition* ItemDef, int32 Quantity)
{
    if (!ItemDef || Quantity <= 0 || !HasInventory(InventoryGuid)) return false;
    
    TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    if (!Inventory) return false;
    
    int32 RemainingToRemove = Quantity;
    
    for (int32 i = 0; i < Inventory->Num() && RemainingToRemove > 0; ++i)
    {
        FInventorySlot& Slot = (*Inventory)[i];
        
        if (Slot.GetItemDefinition() == ItemDef)
        {
            const int32 AmountToRemove = FMath::Min(Slot.Quantity, RemainingToRemove);
            
            if (Slot.Quantity <= AmountToRemove)
            {
                RemainingToRemove -= Slot.Quantity;
                Slot.Clear();
                OnItemRemoved.Broadcast(InventoryGuid, i, ItemDef);
            }
            else
            {
                Slot.Quantity -= AmountToRemove;
                RemainingToRemove -= AmountToRemove;
            }
            
            OnInventoryChanged.Broadcast(InventoryGuid, i);
        }
    }
    
    UpdateWeight(InventoryGuid);
    return RemainingToRemove == 0;
}

bool UInventoryCoreComponent::RemoveItemFromSpecificInventory(FGuid InventoryGuid, UItemInstance* ItemInstance)
{
    if (!ItemInstance || !HasInventory(InventoryGuid))
    {
        return false;
    }

    const TArray<FInventorySlot>* Slots = GetInventory(InventoryGuid);
    if (!Slots) return false;

    // 2. 슬롯 순회하며 인스턴스 찾기
    for (int32 i = 0; i < Slots->Num(); ++i)
    {
        if ((*Slots)[i].ItemInstance == ItemInstance)
        {
            int32 QuantityToRemove = (*Slots)[i].Quantity;
            return RemoveItem(InventoryGuid, i, QuantityToRemove);
        }
    }

    return false;
}

// ========================================
// 이동 / 교환 / 슬롯 관리
// ========================================

bool UInventoryCoreComponent::MoveItem(FGuid FromInventoryGuid, int32 FromSlot, FGuid ToInventoryGuid, int32 ToSlot)
{
    if (!IsValidSlotIndex(FromInventoryGuid, FromSlot) || !IsValidSlotIndex(ToInventoryGuid, ToSlot)) return false;

    FInventorySlot* SourceSlot = GetSlot(FromInventoryGuid, FromSlot);
    FInventorySlot* TargetSlot = GetSlot(ToInventoryGuid, ToSlot);
    if (!SourceSlot || !TargetSlot || SourceSlot->IsEmpty()) return false;

    const UItemDefinition* ItemDef = SourceSlot->GetItemDefinition();
    if (!ItemDef) return false;

    if (!PassesFilter(ToInventoryGuid, ItemDef)) return false;

    // 무게 체크 (다른 인벤토리 이동 시)
    if (FromInventoryGuid != ToInventoryGuid)
    {
        FInventoryMetaData* ToMeta = GetMetaData(ToInventoryGuid);
        if (ToMeta && ToMeta->bUseWeight && WeightPolicy == EInventoryWeightPolicy::BlockOverflow)
        {
            const float CurrentWeight = ToMeta->CurrentWeight;
            const float AddWeight     = GetItemWeight(ItemDef, SourceSlot->Quantity);
            if (CurrentWeight + AddWeight > ToMeta->MaxWeight) return false;
        }
    }

    if (!TargetSlot->IsEmpty()) return false;

    *TargetSlot = *SourceSlot;
    SourceSlot->Clear();

    HandleInventoryChanged(FromInventoryGuid, FromSlot, EInventoryRefreshType::SingleSlot, nullptr, false);
    HandleInventoryChanged(ToInventoryGuid, ToSlot, EInventoryRefreshType::SingleSlot, nullptr, true);

    return true;
}

bool UInventoryCoreComponent::SwapItems(FGuid InventoryAGuid, int32 SlotA, FGuid InventoryBGuid, int32 SlotB)
{
    if (!IsValidSlotIndex(InventoryAGuid, SlotA) || !IsValidSlotIndex(InventoryBGuid, SlotB)) return false;

    FInventorySlot* SlotPtrA = GetSlot(InventoryAGuid, SlotA);
    FInventorySlot* SlotPtrB = GetSlot(InventoryBGuid, SlotB);
    if (!SlotPtrA || !SlotPtrB) return false;

    if (!SlotPtrA->IsEmpty() && !PassesFilter(InventoryBGuid, SlotPtrA->GetItemDefinition())) return false;
    if (!SlotPtrB->IsEmpty() && !PassesFilter(InventoryAGuid, SlotPtrB->GetItemDefinition())) return false;

    // 무게 체크 생략 (복잡도 감소, 필요시 추가)

    FInventorySlot Temp = *SlotPtrA;
    *SlotPtrA = *SlotPtrB;
    *SlotPtrB = Temp;

    HandleInventoryChanged(InventoryAGuid, SlotA, EInventoryRefreshType::SingleSlot, nullptr, true);
    HandleInventoryChanged(InventoryBGuid, SlotB, EInventoryRefreshType::SingleSlot, nullptr, true);

    return true;
}

bool UInventoryCoreComponent::AddSlots(FGuid InventoryGuid, int32 Count)
{
    if (Count <= 0 || !HasInventory(InventoryGuid)) return false;
    
    TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    FInventoryMetaData* MetaData = GetMetaData(InventoryGuid);
    if (!Inventory || !MetaData) return false;
    
    if (Inventory->Num() + Count > MetaData->MaxSlots)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot exceed max slots"));
        return false;
    }
    
    const int32 CurrentSize = Inventory->Num();
    Inventory->SetNum(CurrentSize + Count);
    
    for (int32 i = CurrentSize; i < Inventory->Num(); ++i)
    {
        (*Inventory)[i].SlotIndex = i;
    }
    
    return true;
}

void UInventoryCoreComponent::CompactInventory(FGuid InventoryGuid)
{
    TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    if (!Inventory) return;
    
    TArray<FInventorySlot> CompactedSlots;
    for (const FInventorySlot& Slot : *Inventory)
    {
        if (!Slot.IsEmpty()) CompactedSlots.Add(Slot);
    }
    
    const int32 OriginalSize = Inventory->Num();
    CompactedSlots.SetNum(OriginalSize);
    
    for (int32 i = 0; i < CompactedSlots.Num(); ++i)
    {
        CompactedSlots[i].SlotIndex = i;
    }
    
    *Inventory = CompactedSlots;
    HandleInventoryChanged(InventoryGuid, -1, EInventoryRefreshType::FullRefresh, nullptr, false);
}

void UInventoryCoreComponent::ClearInventory(FGuid InventoryGuid)
{
    TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    if (!Inventory) return;
    
    for (FInventorySlot& Slot : *Inventory)
    {
        Slot.Clear();
    }
    HandleInventoryChanged(InventoryGuid, -1, EInventoryRefreshType::FullRefresh, nullptr, false);
}

// ========================================
// 유틸리티
// ========================================

int32 UInventoryCoreComponent::CountItemByDef(FGuid InventoryGuid, const UItemDefinition* ItemDef) const
{
    if (!ItemDef || !HasInventory(InventoryGuid)) return 0;
    
    const TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    int32 TotalCount = 0;
    for (const FInventorySlot& Slot : *Inventory)
    {
        if (Slot.GetItemDefinition() == ItemDef) TotalCount += Slot.Quantity;
    }
    return TotalCount;
}

bool UInventoryCoreComponent::CanAddItemType(FGuid InventoryGuid, const UItemDefinition* ItemDef) const
{
    return PassesFilter(InventoryGuid, ItemDef);
}

float UInventoryCoreComponent::CalculateCurrentWeight(FGuid InventoryGuid) const
{
    const TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    if (!Inventory) return 0.0f;
    
    float TotalWeight = 0.0f;
    for (const FInventorySlot& Slot : *Inventory)
    {
        if (!Slot.IsEmpty())
        {
            TotalWeight += GetItemWeight(Slot.GetItemDefinition(), Slot.Quantity);
        }
    }
    return TotalWeight;
}

int32 UInventoryCoreComponent::GetInventoryCount() const
{
    return InventoryList.GetInventoryCount();
}

// =============================================================
// [내부 로직] InsertItem_Internal (핵심)
// =============================================================

int32 UInventoryCoreComponent::InsertItem_Internal(FGuid InventoryGuid, const UItemDefinition* ItemDef, int32 Quantity, UItemInstance* SpecificInstance)
{
    if (!HasInventory(InventoryGuid) || !ItemDef || Quantity <= 0) return 0;

    // 1. 필터 체크
    if (!PassesFilter(InventoryGuid, ItemDef)) return 0;

    // 2. 무게 체크
    FInventoryMetaData* Meta = GetMetaData(InventoryGuid);
    if (Meta && Meta->bUseWeight && WeightPolicy == EInventoryWeightPolicy::BlockOverflow)
    {
        float AddWeight = GetItemWeight(ItemDef, Quantity);
        if (Meta->CurrentWeight + AddWeight > Meta->MaxWeight) return 0;
    }

    TArray<FInventorySlot>* Slots = GetInventory(InventoryGuid);
    int32 AmountRemaining = Quantity;
    int32 AmountAdded = 0;

    // A. 스택 합치기 (Stacking) - Instance가 없을 때만 자동 합치기 권장
    // (Instance가 있으면 상태가 다를 수 있으므로 합치기 정책 필요하나 여기선 단순화)
    if (IsItemStackable(ItemDef) && !SpecificInstance)
    {
        while (AmountRemaining > 0)
        {
            int32 StackIdx = FindPartialStack(InventoryGuid, ItemDef);
            if (StackIdx == -1) break;

            FInventorySlot& Slot = (*Slots)[StackIdx];
            int32 Space = GetMaxStackSize(ItemDef) - Slot.Quantity;
            int32 Add = FMath::Min(AmountRemaining, Space);

            Slot.Quantity += Add;
            AmountRemaining -= Add;
            AmountAdded += Add;

            HandleInventoryChanged(InventoryGuid, StackIdx, EInventoryRefreshType::SingleSlot, ItemDef, true);
        }
    }

    // B. 빈 슬롯 채우기 (New Slot)
    while (AmountRemaining > 0)
    {
        int32 EmptyIdx = GetEmptySlotIndex(InventoryGuid);
        if (EmptyIdx == -1) break; 

        FInventorySlot& Slot = (*Slots)[EmptyIdx];
        
        if (SpecificInstance)
        {
            SpecificInstance->Rename(nullptr, this);
            Slot.ItemInstance = SpecificInstance;
            SpecificInstance = nullptr; // 사용됨
        }
        else
        {
            Slot.ItemInstance = CreateItemInstance(ItemDef);
        }

        int32 Add = IsItemStackable(ItemDef) ? FMath::Min(AmountRemaining, GetMaxStackSize(ItemDef)) : 1;
        Slot.Quantity = Add;
        
        AmountRemaining -= Add;
        AmountAdded += Add;

        HandleInventoryChanged(InventoryGuid, EmptyIdx, EInventoryRefreshType::SingleSlot, ItemDef, true);

        if (!SpecificInstance) break; // 인스턴스는 한 슬롯만 차지
    }

    return AmountAdded;
}

// ========================================
// Internal Helpers
// ========================================

void UInventoryCoreComponent::HandleInventoryChanged(FGuid InventoryGuid, int32 SlotIndex,
    EInventoryRefreshType RefreshType, const UItemDefinition* ItemDefAddedOrRemoved, bool bWasAdded)
{
    UpdateWeight(InventoryGuid);

    switch (RefreshType)
    {
    case EInventoryRefreshType::SingleSlot:
        OnInventoryChanged.Broadcast(InventoryGuid, SlotIndex);
        break;
    case EInventoryRefreshType::FullRefresh:
        OnInventoryChanged.Broadcast(InventoryGuid, -1);
        break;
    default:
        break;
    }
    
    if (ItemDefAddedOrRemoved)
    {
        if (bWasAdded) OnItemAdded.Broadcast(InventoryGuid, SlotIndex, ItemDefAddedOrRemoved);
        else OnItemRemoved.Broadcast(InventoryGuid, SlotIndex, ItemDefAddedOrRemoved);
    }
}

int32 UInventoryCoreComponent::FindPartialStack(FGuid InventoryGuid, const UItemDefinition* ItemDef) const
{
    if (!ItemDef || !IsItemStackable(ItemDef)) return -1;
    
    const TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    if (!Inventory) return -1;
    
    const int32 MaxStack = GetMaxStackSize(ItemDef);
    
    for (int32 i = 0; i < Inventory->Num(); ++i)
    {
        const FInventorySlot& Slot = (*Inventory)[i];
        if (Slot.GetItemDefinition() == ItemDef && Slot.Quantity < MaxStack) return i;
    }
    return -1;
}

bool UInventoryCoreComponent::IsItemStackable(const UItemDefinition* ItemDef) const
{
    return ItemDef && ItemDef->IsStackable();
}

int32 UInventoryCoreComponent::GetMaxStackSize(const UItemDefinition* ItemDef) const
{
    return ItemDef ? ItemDef->GetMaxStackSize() : 1;
}

float UInventoryCoreComponent::GetItemWeight(const UItemDefinition* ItemDef, int32 Quantity) const
{
    return ItemDef ? ItemDef->GetWeight() * Quantity : 0.0f;
}

UItemInstance* UInventoryCoreComponent::CreateItemInstance(const UItemDefinition* ItemDef)
{
    return ItemDef ? ItemDef->CreateInstance(this) : nullptr;
}

bool UInventoryCoreComponent::IsValidSlotIndex(FGuid InventoryGuid, int32 SlotIndex) const
{
    const TArray<FInventorySlot>* Inventory = GetInventory(InventoryGuid);
    return Inventory && Inventory->IsValidIndex(SlotIndex);
}

void UInventoryCoreComponent::UpdateWeight(FGuid InventoryGuid)
{
    FInventoryMetaData* MetaData = GetMetaData(InventoryGuid);
    if (!MetaData || !MetaData->bUseWeight) return;
    
    const float NewWeight = CalculateCurrentWeight(InventoryGuid);
    MetaData->CurrentWeight = NewWeight;
    OnWeightChanged.Broadcast(InventoryGuid, NewWeight);
}

bool UInventoryCoreComponent::PassesFilter(FGuid InventoryGuid, const UItemDefinition* ItemDef) const
{
    if (!ItemDef) return false;
    const FInventoryMetaData* MetaData = GetMetaData(InventoryGuid);
    if (!MetaData) return false;
    
    if (MetaData->AllowedTypes.IsEmpty()) return true;
    return ItemDef->HasAnyTag(MetaData->AllowedTypes);
}

void UInventoryCoreComponent::AddToStack(FGuid InventoryGuid, int32 SlotIndex, int32 Quantity)
{
    FInventorySlot* Slot = GetSlot(InventoryGuid, SlotIndex);
    if (Slot && !Slot->IsEmpty())
    {
        Slot->Quantity += Quantity;
        OnInventoryChanged.Broadcast(InventoryGuid, SlotIndex);
        UpdateWeight(InventoryGuid);
    }
}

FGuid UInventoryCoreComponent::GetGuidByName_NoLog(FName InventoryName) const
{
    return FindInventoryGuid(InventoryName);
}

void UInventoryCoreComponent::ServerMoveItemToSlot_Implementation(FGuid SourceGuid, int32 SourceIdx, FGuid DestGuid,
    int32 DestIdx)
{
    if (SourceGuid == DestGuid && SourceIdx == DestIdx) return;

    FInventorySlot* SourceSlot = GetSlot(SourceGuid, SourceIdx);
    FInventorySlot* DestSlot = GetSlot(DestGuid, DestIdx);

    if (!SourceSlot || !DestSlot || SourceSlot->IsEmpty()) return;

    // 3. 아이템 정의 및 타입 확인
    const UItemDefinition* SourceDef = SourceSlot->GetItemDefinition();
    const UItemDefinition* DestDef = DestSlot->GetItemDefinition();

    // 4. 도착지 필터 검사 (예: 장비 슬롯에 잘못된 아이템을 넣으려는지)
    // (이 부분은 GetSlot이나 별도 검증 함수에서 처리 가능)
    if (!PassesFilter(DestGuid, SourceDef)) return;

    // 5. 로직 분기
    if (DestSlot->IsEmpty())
    {
        // [CASE 1] 빈 슬롯으로 이동 (Move)
        *DestSlot = *SourceSlot;
        SourceSlot->Clear();
    }
    else if (SourceDef == DestDef && SourceDef->IsStackable())
    {
        // [CASE 2] 같은 아이템 & 스택 가능 -> 합치기 (Stack)
        int32 MaxStack = SourceDef->GetMaxStackSize();
        int32 SpaceRemaining = MaxStack - DestSlot->Quantity;

        if (SpaceRemaining > 0)
        {
            int32 AmountToMove = FMath::Min(SourceSlot->Quantity, SpaceRemaining);
            
            DestSlot->Quantity += AmountToMove;
            SourceSlot->Quantity -= AmountToMove;

            if (SourceSlot->Quantity <= 0)
            {
                SourceSlot->Clear();
            }
        }
        else
        {
            // 꽉 찼으면 교환(Swap)
            FInventorySlot Temp = *SourceSlot;
            *SourceSlot = *DestSlot;
            *DestSlot = Temp;
        }
    }
    else
    {
        // [CASE 3] 다른 아이템이거나 스택 불가 -> 교환 (Swap)
        // 도착지 아이템이 출발지에 들어갈 수 있는지(필터) 확인 필요
        if (PassesFilter(SourceGuid, DestDef))
        {
            FInventorySlot Temp = *SourceSlot;
            *SourceSlot = *DestSlot;
            *DestSlot = Temp;
        }
    }

    // 6. 변경 사항 알림 (Fast Array를 쓰면 MarkItemDirty 호출)
    if (FInventoryEntry* SourceEntry = InventoryList.FindInventoryByGuid(SourceGuid))
    {
        InventoryList.MarkItemDirty(*SourceEntry);
    }

    // 2. Dest 쪽 Entry 찾기 (Source와 다를 경우)
    if (SourceGuid != DestGuid)
    {
        if (FInventoryEntry* DestEntry = InventoryList.FindInventoryByGuid(DestGuid))
        {
            InventoryList.MarkItemDirty(*DestEntry);
        }
    }
    
    // UI 강제 갱신이 필요하다면 Delegate 호출
    HandleInventoryChanged(SourceGuid, SourceIdx, EInventoryRefreshType::SingleSlot, nullptr, false);
    HandleInventoryChanged(DestGuid, DestIdx, EInventoryRefreshType::SingleSlot, nullptr, true);
}


