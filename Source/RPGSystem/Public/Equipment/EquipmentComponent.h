// EquipmentComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Net/Serialization/FastArraySerializer.h" // 필수 헤더
#include "EquipmentComponent.generated.h"

class UInventoryCoreComponent;
class UItemInstance;
class UEquipmentComponent;

// 1. 장비 슬롯 데이터 (TMap의 Key-Value 쌍을 대체)
USTRUCT(BlueprintType)
struct FEquipmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag SlotTag; 

	UPROPERTY()
	TObjectPtr<UItemInstance> ItemInstance; 

	FEquipmentEntry() : ItemInstance(nullptr) {}
	FEquipmentEntry(FGameplayTag InSlot, UItemInstance* InItem) 
		: SlotTag(InSlot), ItemInstance(InItem) {}

	bool operator==(const FEquipmentEntry& Other) const
	{
		return SlotTag == Other.SlotTag && ItemInstance == Other.ItemInstance;
	}
};

// 2. 장비 리스트 컨테이너 (TMap을 대체하는 배열 래퍼)
USTRUCT(BlueprintType)
struct FEquipmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FEquipmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UEquipmentComponent> OwnerComponent;

	// Fast Array 필수 구현: 델타 직렬화
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FEquipmentEntry, FEquipmentList>(Entries, DeltaParms, *this);
	}

	// 아이템 추가/수정
	void AddOrUpdateEntry(FGameplayTag SlotTag, UItemInstance* Item);
	// 아이템 제거
	bool RemoveEntry(FGameplayTag SlotTag);
	// 아이템 찾기 (Getter)
	UItemInstance* FindItem(FGameplayTag SlotTag) const;
	// 슬롯이 비어있는지 확인
	bool Contains(FGameplayTag SlotTag) const;

	// 서버->클라이언트 복제 시 호출되는 콜백 (UI/비주얼 갱신용)
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
};

// Fast Array 특성 정의 (필수)
template<>
struct TStructOpsTypeTraits<FEquipmentList> : public TStructOpsTypeTraitsBase2<FEquipmentList>
{
	enum { WithNetDeltaSerializer = true };
};

// ---------------------------------------------------
// 컴포넌트 클래스
// ---------------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentUpdated, FGameplayTag, SlotTag, const UItemInstance*, ItemInstance);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSYSTEM_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UEquipmentComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	
	FOnEquipmentUpdated OnEquipmentUpdated;

	void InitializeEquipment(UInventoryCoreComponent* InInventoryComp);
	bool EquipItem(UItemInstance* ItemToEquip);
	bool UnequipItem(FGameplayTag SlotTag);
	UItemInstance* GetEquippedItem(FGameplayTag SlotTag) const;
	void BroadcastEquipmentUpdate(FGameplayTag SlotTag, UItemInstance* ItemInstance);
	
protected:
	void ApplyEquipmentVisuals(FGameplayTag SlotTag, UItemInstance* Item);
	void UnapplyEquipmentVisuals(FGameplayTag SlotTag);
	void ApplyEquipmentStats(UItemInstance* Item);
	void UnapplyEquipmentStats(UItemInstance* Item);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TArray<FGameplayTag> ValidEquipmentSlots;
	
	// 장비 아이템이 보관되는 인벤토리 이름 (예: "Equipment")
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	FName EquipmentInventoryName = FName("Equipment");

	// 캐싱된 장비 인벤토리 GUID
	UPROPERTY(BlueprintReadOnly, Category = "Config")
	FGuid EquipmentInventoryGUID;

	// 현재 장착 중인 아이템들 (Fast Array)
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	FEquipmentList EquippedItems;

	UPROPERTY()
	TObjectPtr<UInventoryCoreComponent> InventoryCore;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<AActor>> SpawnedEquipmentActors;

	friend struct FEquipmentList;
};