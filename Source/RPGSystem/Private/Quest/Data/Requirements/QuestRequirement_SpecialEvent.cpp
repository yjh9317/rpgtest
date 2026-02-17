// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Requirements/QuestRequirement_SpecialEvent.h"

#include "Quest/Components/QuestManagerComponent.h"

bool UQuestRequirement_SpecialEvent::IsMet(const UQuestManagerComponent* Manager,
	const ARPGPlayerController* Player) const
{
	if (!Manager)
	{
		return false;
	}

	if (TargetEventAsset && Manager->HasSpecialEventOccurred(TargetEventAsset))
	{
		return true;
	}

	if (TargetEventAsset && TargetEventAsset->EventTag.IsValid() && Manager->HasEventTagOccurred(TargetEventAsset->EventTag))
	{
		return true;
	}

	if (TargetEventTag.IsValid() && Manager->HasEventTagOccurred(TargetEventTag))
	{
		return true;
	}

	return false;
}
