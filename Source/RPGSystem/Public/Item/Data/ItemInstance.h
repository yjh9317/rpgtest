#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "GameplayTagContainer.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "ItemInstance.generated.h"

class UItemDefinition;
class UItemFragment;

/**
 * Fast Array Serializer for Fragment Replication
 * Lyra 스타일의 Fragment 네트워크 복제
 */
USTRUCT()
struct FItemFragmentEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UItemFragment> Fragment = nullptr;

	FItemFragmentEntry()
	{}

	FItemFragmentEntry(UItemFragment* InFragment)
		: Fragment(InFragment)
	{}
};

USTRUCT()
struct FItemFragmentList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FItemFragmentEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UItemInstance> OwnerInstance = nullptr;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FItemFragmentEntry, FItemFragmentList>(
			Entries, DeltaParms, *this);
	}

	void AddFragment(UItemFragment* Fragment);
	UItemFragment* FindFragmentByClass(TSubclassOf<UItemFragment> FragmentClass) const;
	
	template<typename T>
	const T* FindFragmentByClass() const
	{
		for (const FItemFragmentEntry& Entry : Entries)
		{
			if (Entry.Fragment && Entry.Fragment->IsA<T>())
			{
				return Cast<T>(Entry.Fragment);
			}
		}
		return nullptr;
	}

	template<typename T>
	T* FindMutableFragmentByClass()
	{
		for (FItemFragmentEntry& Entry : Entries)
		{
			if (Entry.Fragment && Entry.Fragment->IsA<T>())
			{
				return Cast<T>(Entry.Fragment);
			}
		}
		return nullptr;
	}
};

template<>
struct TStructOpsTypeTraits<FItemFragmentList> : public TStructOpsTypeTraitsBase2<FItemFragmentList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};

/**
 * 런타임 아이템 인스턴스
 * 
 * 각 아이템 인스턴스는:
 * - ItemDefinition에 대한 const 참조를 가지고
 * - Definition의 Fragment들을 복제하여 자신의 ItemFragments에 저장
 * - 각 Fragment는 자신의 런타임 상태를 자체적으로 관리
 */
UCLASS()
class RPGSYSTEM_API UItemInstance : public UObject
{
	GENERATED_BODY()
    
public:
	UItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UObject interface
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	/** 이 인스턴스가 참조하는 ItemDefinition (변하지 않음) */
	UPROPERTY(Replicated)
	TObjectPtr<const UItemDefinition> ItemDef = nullptr;
    
	/** 복제된 Fragment 인스턴스들 (Fast Array로 네트워크 복제) */
	UPROPERTY(Replicated)
	FItemFragmentList ItemFragments;

public:
	/** ItemDefinition 가져오기 */
	const UItemDefinition* GetItemDef() const { return ItemDef; }
    
	/** Fragment 검색 (const) */
	template<typename FragmentType>
	const FragmentType* FindFragmentByClass() const
	{
		return ItemFragments.FindFragmentByClass<FragmentType>();
	}

	/** Fragment 검색 (mutable) - 런타임 상태 변경용 */
	template<typename FragmentType>
	FragmentType* FindMutableFragmentByClass()
	{
		return ItemFragments.FindMutableFragmentByClass<FragmentType>();
	}

	/** Fragment 추가 (CreateInstance에서 사용) */
	void AddFragment(UItemFragment* Fragment);
};