// Fill out your copyright notice in the Description page of Project Settings.

#include "Quest/Data/Objectives/QuestObjective_Kill.h"

#include "GameplayTagAssetInterface.h"
#include "RPGSystemGameplayTags.h"

namespace
{
	int32 ExtractAmountFromMetadata(const TArray<FString>& Metadata)
	{
		for (const FString& Entry : Metadata)
		{
			if (Entry.StartsWith(TEXT("Amount=")))
			{
				return FMath::Max(1, FCString::Atoi(*Entry.RightChop(7)));
			}
		}
		return 1;
	}

	bool MatchesTargetTag(const AActor* Actor, const FGameplayTag& TargetTag)
	{
		if (!Actor)
		{
			return false;
		}

		if (!TargetTag.IsValid())
		{
			return true;
		}

		if (const IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Actor))
		{
			FGameplayTagContainer OwnedTags;
			TagInterface->GetOwnedGameplayTags(OwnedTags);
			if (OwnedTags.HasTag(TargetTag))
			{
				return true;
			}
		}

		return Actor->ActorHasTag(TargetTag.GetTagName());
	}
}

void UQuestObjective_Kill::ActivateObjective(URPGQuest* OwnerQuest)
{
	Super::ActivateObjective(OwnerQuest);
	CurrentAmount = 0;
}

void UQuestObjective_Kill::DeactivateObjective()
{
	Super::DeactivateObjective();
}

FString UQuestObjective_Kill::GetProgressString() const
{
	return FString::Printf(TEXT("%d / %d"), CurrentAmount, TargetAmount);
}

TArray<FGameplayTag> UQuestObjective_Kill::GetListenedEventTags() const
{
	TArray<FGameplayTag> Tags;
	const FGameplayTag KillEventTag = RPGGameplayTags::Event_Combat_Kill.GetTag();
	if (KillEventTag.IsValid())
	{
		Tags.Add(KillEventTag);
	}
	return Tags;
}

void UQuestObjective_Kill::OnGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata)
{
	Super::OnGlobalEvent(Publisher, Payload, Metadata);

	if (bIsCompleted)
	{
		return;
	}

	AActor* DeadActor = Cast<AActor>(Payload);
	if (!DeadActor || !MatchesTargetTag(DeadActor, TargetEnemyTag))
	{
		return;
	}

	CurrentAmount += ExtractAmountFromMetadata(Metadata);
	CurrentAmount = FMath::Min(CurrentAmount, TargetAmount);
	if (OnProgressChanged.IsBound())
	{
		OnProgressChanged.Broadcast(this);
	}

	if (CurrentAmount >= TargetAmount)
	{
		FinishObjective();
	}
}
