// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objectives/QuestObjective_Interact.h"
// #include "Event/GlobalEventHandler.h"

void UQuestObjective_Interact::ActivateObjective(URPGQuest* OwnerQuest)
{
	Super::ActivateObjective(OwnerQuest);
	// UGlobalEventHandler::Get(this)->OnInteractionSuccess.AddDynamic(this, &UQuestObjective_Interact::OnInteracted);
}

void UQuestObjective_Interact::DeactivateObjective()
{
	// UGlobalEventHandler::Get(this)->OnInteractionSuccess.RemoveDynamic(this, &UQuestObjective_Interact::OnInteracted);
	Super::DeactivateObjective();
}

void UQuestObjective_Interact::OnInteracted(const FGameplayTag& InteractTag)
{
	if (bIsCompleted) return;

	if (InteractTag.MatchesTag(TargetInteractableTag))
	{
		CurrentCount++;
		if (OnProgressChanged.IsBound()) OnProgressChanged.Broadcast(this);

		if (CurrentCount >= TargetCount)
		{
			FinishObjective();
		}
	}
}

FString UQuestObjective_Interact::GetProgressString() const
{
	return FString::Printf(TEXT("%d / %d"), CurrentCount, TargetCount);
}