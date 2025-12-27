// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objectives/QuestObjective_Location.h"
// #include "Event/GlobalEventHandler.h"

void UQuestObjective_Location::ActivateObjective(URPGQuest* OwnerQuest)
{
	Super::ActivateObjective(OwnerQuest);
	// UGlobalEventHandler::Get(this)->OnPlayerEnterLocation.AddDynamic(this, &UQuestObjective_Location::OnLocationEntered);
}

void UQuestObjective_Location::DeactivateObjective()
{
	// UGlobalEventHandler::Get(this)->OnPlayerEnterLocation.RemoveDynamic(this, &UQuestObjective_Location::OnLocationEntered);
	Super::DeactivateObjective();
}

void UQuestObjective_Location::OnLocationEntered(const FGameplayTag& LocationTag)
{
	if (bIsCompleted) return;

	if (LocationTag.MatchesTag(TargetLocationTag))
	{
		FinishObjective();
	}
}