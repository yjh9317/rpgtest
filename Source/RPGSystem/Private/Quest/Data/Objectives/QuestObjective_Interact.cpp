// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/Data/Objectives/QuestObjective_Interact.h"

#include "RPGSystemGameplayTags.h"

void UQuestObjective_Interact::ActivateObjective(URPGQuest* OwnerQuest)
{
	Super::ActivateObjective(OwnerQuest);
	CurrentCount = 0;
}

void UQuestObjective_Interact::DeactivateObjective()
{
	Super::DeactivateObjective();
}

void UQuestObjective_Interact::OnInteracted(const FGameplayTag& InteractTag)
{
	if (bIsCompleted)
	{
		return;
	}

	if (TargetInteractableTag.IsValid() && !InteractTag.IsValid())
	{
		return;
	}

	if (!TargetInteractableTag.IsValid() || InteractTag.MatchesTag(TargetInteractableTag))
	{
		CurrentCount++;
		if (OnProgressChanged.IsBound())
		{
			OnProgressChanged.Broadcast(this);
		}

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

TArray<FGameplayTag> UQuestObjective_Interact::GetListenedEventTags() const
{
	TArray<FGameplayTag> Tags;
	const FGameplayTag InteractionSuccessTag = RPGGameplayTags::Event_Interaction_Success.GetTag();
	if (InteractionSuccessTag.IsValid())
	{
		Tags.Add(InteractionSuccessTag);
	}
	return Tags;
}

void UQuestObjective_Interact::OnGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata)
{
	Super::OnGlobalEvent(Publisher, Payload, Metadata);

	FGameplayTag InteractTag;
	for (const FString& Entry : Metadata)
	{
		if (Entry.StartsWith(TEXT("InteractTag=")))
		{
			const FString TagString = Entry.RightChop(12);
			InteractTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
			break;
		}
	}

	OnInteracted(InteractTag);
}
