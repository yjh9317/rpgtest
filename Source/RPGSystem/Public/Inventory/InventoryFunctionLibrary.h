// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Data/ItemDataStructure.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "InventoryFunctionLibrary.generated.h"

struct FStreamableHandle;
class UImage;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** 두 슬롯이 같은 아이템을 가지고 있는지 (Definition 비교) */
	static bool AreItemSlotsEqual(const FInventorySlot& FirstSlot, const FInventorySlot& SecondSlot);
	/** 두 슬롯의 아이템이 완전히 동일한지 (Instance까지 비교) */
	static bool AreItemsTheSame(const FInventorySlot& FirstSlot, const FInventorySlot& SecondSlot);
	/** 두 아이템이 스택 가능한지 확인 */
	static bool AreItemsStackable(const FInventorySlot& FirstSlot, const FInventorySlot& SecondSlot);
	/** 장비 슬롯에서 빈 슬롯 찾기 */
	static bool FindEmptyEquipmentSlot(
		const TMap<FName, FInventorySlot>& EquipmentSlots,
		const TArray<FName>& SlotsToSearch,
		FName DefaultSlot,
		FName& OutSlot);
	
	/** 아이템이 장착 가능한 슬롯 찾기 */
	static bool FindCompatibleEquipmentSlot(
		const UItemDefinition* ItemDef,
		const TMap<FName, FInventorySlot>& EquipmentSlots,
		FName& OutSlot);
	
	// ========================================
	// Array Search Operations
	// ========================================
    
	/** 부분 스택 찾기 (같은 아이템이지만 최대 스택이 아닌 슬롯) */
	static bool HasPartialStack(
		const TArray<FInventorySlot>& Slots,
		const UItemDefinition* ItemDef,
		int32& OutSlotIndex);
    
	/** 배열에서 빈 슬롯 찾기 */
	static bool FindEmptySlotInArray(
		const TArray<FInventorySlot>& Slots,
		int32& OutIndex);
    
	/** Definition으로 아이템 인덱스 찾기 */
	static int32 FindItemIndexByDefinition(
		const TArray<FInventorySlot>& Slots,
		const UItemDefinition* ItemDef);
    
	/** 특정 조건의 모든 아이템 찾기 */
	static bool FindAllItemsByDefinition(
		const TArray<FInventorySlot>& Slots,
		const UItemDefinition* ItemDef,
		int32& OutTotalQuantity,
		TArray<int32>& OutSlotIndices);
    
	/** 빈 슬롯 개수 세기 */
	static int32 FindAmountOfEmptySlots(const TArray<FInventorySlot>& Slots);
	
	// ========================================
	// Item Validation
	// ========================================
    
	/** 아이템이 제거 가능한지 확인 (장착 상태, 잠금 등 체크) */
	static bool CanItemBeRemoved(const FInventorySlot& Slot);
    
	/** ItemDefinition이 유효한지 확인 */
	static bool IsItemDefinitionValid(const UItemDefinition* ItemDef);
    
	/** 아이템이 스택 가능하고 스택이 있는지 */
	static bool IsStackableAndHaveStacks(const FInventorySlot& Slot, int32 RequiredQuantity);
	
	// ========================================
	// Value & Weight Calculations (Extended)
	// ========================================
    
	/** 스택된 아이템의 총 가치 계산 */
	static float CalculateStackedItemValue(const FInventorySlot& Slot);
    
	/** 스택된 아이템의 총 무게 계산 */
	static float CalculateStackedItemWeight(const FInventorySlot& Slot);
	
	// ========================================
	// Weapon & Equipment Specific
	// ========================================
    
	/** 두 무기가 교환 가능한지 확인 */
	static bool CanWeaponsBeSwapped(const FInventorySlot& FirstSlot, const FInventorySlot& SecondSlot);
    
	/** 두 무기의 타입이 같은지 확인 */
	static bool AreWeaponTypesEqual(const FInventorySlot& FirstSlot, const FInventorySlot& SecondSlot);
    
	/** 무기의 타입 태그 가져오기 */
	static FGameplayTag GetWeaponTypeTag(const UItemDefinition* ItemDef);
	
	    // ========================================
    // Item Type Classification
    // ========================================
    
    /** 아이템이 화폐인지 확인 */
    static bool IsItemCurrency(const UItemDefinition* ItemDef);
    
    /** 아이템이 소비 아이템인지 확인 */
    static bool IsItemConsumable(const UItemDefinition* ItemDef);
    
    /** 아이템이 장비인지 확인 */
    static bool IsItemEquipment(const UItemDefinition* ItemDef);
    
    /** 특정 타입의 모든 아이템 가져오기 */
    static TArray<FInventorySlot> GetAllItemsOfType(
        const TArray<FInventorySlot>& Slots,
        FGameplayTag TypeTag);
    
    /** 특정 태그를 가진 모든 아이템 가져오기 */
    static TArray<FInventorySlot> GetAllItemsWithTag(
        const TArray<FInventorySlot>& Slots,
        FGameplayTag Tag);

    // ========================================
    // Fragment-Specific Queries
    // ========================================
    
    /** 아이템에 특정 Fragment가 있는지 확인 */
    template<typename T>
    static bool HasFragmentOfType(const FInventorySlot& Slot)
    {
        if (!Slot.ItemInstance) return false;
        return Slot.ItemInstance->FindFragmentByClass<T>() != nullptr;
    }
    
    /** 아이템의 내구도 정보 가져오기 */
    static bool GetItemDurabilityInfo(
        const FInventorySlot& Slot,
        float& OutCurrentDurability,
        float& OutMaxDurability);
    
    /** 아이템의 장착 정보 가져오기 */
    static bool GetItemEquipmentInfo(
        const FInventorySlot& Slot,
        FName& OutSocketName,
        bool& bOutIsEquipped);

    // ========================================
    // Network & Authority
    // ========================================
    
    /** 컴포넌트의 Owner가 Authority를 가지고 있는지 확인 */
    static bool HasOwnerAuthority(UActorComponent* Component);
    
    /** 컴포넌트가 로컬로 제어되는지 확인 */
    static bool IsLocallyControlled(UActorComponent* Component);

    // ========================================
    // UI Helpers (Extended)
    // ========================================
    
    /** 아이템 아이콘 비동기 로드 */
    static void LoadItemIconAsync(
        const UItemDefinition* ItemDef,
        UImage* TargetImage);
	
    /** 슬롯 아이콘 비동기 로드 */
    static void LoadSlotIconAsync(
        const FInventorySlot& Slot,
        UImage* TargetImage);
	
    /** 아이템 아이콘 동기 로드 (블로킹) */
    static UTexture2D* LoadItemIconBlocking(const UItemDefinition* ItemDef);
    
    /** 아이템 설명 포맷팅 */
    static FText FormatItemDescription(
        const UItemDefinition* ItemDef,
        bool bIncludeStats = true);
    
    /** 아이템 툴팁 텍스트 생성 */
    static FText GenerateItemTooltip(const FInventorySlot& Slot);
	
};
