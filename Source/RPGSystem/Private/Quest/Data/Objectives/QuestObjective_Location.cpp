// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objectives/QuestObjective_Location.h"

#include "RPGSystemGameplayTags.h"

void UQuestObjective_Location::ActivateObjective(URPGQuest* OwnerQuest)
{
	Super::ActivateObjective(OwnerQuest);
}

void UQuestObjective_Location::DeactivateObjective()
{
	Super::DeactivateObjective();
}

void UQuestObjective_Location::OnLocationEntered(const FGameplayTag& LocationTag)
{
	if (bIsCompleted)
	{
		return;
	}

	if (!TargetLocationTag.IsValid() || LocationTag.MatchesTag(TargetLocationTag))
	{
		FinishObjective();
	}
}

TArray<FGameplayTag> UQuestObjective_Location::GetListenedEventTags() const
{
	TArray<FGameplayTag> Tags;
	const FGameplayTag LocationEnteredTag = RPGGameplayTags::Event_World_LocationEntered.GetTag();
	if (LocationEnteredTag.IsValid())
	{
		Tags.Add(LocationEnteredTag);
	}
	return Tags;
}

void UQuestObjective_Location::OnGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata)
{
	Super::OnGlobalEvent(Publisher, Payload, Metadata);

	FGameplayTag LocationTag;
	for (const FString& Entry : Metadata)
	{
		if (Entry.StartsWith(TEXT("LocationTag=")))
		{
			const FString TagString = Entry.RightChop(12);
			LocationTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
			break;
		}
	}

	OnLocationEntered(LocationTag);
}
