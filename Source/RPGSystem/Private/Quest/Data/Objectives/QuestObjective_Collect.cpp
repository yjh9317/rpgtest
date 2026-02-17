// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objectives/QuestObjective_Collect.h"

#include "GameFramework/Pawn.h"
#include "Inventory/InventoryCoreComponent.h"
#include "Item/Data/ItemDefinition.h"
#include "Quest/Components/QuestManagerComponent.h"
#include "Quest/RPGQuest.h"

void UQuestObjective_Collect::ActivateObjective(URPGQuest* OwnerQuest)
{
	Super::ActivateObjective(OwnerQuest);
	CurrentAmount = 0;

	if (!TargetItem || !OwningQuest)
	{
		return;
	}

	UQuestManagerComponent* Manager = Cast<UQuestManagerComponent>(OwningQuest->GetOuter());
	AActor* ManagerOwner = Manager ? Manager->GetOwner() : nullptr;
	APawn* PawnOwner = Cast<APawn>(ManagerOwner);
	if (!PawnOwner)
	{
		return;
	}

	UInventoryCoreComponent* Inventory = PawnOwner->FindComponentByClass<UInventoryCoreComponent>();
	if (!Inventory)
	{
		return;
	}

	CachedInventoryComponent = Inventory;
	InventoryChangedHandle = Inventory->OnInventoryChanged.AddUObject(this, &UQuestObjective_Collect::HandleInventoryChanged);
	CheckInventory();
}

void UQuestObjective_Collect::DeactivateObjective()
{
	if (UInventoryCoreComponent* Inventory = CachedInventoryComponent.Get())
	{
		if (InventoryChangedHandle.IsValid())
		{
			Inventory->OnInventoryChanged.Remove(InventoryChangedHandle);
		}
	}
	CachedInventoryComponent.Reset();
	InventoryChangedHandle.Reset();

	Super::DeactivateObjective();
}

void UQuestObjective_Collect::CheckInventory()
{
	if (!TargetItem)
	{
		return;
	}

	UInventoryCoreComponent* Inventory = CachedInventoryComponent.Get();
	if (!Inventory)
	{
		return;
	}

	int32 Total = 0;
	for (const FGuid& Guid : Inventory->GetAllInventoryGuids())
	{
		Total += Inventory->CountItemByDef(Guid, TargetItem);
	}

	if (CurrentAmount != Total)
	{
		CurrentAmount = Total;
		if (OnProgressChanged.IsBound())
		{
			OnProgressChanged.Broadcast(this);
		}
	}

	if (!bIsCompleted && CurrentAmount >= TargetAmount)
	{
		CurrentAmount = TargetAmount;
		FinishObjective();
	}
}

void UQuestObjective_Collect::HandleInventoryChanged(FGuid InventoryGuid, int32 SlotIndex)
{
	CheckInventory();
}

FString UQuestObjective_Collect::GetProgressString() const
{
	return FString::Printf(TEXT("%d / %d"), CurrentAmount, TargetAmount);
}
