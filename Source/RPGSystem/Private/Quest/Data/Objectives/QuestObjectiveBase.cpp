#include "Quest/Data/Objectives/QuestObjectiveBase.h"
#include "Quest/RPGQuest.h"

void UQuestObjectiveBase::ActivateObjective(URPGQuest* OwnerQuest)
{
	OwningQuest = OwnerQuest;
	bIsCompleted = false;
}

void UQuestObjectiveBase::DeactivateObjective()
{
	OwningQuest = nullptr;
}

void UQuestObjectiveBase::FinishObjective()
{
	if (bIsCompleted) return;

	bIsCompleted = true;
	if (OnCompleted.IsBound())
	{
		OnCompleted.Broadcast(this);
	}

	// 퀘스트 본체에 "나 끝났어"라고 알림 (선택 사항)
	if (OwningQuest)
	{
		// OwningQuest->CheckAllObjectives(); 
	}
}

void UQuestObjectiveBase::OnGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata)
{
}

bool UQuestObjectiveBase::OnQuestEvent(const FQuestEventPayload& Payload)
{
	OnGlobalEvent(nullptr, Payload.Target, Payload.Metadata);
	return bIsCompleted;
}

TArray<FGameplayTag> UQuestObjectiveBase::GetListenedEventTags() const
{
	return {};
}
