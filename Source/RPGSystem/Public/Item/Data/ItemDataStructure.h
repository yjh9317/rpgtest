// ItemDataStructure.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ItemDefinition.h"
#include "ItemInstance.h"
#include "ItemDataStructure.generated.h"

class UItemDefinition;
class UItemInstance;

/**
 * 인벤토리 슬롯
 * 
 * - ItemInstance를 직접 참조 (CDO가 아닌 실제 인스턴스)
 * - 각 슬롯은 고유한 런타임 상태를 가진 아이템을 보관
 */
USTRUCT(BlueprintType)
struct FInventorySlot
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 SlotIndex = -1;
	
	/** 실제 아이템 인스턴스 (런타임 상태 포함) */
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	TObjectPtr<UItemInstance> ItemInstance = nullptr;
	
	UPROPERTY(BlueprintReadWrite, Category = "Inventory")
	int32 Quantity = 0;

	const UItemDefinition* GetItemDefinition() const
	{
		return ItemInstance ? ItemInstance->GetItemDef() : nullptr;
	}

	FText GetItemName() const
	{
		const UItemDefinition* ItemDef = GetItemDefinition();
		return ItemDef ? ItemDef->ItemName : FText::GetEmpty();
	}
	bool IsEmpty() const { return ItemInstance == nullptr || Quantity <= 0; }
	bool IsValid() const { return !IsEmpty(); }
	void Clear() { ItemInstance = nullptr; Quantity = 0; }
};

/**
 * 인벤토리 메타데이터
 * 인벤토리의 제약사항 및 설정
 */
USTRUCT(BlueprintType)
struct FInventoryMetaData
{
	GENERATED_BODY()

	// 식별자 검색용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName InventoryName;  // "Main", "Equipment", "Storage_001"
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText DisplayName;  // "메인 인벤토리", "장비"
	
	// ========================================
	// Size
	// ========================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
	int32 MaxSlots = 30;
	
	// ========================================
	// Weight System
	// ========================================
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	bool bUseWeight = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight", meta = (EditCondition = "bUseWeight"))
	float MaxWeight = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	float CurrentWeight = 0.0f;
	
	// ========================================
	// Filter
	// ========================================
	
	/** 허용되는 아이템 타입 (비어있으면 모두 허용) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", meta = (Categories = "Item"))
	FGameplayTagContainer AllowedTypes;
};

/**
 * 인벤토리 생성 설정
 */
USTRUCT(BlueprintType)
struct FInventoryCreateConfig : public FTableRowBase
{
	GENERATED_BODY()

	// ========================================
	// Default
	// ========================================
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName InventoryName = NAME_None;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
	FText DisplayName;
    
	// ========================================
	// 크기
	// ========================================
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
	int32 SlotCount = 30;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Size")
	int32 MaxSlots = 30;
    
	// ========================================
	// Weight
	// ========================================
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight")
	bool bUseWeight = false;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weight", meta = (EditCondition = "bUseWeight"))
	float MaxWeight = 100.0f;
    
	// ========================================
	// Filter
	// ========================================
    
	/** 허용되는 아이템 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter", meta = (Categories = "Item"))
	FGameplayTagContainer AllowedTypes;
    
	// ========================================
	// DataTable 전용 옵션
	// ========================================
    
	/** DataTable에서 로드 시 자동으로 생성할지 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	bool bAutoCreateFromTable = true;
    
	/** 생성 우선순위 (낮을수록 먼저 생성) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DataTable")
	int32 CreationPriority = 0;
};

USTRUCT()
struct FInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()
public:
	/** 인벤토리 고유 ID */
	UPROPERTY()
	FGuid InventoryGuid;
    
	/** 메타데이터 */
	UPROPERTY()
	FInventoryMetaData MetaData;
    
	/** 슬롯 배열 */
	UPROPERTY()
	TArray<FInventorySlot> Slots;

	FInventoryEntry()
		: InventoryGuid()
	{}

	FInventoryEntry(const FGuid& InGuid, const FInventoryMetaData& InMetaData, const TArray<FInventorySlot>& InSlots)
		: InventoryGuid(InGuid)
		, MetaData(InMetaData)
		, Slots(InSlots)
	{}
};

class UInventoryCoreComponent;

/**
 * 인벤토리 목록 (네트워크 복제용)
 * FFastArraySerializer를 상속받아 델타 복제 지원
 */

USTRUCT()
struct FInventoryList : public FFastArraySerializer
{
    GENERATED_BODY()

    /** 인벤토리 엔트리 배열 */
    UPROPERTY()
    TArray<FInventoryEntry> Entries;

    /** Owner 컴포넌트 참조 (델리게이트 브로드캐스트용) */
    UPROPERTY(NotReplicated)
    TObjectPtr<UInventoryCoreComponent> OwnerComponent = nullptr;

    // ========================================
    // Fast Array Serializer Interface
    // ========================================

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FInventoryEntry, FInventoryList>(
            Entries, DeltaParms, *this);
    }

    // ========================================
    // 콜백 함수들 (복제 시 자동 호출)
    // ========================================

    /** 아이템이 추가되었을 때 */
    void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
    
    /** 아이템이 변경되었을 때 */
    void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
    
    /** 아이템이 제거되었을 때 */
    void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);

    // ========================================
    // 헬퍼 함수들
    // ========================================

    /** GUID로 인벤토리 찾기 */
    FInventoryEntry* FindInventoryByGuid(const FGuid& InGuid);
    const FInventoryEntry* FindInventoryByGuid(const FGuid& InGuid) const;
    
    /** 이름으로 인벤토리 찾기 */
    FInventoryEntry* FindInventoryByName(FName InName);
    const FInventoryEntry* FindInventoryByName(FName InName) const;
    
    /** 인벤토리 추가 */
    FInventoryEntry& AddInventory(const FGuid& InGuid, const FInventoryMetaData& InMetaData, const TArray<FInventorySlot>& InSlots);
    
    /** 인벤토리 제거 */
    bool RemoveInventory(const FGuid& InGuid);
	
	TArray<FName> GetAllInventoryNames() const;
	TArray<FGuid> GetAllInventoryGuids() const;
	int32 GetInventoryCount() const;
	bool IsEmpty() const;
};

// TStructOpsTypeTraits 특수화 (필수)
template<>
struct TStructOpsTypeTraits<FInventoryList> : public TStructOpsTypeTraitsBase2<FInventoryList>
{
    enum
    {
        WithNetDeltaSerializer = true,
    };
};