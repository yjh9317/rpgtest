#include "Quest/Data/Requirements/QuestRequirement_QuestState.h"
#include "Quest/Components/QuestManagerComponent.h"

bool UQuestRequirement_QuestState::IsMet(const UQuestManagerComponent* Manager, const ARPGPlayerController* Player) const
{
	if (!IsValid(Manager)) return false;

	// Manager에 GetQuestState(int32 QuestID) 같은 함수가 있다고 가정
	// EQuestState CurrentState = Manager->GetQuestState(TargetQuestID);
    
	// 예시: 완료 여부 체크
	if (bMustBeCompleted)
	{
		return Manager->IsQuestCompleted(TargetQuestID);
	}

	return true;
}

FText UQuestRequirement_QuestState::GetFailReason() const
{
	return NSLOCTEXT("Quest", "FailPreQuest", "선행 퀘스트를 완료해야 합니다.");
}