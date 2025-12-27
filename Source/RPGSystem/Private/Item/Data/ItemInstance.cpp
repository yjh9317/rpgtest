// ItemInstance.cpp
#include "Item/Data/ItemInstance.h"
#include "Item/Data/ItemDefinition.h"
#include "Item/Data/Fragment/ItemFragment.h"
#include "Net/UnrealNetwork.h"

UItemInstance::UItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UItemInstance, ItemDef);
	DOREPLIFETIME(UItemInstance, ItemFragments);
}

void UItemInstance::AddFragment(UItemFragment* Fragment)
{
	if (Fragment)
	{
		ItemFragments.AddFragment(Fragment);
	}
}

// ========================================
// FItemFragmentList Implementation
// ========================================

void FItemFragmentList::AddFragment(UItemFragment* Fragment)
{
	if (Fragment)
	{
		FItemFragmentEntry& NewEntry = Entries.AddDefaulted_GetRef();
		NewEntry.Fragment = Fragment;
		MarkItemDirty(NewEntry);
	}
}

UItemFragment* FItemFragmentList::FindFragmentByClass(TSubclassOf<UItemFragment> FragmentClass) const
{
	if (!FragmentClass)
	{
		return nullptr;
	}

	for (const FItemFragmentEntry& Entry : Entries)
	{
		if (Entry.Fragment && Entry.Fragment->IsA(FragmentClass))
		{
			return Entry.Fragment;
		}
	}
	return nullptr;
}