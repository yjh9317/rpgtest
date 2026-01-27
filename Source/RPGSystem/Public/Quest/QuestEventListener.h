// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "Quest/Data/QuestEventData.h"
#include "QuestEventListener.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UQuestEventListener : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IQuestEventListener
{
	GENERATED_BODY()

public:
	virtual bool OnQuestEvent(const FQuestEventPayload& Payload) = 0;
	virtual TArray<FGameplayTag> GetListenedEventTags() const = 0;
};
