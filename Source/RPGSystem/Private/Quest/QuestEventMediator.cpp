// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestEventMediator.h"

#include "Event/GlobalEventHandler.h"
#include "Event/RPGEventBase.h"
#include "Quest/QuestEventListener.h"
#include "Quest/RPGQuest.h"
#include "Quest/Data/RPGQuestData.h"
#include "Quest/Data/Objectives/QuestObjectiveBase.h"

void UQuestEventMediator::Initialize(UGlobalEventHandler* InGlobalEventHandler)
{
	if (!ensureMsgf(InGlobalEventHandler, TEXT("QuestEventMediator::Initialize - GlobalEventHandler is null")))
	{
		return;
	}

	GlobalEventHandler = InGlobalEventHandler;
}

void UQuestEventMediator::RegisterListener(UObject* ListenerObject)
{
	if (!IsValid(ListenerObject) || !ListenerObject->Implements<UQuestEventListener>())
	{
		return;
	}

	if (!ensureMsgf(GlobalEventHandler.IsValid(), TEXT("QuestEventMediator::RegisterListener - Initialize() must be called first")))
	{
		return;
	}

	IQuestEventListener* Listener = Cast<IQuestEventListener>(ListenerObject);
	if (!Listener)
	{
		return;
	}

	const TArray<FGameplayTag> ListenedTags = Listener->GetListenedEventTags();
	for (const FGameplayTag& Tag : ListenedTags)
	{
		if (!Tag.IsValid())
		{
			continue;
		}

		TArray<TWeakObjectPtr<UObject>>& Listeners = ListenersByTag.FindOrAdd(Tag);
		const bool bAlreadyRegistered = Listeners.ContainsByPredicate([ListenerObject](const TWeakObjectPtr<UObject>& Entry)
		{
			return Entry.Get() == ListenerObject;
		});

		if (!bAlreadyRegistered)
		{
			Listeners.Add(ListenerObject);
		}

		BindTagIfNeeded(Tag);
	}
}

void UQuestEventMediator::UnregisterListener(UObject* ListenerObject)
{
	if (!IsValid(ListenerObject))
	{
		return;
	}

	for (auto It = ListenersByTag.CreateIterator(); It; ++It)
	{
		TArray<TWeakObjectPtr<UObject>>& Listeners = It.Value();
		Listeners.RemoveAll([ListenerObject](const TWeakObjectPtr<UObject>& Entry)
		{
			return !Entry.IsValid() || Entry.Get() == ListenerObject;
		});

		if (Listeners.Num() == 0)
		{
			if (GlobalEventHandler.IsValid())
			{
				GlobalEventHandler->UnbindAllGlobalEventsByGameplayTag(this, It.Key());
			}
			BoundTags.Remove(It.Key());
			It.RemoveCurrent();
		}
	}
}

void UQuestEventMediator::UnregisterAllListenersForQuest(URPGQuest* Quest)
{
	if (!IsValid(Quest))
	{
		return;
	}

	for (UQuestObjectiveBase* Objective : Quest->RuntimeObjectives)
	{
		UnregisterListener(Objective);
	}
}

void UQuestEventMediator::HandleGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata)
{
	if (ListenersByTag.Num() == 0)
	{
		return;
	}

	FGameplayTag EventTag;
	if (const URPGEventBase* EventBase = Cast<URPGEventBase>(Publisher))
	{
		EventTag = EventBase->EventTag;
	}

	if (!EventTag.IsValid())
	{
		for (const FString& Entry : Metadata)
		{
			if (Entry.StartsWith(TEXT("EventTag=")))
			{
				const FString TagString = Entry.RightChop(9);
				EventTag = FGameplayTag::RequestGameplayTag(FName(*TagString), false);
				break;
			}
		}
	}

	if (!EventTag.IsValid() && Metadata.Num() > 0)
	{
		EventTag = FGameplayTag::RequestGameplayTag(FName(*Metadata[0]), false);
	}

	if (!EventTag.IsValid())
	{
		return;
	}

	TArray<TWeakObjectPtr<UObject>>* ListenersPtr = ListenersByTag.Find(EventTag);
	if (!ListenersPtr)
	{
		return;
	}

	FQuestEventPayload QuestPayload;
	QuestPayload.Target = Payload;
	QuestPayload.EventTag = EventTag;
	QuestPayload.Metadata = Metadata;

	const TArray<TWeakObjectPtr<UObject>> ListenersCopy = *ListenersPtr;
	for (const TWeakObjectPtr<UObject>& WeakListener : ListenersCopy)
	{
		UObject* ListenerObject = WeakListener.Get();
		if (!IsValid(ListenerObject) || !ListenerObject->Implements<UQuestEventListener>())
		{
			continue;
		}

		if (IQuestEventListener* Listener = Cast<IQuestEventListener>(ListenerObject))
		{
			Listener->OnQuestEvent(QuestPayload);
		}
	}
}

void UQuestEventMediator::BindTagIfNeeded(const FGameplayTag& Tag)
{
	if (!Tag.IsValid() || BoundTags.Contains(Tag))
	{
		return;
	}

	if (!ensureMsgf(GlobalEventHandler.IsValid(), TEXT("QuestEventMediator::BindTagIfNeeded - GlobalEventHandler is invalid")))
	{
		return;
	}

	FRPGOnEventCalledSingle Delegate;
	Delegate.BindUFunction(this, FName("HandleGlobalEvent"));
	GlobalEventHandler->BindGlobalEventByGameplayTag(Tag, Delegate);
	BoundTags.Add(Tag);
}