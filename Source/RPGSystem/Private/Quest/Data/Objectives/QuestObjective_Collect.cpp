// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objectives/QuestObjective_Collect.h"
#include "Quest/RPGQuest.h"
#include "Player/RPGPlayerController.h"
#include "Inventory/InventoryCoreComponent.h"

void UQuestObjective_Collect::ActivateObjective(URPGQuest* OwnerQuest)
{
	Super::ActivateObjective(OwnerQuest);
	
	// 1. 이벤트 구독 (획득/소모 모두 감지해야 함)
	// InventoryComponent->OnItemCountChanged.AddDynamic(this, &UQuestObjective_Collect::OnInventoryUpdated);

	// 2. [중요] 시작 시점의 인벤토리 확인
	CheckInventory();
}

void UQuestObjective_Collect::DeactivateObjective()
{
	Super::DeactivateObjective();
}

void UQuestObjective_Collect::CheckInventory()
{
	// 플레이어 인벤토리 접근 로직
	// APawn* Pawn = GetWorld()->GetFirstPlayerController()->GetPawn();
	// UInventoryCoreComponent* Inventory = Pawn->FindComponentByClass<UInventoryCoreComponent>();
	// if (Inventory)
	// {
	//     CurrentAmount = Inventory->GetItemCount(TargetItem);
	//     if (CurrentAmount >= TargetAmount) FinishObjective();
	// }
}

void UQuestObjective_Collect::OnInventoryUpdated(const UItemDefinition* Item, int32 NewCount)
{
	if (Item == TargetItem)
	{
		CurrentAmount = NewCount; // 혹은 누적 방식이라면 로직 변경
		if (OnProgressChanged.IsBound()) OnProgressChanged.Broadcast(this);

		if (CurrentAmount >= TargetAmount)
		{
			FinishObjective();
		}
		else
		{
			// 아이템을 버려서 개수가 줄어들면 완료 취소 처리도 가능
			bIsCompleted = false;
		}
	}
}

FString UQuestObjective_Collect::GetProgressString() const
{
	return FString::Printf(TEXT("%d / %d"), CurrentAmount, TargetAmount);
}