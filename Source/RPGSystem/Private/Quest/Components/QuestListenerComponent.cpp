#include "Quest/Components/QuestListenerComponent.h"
#include "Quest/Components/QuestManagerComponent.h"
#include "Quest/RPGQuest.h" // URPGQuest 헤더 포함 확인

UQuestListenerComponent::UQuestListenerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UQuestListenerComponent::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	if (GetWorld()->WorldType == EWorldType::PIE && GIsEditor)
		return;
#endif

	/* Get the local Player */
	APawn* localPlayer = GetWorld()->GetFirstPlayerController()->GetPawn();
	if (!localPlayer)
		return;

	/* Get the quest manager */
	QuestManagerComponent = localPlayer->FindComponentByClass<UQuestManagerComponent>();
	ListenToQuests();
}

void UQuestListenerComponent::ListenToQuests()
{
	if (!QuestManagerComponent) return;

	for (auto QuestID : QuestToFollow)
	{
		URPGQuest* questToFollow = QuestManagerComponent->QueryQuest(QuestID);
		if (!questToFollow) continue;
		
		// [수정 1] Delegate 이름 변경 (QuestStateChangedDelegate -> OnQuestStateChanged)
		if (!questToFollow->OnQuestStateChanged.IsAlreadyBound(this, &UQuestListenerComponent::OnQuestStateChangedWrapper))
		{
			questToFollow->OnQuestStateChanged.AddDynamic(this, &UQuestListenerComponent::OnQuestStateChangedWrapper);
		}
	}
}

void UQuestListenerComponent::OnQuestStateChangedWrapper(URPGQuest* QuestUpdate, EQuestState QuestState)
{
	OnQuestStateChanged.Broadcast(QuestUpdate, QuestState);

	/* Unbind if the quest isn't active anymore */
	// [참고] Pending 상태에서 Unbind 하는 로직이 의도된 것인지 확인 필요. 보통은 완료/실패 시 해제.
	switch (QuestState)
	{
	case EQuestState::Pending:
	case EQuestState::Failed:
	case EQuestState::Archive:
		{
			// [수정 2] Delegate 이름 변경
			if (QuestUpdate)
			{
				QuestUpdate->OnQuestStateChanged.RemoveDynamic(this, &UQuestListenerComponent::OnQuestStateChangedWrapper);
			}
			break;
		}

	default:
		break;
	}
}

void UQuestListenerComponent::BindFunctionsToQuestDelegates(URPGQuest* Quest)
{
	// [수정 3] bIsRequirementMet 변수 삭제 (더 이상 URPGQuest에 존재하지 않음)
	// 필요하다면 QuestManagerComponent->CanAcceptQuest(...) 등을 사용해야 하지만,
	// 이미 생성된 퀘스트(Quest 객체)라면 보통 수락 조건을 따질 필요 없이 리스닝하면 됩니다.
	
	if (!QuestManagerComponent || Quest == nullptr)
		return;

	// [수정 4] Delegate 이름 변경
	if (!Quest->OnQuestStateChanged.IsAlreadyBound(this, &UQuestListenerComponent::OnQuestStateChangedWrapper))
	{
		Quest->OnQuestStateChanged.AddDynamic(this, &UQuestListenerComponent::OnQuestStateChangedWrapper);
	}
}