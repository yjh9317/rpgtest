// InventoryCoreComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Item/Data/ItemDataStructure.h"
#include "Item/Data/ItemInitializationData.h"
#include "InventoryCoreComponent.generated.h"

class UItemDefinition;
class UItemInstance;
class APlayerController;

UENUM(BlueprintType)
enum class EInventoryRefreshType : uint8
{
    None            UMETA(DisplayName = "None"),          // UI 갱신 필요 없음 (내부 연산 등)
    SingleSlot      UMETA(DisplayName = "Single Slot"),   // 특정 슬롯만 갱신
    FullRefresh     UMETA(DisplayName = "Full Refresh")   // 인벤토리 전체 갱신
};

UENUM()
enum class EInventoryWeightPolicy : uint8
{
    AllowOverflow,   // 초과 허용
    BlockOverflow    // 초과 시 추가 불가
};

// 델리게이트 선언
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryChanged, FGuid /*InventoryGuid*/, int32 /*SlotIndex*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemAdded, FGuid /*InventoryGuid*/, int32 /*SlotIndex*/, const UItemDefinition* /*ItemDef*/);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnItemRemoved, FGuid /*InventoryGuid*/, int32 /*SlotIndex*/, const UItemDefinition* /*ItemDef*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnWeightChanged, FGuid /*InventoryGuid*/, float /*CurrentWeight*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnInventoryCleared, FGuid /*InventoryGuid*/, FName /*InventoryName*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSYSTEM_API UInventoryCoreComponent : public UActorComponent
{
    GENERATED_BODY()
    
    friend class UInventoryInitializer;
public:
    UInventoryCoreComponent(const FObjectInitializer& ObjectInitializer);

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
  
    UFUNCTION(Server,Reliable)
    void ServerMoveItemToSlot(FGuid SourceGuid, int32 SourceIdx, FGuid DestGuid, int32 DestIdx);
    
    FOnInventoryChanged OnInventoryChanged;
    FOnItemAdded OnItemAdded;
    FOnItemRemoved OnItemRemoved;
    FOnWeightChanged OnWeightChanged;
    FOnInventoryCleared OnInventoryCleared;

protected:
    UPROPERTY(Replicated)
    FInventoryList InventoryList;
   
    /** 에디터에서 직접 설정하는 인벤토리 설정들 */
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Initialization")
    TArray<FInventoryCreateConfig> AutoCreateInventories;
    
    /** DataTable에서 인벤토리 설정 로드 */
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Initialization")
    UDataTable* InventoryConfigTable;
    
    /** DataTable에서 초기 아이템 로드 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Initial Items")
    TArray<UDataTable*> InitialItemTables;
    
    /** 개별 초기 아이템 스택 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Initial Items")
    TArray<FItemStack> SingleInitialItems;
    
    /** 랜덤 루트 테이블 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Initial Items")
    TArray<FRandomizedLootTable> RandomLootTables;
    
    /** 초기 아이템을 어느 인벤토리에 넣을지 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Initial Items")
    FName DefaultInventoryForInitialItems = "Main";
    
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Inventory|Sorting")
    bool bSortInitialItems = false;

    UPROPERTY(EditAnywhere, Instanced, Category = "Inventory|Initialization")
    TObjectPtr<UInventoryInitializer> InventoryInitializer;
    
    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Weight")
    EInventoryWeightPolicy WeightPolicy = EInventoryWeightPolicy::BlockOverflow;
public:
    // ========================================
    // 초기화 및 인벤토리 관리
    // ========================================
    virtual void InitializeInventory();
    FGuid CreateInventory(const FInventoryCreateConfig& Config);
    bool DestroyInventory(FGuid InventoryGuid);
    bool DestroyInventoryByName(FName InventoryName);

    // ========================================
    // 조회 (Query)
    // ========================================
    TArray<FInventorySlot>* GetInventory(FGuid InventoryGuid);
    const TArray<FInventorySlot>* GetInventory(FGuid InventoryGuid) const;
    
    FInventoryMetaData* GetMetaData(FGuid InventoryGuid);
    const FInventoryMetaData* GetMetaData(FGuid InventoryGuid) const;
    
    FInventorySlot* GetSlot(FGuid InventoryGuid, int32 SlotIndex);
    const FInventorySlot* GetSlot(FGuid InventoryGuid, int32 SlotIndex) const;
    
    bool HasInventory(FGuid InventoryGuid) const;
    bool IsInventoryFull(FGuid InventoryGuid) const;
    int32 GetEmptySlotIndex(FGuid InventoryGuid) const;
    FGuid FindInventoryGuid(FName InventoryName) const;
    bool GetInventoryByName(FName InventoryName, TArray<FInventorySlot>& OutSlots) const;
    TArray<FName> GetAllInventoryNames() const;
    TArray<FGuid> GetAllInventoryGuids() const;

    // =============================================================
    // [메인 API] 게임플레이 중 아이템 획득 (Looting)
    // =============================================================
    
    int32 LootNewItem(const UItemDefinition* ItemDef, int32 Quantity = 1);
    bool LootItemInstance(UItemInstance* InInstance, int32 Quantity = 1);

    int32 AddItemToInventory(FGuid InventoryGuid, const UItemDefinition* ItemDef, int32 Quantity);
    bool AddItemToSlot(FGuid InventoryGuid, int32 SlotIndex, UItemInstance* InInstance, int32 Quantity);
    bool AddNewItemToSlot(FGuid InventoryGuid, int32 SlotIndex, const UItemDefinition* ItemDef, int32 Quantity);

    /** 아이템 제거 */
    bool RemoveItem(FGuid InventoryGuid, int32 SlotIndex, int32 Quantity = 1);
    bool RemoveItemByDef(FGuid InventoryGuid, const UItemDefinition* ItemDef, int32 Quantity = 1);
    bool RemoveItemFromSpecificInventory(FGuid InventoryGuid, UItemInstance* ItemInstance);
    // ========================================
    // 이동 / 교환 / 슬롯 관리
    // ========================================
    
    bool MoveItem(FGuid FromInventoryGuid, int32 FromSlot, FGuid ToInventoryGuid, int32 ToSlot);
    bool SwapItems(FGuid InventoryAGuid, int32 SlotA, FGuid InventoryBGuid, int32 SlotB);
    bool AddSlots(FGuid InventoryGuid, int32 Count);
    void CompactInventory(FGuid InventoryGuid);
    void ClearInventory(FGuid InventoryGuid);

    // ========================================
    // 유틸리티 / 계산
    // ========================================
    
    int32 CountItemByDef(FGuid InventoryGuid, const UItemDefinition* ItemDef) const;
    bool CanAddItemType(FGuid InventoryGuid, const UItemDefinition* ItemDef) const;
    float CalculateCurrentWeight(FGuid InventoryGuid) const;
    int32 GetInventoryCount() const;

protected:

    /**
     * 실제 아이템 삽입 로직 (스택 합치기 -> 빈 슬롯 찾기 -> 무게 체크)
     * @param SpecificInstance : nullptr이면 새 아이템 생성, 값이 있으면 해당 인스턴스 사용
     * @return 실제로 추가된 수량
     */
    int32 InsertItem_Internal(FGuid InventoryGuid, const UItemDefinition* ItemDef, int32 Quantity, UItemInstance* SpecificInstance);

    void HandleInventoryChanged(FGuid InventoryGuid, int32 SlotIndex, EInventoryRefreshType RefreshType, const UItemDefinition* ItemDefAddedOrRemoved, bool bWasAdded);
    
    int32 FindPartialStack(FGuid InventoryGuid, const UItemDefinition* ItemDef) const;
    bool IsItemStackable(const UItemDefinition* ItemDef) const;
    int32 GetMaxStackSize(const UItemDefinition* ItemDef) const;
    float GetItemWeight(const UItemDefinition* ItemDef, int32 Quantity) const;
    
    UItemInstance* CreateItemInstance(const UItemDefinition* ItemDef);
    bool IsValidSlotIndex(FGuid InventoryGuid, int32 SlotIndex) const;
    void UpdateWeight(FGuid InventoryGuid);
    bool PassesFilter(FGuid InventoryGuid, const UItemDefinition* ItemDef) const;
    void AddToStack(FGuid InventoryGuid, int32 SlotIndex, int32 Quantity);  
    FGuid GetGuidByName_NoLog(FName InventoryName) const;
    
};
