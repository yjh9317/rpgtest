// InventoryFunctionLibrary.cpp 구현

#include "Inventory/InventoryFunctionLibrary.h"
#include "Item/Data/Fragment/ItemFragment_Durability.h"
#include "Item/Data/Fragment/ItemFragment_Equippable.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"

// ========================================
// Slot & Item Comparison
// ========================================

bool UInventoryFunctionLibrary::AreItemSlotsEqual(const FInventorySlot& FirstSlot, const FInventorySlot& SecondSlot)
{
    // Definition이 같으면 같은 아이템
    return FirstSlot.GetItemDefinition() == SecondSlot.GetItemDefinition();
}

bool UInventoryFunctionLibrary::AreItemsTheSame(const FInventorySlot& FirstSlot, const FInventorySlot& SecondSlot)
{
    // Definition과 Instance까지 모두 같아야 함 (완전히 동일한 객체)
    return FirstSlot.ItemInstance == SecondSlot.ItemInstance;
}

bool UInventoryFunctionLibrary::AreItemsStackable(const FInventorySlot& FirstSlot, const FInventorySlot& SecondSlot)
{
    if (FirstSlot.IsEmpty() || SecondSlot.IsEmpty()) return false;
    
    const UItemDefinition* DefA = FirstSlot.GetItemDefinition();
    const UItemDefinition* DefB = SecondSlot.GetItemDefinition();
    
    if (!DefA || !DefB) return false;
    if (DefA != DefB) return false;
    if (!DefA->IsStackable()) return false;
    
    return true;
}

// ========================================
// Equipment Slot Management
// ========================================

bool UInventoryFunctionLibrary::FindEmptyEquipmentSlot(
    const TMap<FName, FInventorySlot>& EquipmentSlots,
    const TArray<FName>& SlotsToSearch,
    FName DefaultSlot,
    FName& OutSlot)
{
    // // 지정된 슬롯들 중 빈 슬롯 찾기
    // for (const FName& SlotName : SlotsToSearch)
    // {
    //     if (const FInventorySlot* Slot = EquipmentSlots.Find(SlotName))
    //     {
    //         if (Slot->IsEmpty())
    //         {
    //             OutSlot = SlotName;
    //             return true;
    //         }
    //     }
    // }
    //
    // // 빈 슬롯이 없으면 기본 슬롯 반환
    // if (DefaultSlot != NAME_None)
    // {
    //     OutSlot = DefaultSlot;
    //     return true;
    // }
    
    return false;
}

bool UInventoryFunctionLibrary::FindCompatibleEquipmentSlot(
    const UItemDefinition* ItemDef,
    const TMap<FName, FInventorySlot>& EquipmentSlots,
    FName& OutSlot)
{
    if (!ItemDef) return false;
    
    // Equippable Fragment에서 소켓 이름 가져오기
    if (const UItemFragment_Equippable* EquipFragment = ItemDef->FindFragmentByClass<UItemFragment_Equippable>())
    {
        // const FName SocketName = EquipFragment->GetEquipmentSocket();
        //
        // // 해당 슬롯이 존재하는지 확인
        // if (EquipmentSlots.Contains(SocketName))
        // {
        //     OutSlot = SocketName;
        //     return true;
        // }
    }
    
    return false;
}

// ========================================
// Array Search Operations
// ========================================

bool UInventoryFunctionLibrary::HasPartialStack(
    const TArray<FInventorySlot>& Slots,
    const UItemDefinition* ItemDef,
    int32& OutSlotIndex)
{
    if (!ItemDef || !ItemDef->IsStackable())
    {
        OutSlotIndex = -1;
        return false;
    }
    
    const int32 MaxStackSize = ItemDef->GetMaxStackSize();
    
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].GetItemDefinition() == ItemDef)
        {
            if (Slots[i].Quantity < MaxStackSize)
            {
                OutSlotIndex = i;
                return true;
            }
        }
    }
    
    OutSlotIndex = -1;
    return false;
}

bool UInventoryFunctionLibrary::FindEmptySlotInArray(
    const TArray<FInventorySlot>& Slots,
    int32& OutIndex)
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].IsEmpty())
        {
            OutIndex = i;
            return true;
        }
    }
    
    OutIndex = -1;
    return false;
}

int32 UInventoryFunctionLibrary::FindItemIndexByDefinition(
    const TArray<FInventorySlot>& Slots,
    const UItemDefinition* ItemDef)
{
    if (!ItemDef) return -1;
    
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].GetItemDefinition() == ItemDef)
        {
            return i;
        }
    }
    
    return -1;
}

bool UInventoryFunctionLibrary::FindAllItemsByDefinition(
    const TArray<FInventorySlot>& Slots,
    const UItemDefinition* ItemDef,
    int32& OutTotalQuantity,
    TArray<int32>& OutSlotIndices)
{
    OutTotalQuantity = 0;
    OutSlotIndices.Empty();
    
    if (!ItemDef) return false;
    
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (Slots[i].GetItemDefinition() == ItemDef)
        {
            OutSlotIndices.Add(i);
            OutTotalQuantity += Slots[i].Quantity;
        }
    }
    
    return OutSlotIndices.Num() > 0;
}

int32 UInventoryFunctionLibrary::FindAmountOfEmptySlots(const TArray<FInventorySlot>& Slots)
{
    int32 EmptyCount = 0;
    for (const FInventorySlot& Slot : Slots)
    {
        if (Slot.IsEmpty())
        {
            ++EmptyCount;
        }
    }
    return EmptyCount;
}

// ========================================
// Item Validation
// ========================================

bool UInventoryFunctionLibrary::CanItemBeRemoved(const FInventorySlot& Slot)
{
    if (Slot.IsEmpty()) return false;
    
    if (const UItemFragment_Equippable* EquipFragment = 
        Slot.ItemInstance->FindFragmentByClass<UItemFragment_Equippable>())
    {
        // 장착 중인 아이템은 제거 불가
        // if (EquipFragment->IsEquipped())
        // {
        //     return false;
        // }
    }
    
    // TODO: 다른 조건들 추가 (잠금 상태, 퀘스트 아이템 등)
    
    return true;
}

bool UInventoryFunctionLibrary::IsItemDefinitionValid(const UItemDefinition* ItemDef)
{
    if (!ItemDef) return false;
    
    // 기본 검증
    if (ItemDef->ItemName.IsEmpty()) return false;
    if (ItemDef->bStackable && ItemDef->MaxStackSize < 1) return false;
    
    return true;
}

bool UInventoryFunctionLibrary::IsStackableAndHaveStacks(
    const FInventorySlot& Slot,
    int32 RequiredQuantity)
{
    if (Slot.IsEmpty()) return false;
    
    const UItemDefinition* ItemDef = Slot.GetItemDefinition();
    if (!ItemDef || !ItemDef->IsStackable()) return false;
    
    return Slot.Quantity >= RequiredQuantity;
}

// ========================================
// Value & Weight Calculations
// ========================================

float UInventoryFunctionLibrary::CalculateStackedItemValue(const FInventorySlot& Slot)
{
    if (Slot.IsEmpty()) return 0.0f;
    
    const UItemDefinition* ItemDef = Slot.GetItemDefinition();
    return ItemDef ? ItemDef->GetValue() * Slot.Quantity : 0.0f;
}

float UInventoryFunctionLibrary::CalculateStackedItemWeight(const FInventorySlot& Slot)
{
    if (Slot.IsEmpty()) return 0.0f;
    
    const UItemDefinition* ItemDef = Slot.GetItemDefinition();
    return ItemDef ? ItemDef->GetWeight() * Slot.Quantity : 0.0f;
}

// ========================================
// Weapon & Equipment Specific
// ========================================

bool UInventoryFunctionLibrary::CanWeaponsBeSwapped(
    const FInventorySlot& FirstSlot,
    const FInventorySlot& SecondSlot)
{
    // 둘 다 무기여야 함
    if (!IsItemEquipment(FirstSlot.GetItemDefinition()) ||
        !IsItemEquipment(SecondSlot.GetItemDefinition()))
    {
        return false;
    }
    
    // 같은 무기 타입이어야 함
    return AreWeaponTypesEqual(FirstSlot, SecondSlot);
}

bool UInventoryFunctionLibrary::AreWeaponTypesEqual(
    const FInventorySlot& FirstSlot,
    const FInventorySlot& SecondSlot)
{
    const FGameplayTag FirstType = GetWeaponTypeTag(FirstSlot.GetItemDefinition());
    const FGameplayTag SecondType = GetWeaponTypeTag(SecondSlot.GetItemDefinition());
    
    return FirstType.MatchesTagExact(SecondType);
}

FGameplayTag UInventoryFunctionLibrary::GetWeaponTypeTag(const UItemDefinition* ItemDef)
{
    if (!ItemDef) return FGameplayTag::EmptyTag;
    
    // Item.Type.Weapon.* 태그 찾기
    static const FGameplayTag WeaponParentTag = 
        FGameplayTag::RequestGameplayTag(FName("Item.Type.Weapon"));
    
    FGameplayTagContainer FilteredTags = ItemDef->ItemTags.Filter(FGameplayTagContainer(WeaponParentTag));
    
    if (FilteredTags.Num() > 0)
    {
        return FilteredTags.First();
    }
    
    return FGameplayTag::EmptyTag;
}

// ========================================
// Item Type Classification
// ========================================

bool UInventoryFunctionLibrary::IsItemCurrency(const UItemDefinition* ItemDef)
{
    if (!ItemDef) return false;
    
    static const FGameplayTag CurrencyTag = 
        FGameplayTag::RequestGameplayTag(FName("Item.Type.Currency"));
    
    return ItemDef->HasTag(CurrencyTag);
}

bool UInventoryFunctionLibrary::IsItemConsumable(const UItemDefinition* ItemDef)
{
    if (!ItemDef) return false;
    
    static const FGameplayTag ConsumableTag = 
        FGameplayTag::RequestGameplayTag(FName("Item.Type.Consumable"));
    
    return ItemDef->HasTag(ConsumableTag);
}

bool UInventoryFunctionLibrary::IsItemEquipment(const UItemDefinition* ItemDef)
{
    if (!ItemDef) return false;
    
    return ItemDef->FindFragmentByClass<UItemFragment_Equippable>() != nullptr;
}

TArray<FInventorySlot> UInventoryFunctionLibrary::GetAllItemsOfType(
    const TArray<FInventorySlot>& Slots,
    FGameplayTag TypeTag)
{
    TArray<FInventorySlot> Result;
    
    for (const FInventorySlot& Slot : Slots)
    {
        if (Slot.IsEmpty()) continue;
        
        const UItemDefinition* ItemDef = Slot.GetItemDefinition();
        if (ItemDef && ItemDef->HasTag(TypeTag))
        {
            Result.Add(Slot);
        }
    }
    
    return Result;
}

TArray<FInventorySlot> UInventoryFunctionLibrary::GetAllItemsWithTag(
    const TArray<FInventorySlot>& Slots,
    FGameplayTag Tag)
{
    return GetAllItemsOfType(Slots, Tag);
}

// ========================================
// Fragment-Specific Queries
// ========================================

bool UInventoryFunctionLibrary::GetItemDurabilityInfo(
    const FInventorySlot& Slot,
    float& OutCurrentDurability,
    float& OutMaxDurability)
{
    if (Slot.IsEmpty() || !Slot.ItemInstance)
    {
        OutCurrentDurability = 0.0f;
        OutMaxDurability = 0.0f;
        return false;
    }
    
    if (UItemFragment_Durability* DurabilityFragment = 
        Slot.ItemInstance->FindMutableFragmentByClass<UItemFragment_Durability>())
    {
        // OutCurrentDurability = DurabilityFragment->GetCurrentDurability();
        // OutMaxDurability = DurabilityFragment->GetMaxDurability();
        return true;
    }
    
    return false;
}

bool UInventoryFunctionLibrary::GetItemEquipmentInfo(
    const FInventorySlot& Slot,
    FName& OutSocketName,
    bool& bOutIsEquipped)
{
    if (Slot.IsEmpty() || !Slot.ItemInstance)
    {
        OutSocketName = NAME_None;
        bOutIsEquipped = false;
        return false;
    }
    
    if (UItemFragment_Equippable* EquipFragment = 
        Slot.ItemInstance->FindMutableFragmentByClass<UItemFragment_Equippable>())
    {
        // OutSocketName = EquipFragment->GetEquipmentSocket();
        // bOutIsEquipped = EquipFragment->IsEquipped();
        return true;
    }
    
    return false;
}

// ========================================
// Network & Authority
// ========================================

bool UInventoryFunctionLibrary::HasOwnerAuthority(UActorComponent* Component)
{
    if (!Component) return false;
    
    AActor* Owner = Component->GetOwner();
    if (!Owner) return false;
    
    return Owner->HasAuthority();
}

bool UInventoryFunctionLibrary::IsLocallyControlled(UActorComponent* Component)
{
    if (!Component) return false;
    
    AActor* Owner = Component->GetOwner();
    if (!Owner) return false;
    
    if (APawn* Pawn = Cast<APawn>(Owner))
    {
        return Pawn->IsLocallyControlled();
    }
    
    return false;
}

// ========================================
// UI Helpers
// ========================================

void UInventoryFunctionLibrary::LoadItemIconAsync(
    const UItemDefinition* ItemDef,
    UImage* TargetImage)
{
    if (!ItemDef || !TargetImage) return;
    
    if (ItemDef->Icon.IsValid())
    {
        TargetImage->SetBrushFromSoftTexture(ItemDef->Icon);
    }
    else if (!ItemDef->Icon.IsNull())
    {
        // TWeakObjectPtr로 약한 참조 생성
        TWeakObjectPtr<UImage> WeakImage = TargetImage;
        
        FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
        Streamable.RequestAsyncLoad(
            ItemDef->Icon.ToSoftObjectPath(),
            FStreamableDelegate::CreateLambda([WeakImage, IconPath = ItemDef->Icon]()
            {
                // 위젯이 여전히 유효한지 확인
                if (UImage* Image = WeakImage.Get())
                {
                    if (UTexture2D* LoadedTexture = IconPath.Get())
                    {
                        Image->SetBrushFromTexture(LoadedTexture);
                    }
                }
            })
        );
    }
}


void UInventoryFunctionLibrary::LoadSlotIconAsync(
    const FInventorySlot& Slot,
    UImage* TargetImage)
{
    if (Slot.IsEmpty())
    {
        if (TargetImage)
        {
            TargetImage->SetVisibility(ESlateVisibility::Collapsed);
        }
        return;
    }
    
    LoadItemIconAsync(Slot.GetItemDefinition(), TargetImage);
}

UTexture2D* UInventoryFunctionLibrary::LoadItemIconBlocking(const UItemDefinition* ItemDef)
{
    if (!ItemDef) return nullptr;
    
    if (ItemDef->Icon.IsValid())
    {
        return ItemDef->Icon.Get();
    }
    else if (!ItemDef->Icon.IsNull())
    {
        // 동기 로딩 (블로킹)
        return ItemDef->Icon.LoadSynchronous();
    }
    
    return nullptr;
}

FText UInventoryFunctionLibrary::FormatItemDescription(
    const UItemDefinition* ItemDef,
    bool bIncludeStats)
{
    if (!ItemDef) return FText::GetEmpty();
    
    FString Description = ItemDef->Description.ToString();
    
    if (bIncludeStats)
    {
        // 스탯 정보 추가
        Description += FString::Printf(TEXT("\n\nWeight: %.1f"), ItemDef->Weight);
        Description += FString::Printf(TEXT("\nValue: %.0f"), ItemDef->BaseValue);
        
        if (ItemDef->bStackable)
        {
            Description += FString::Printf(TEXT("\nMax Stack: %d"), ItemDef->MaxStackSize);
        }
    }
    
    return FText::FromString(Description);
}

FText UInventoryFunctionLibrary::GenerateItemTooltip(const FInventorySlot& Slot)
{
    if (Slot.IsEmpty()) return FText::GetEmpty();
    
    const UItemDefinition* ItemDef = Slot.GetItemDefinition();
    if (!ItemDef) return FText::GetEmpty();
    
    FString Tooltip;
    
    // 이름
    Tooltip += ItemDef->ItemName.ToString();
    
    // 수량
    if (Slot.Quantity > 1)
    {
        Tooltip += FString::Printf(TEXT(" (x%d)"), Slot.Quantity);
    }
    
    Tooltip += TEXT("\n\n");
    
    // 설명
    Tooltip += ItemDef->Description.ToString();
    
    // 내구도
    float CurrentDurability, MaxDurability;
    if (GetItemDurabilityInfo(Slot, CurrentDurability, MaxDurability))
    {
        Tooltip += FString::Printf(TEXT("\n\nDurability: %.0f / %.0f"), 
            CurrentDurability, MaxDurability);
    }
    
    // 무게와 가치
    Tooltip += FString::Printf(TEXT("\n\nWeight: %.1f"), 
        CalculateStackedItemWeight(Slot));
    Tooltip += FString::Printf(TEXT("\nValue: %.0f"), 
        CalculateStackedItemValue(Slot));
    
    return FText::FromString(Tooltip);
}