#include "Quest/Channels/QuestPlayerChannels.h"
#include "Quest/Channels/QuestChannel.h"
#include "Quest/Components/QuestManagerComponent.h"
#include "Quest/RPGQuest.h"
#include "Player/RPGPlayerController.h" 

UQuestPlayerChannels::UQuestPlayerChannels()
{
	PrimaryComponentTick.bCanEverTick = false; // 더 이상 Tick 불필요

	// QuestChannel 생성
	QuestChannel = CreateDefaultSubobject<UQuestChannel>(TEXT("QuestChannel"));
}

void UQuestPlayerChannels::BeginPlay()
{
	Super::BeginPlay();

	// 1. QuestChannel 설정 주입
	if (QuestChannel)
	{
		QuestChannel->QuestWidgetClass = QuestWidgetClass;
		QuestChannel->SoundBank = QuestSounds;
		
		if (ARPGPlayerController* PC = Cast<ARPGPlayerController>(GetOwner()))
		{
			QuestChannel->Initialize(PC);
		}
		else if (APawn* Pawn = Cast<APawn>(GetOwner()))
		{
			// Owner가 Pawn인 경우 Controller 찾기
			if (ARPGPlayerController* PawnPC = Cast<ARPGPlayerController>(Pawn->GetController()))
			{
				QuestChannel->Initialize(PawnPC);
			}
		}
	}

	// 2. QuestManager 찾기 및 이벤트 연결
	QuestManager = GetOwner()->FindComponentByClass<UQuestManagerComponent>();
	if (QuestManager)
	{
		// QuestManager에 "퀘스트가 수락됨" 델리게이트가 있다고 가정 (없으면 추가 필요)
		// 예: QuestManager->OnQuestAccepted.AddDynamic(this, &UQuestPlayerChannels::HandleQuestStarted);
		
		// 임시: 현재 활성화된 퀘스트들에 대해 UI 연결 (로드 게임 등 고려)
		/*
		for (auto& Elem : QuestManager->ActiveQuests)
		{
			HandleQuestStarted(Elem.Value);
		}
		*/
	}
}

void UQuestPlayerChannels::HandleQuestStarted(URPGQuest* NewQuest)
{
	if (!NewQuest || !QuestChannel) return;

	// 퀘스트의 상태 변화를 QuestChannel(연출)에 연결
	NewQuest->OnQuestStateChanged.AddUniqueDynamic(QuestChannel, &UQuestChannel::OnQuestStateChanged);
	NewQuest->OnQuestProgressUpdated.AddUniqueDynamic(QuestChannel, &UQuestChannel::OnQuestProgressUpdated);
	
	// 시작 사운드 재생을 위해 강제 호출 (이미 Active 상태라면)
	if (NewQuest->QuestState == EQuestState::Active)
	{
		QuestChannel->OnQuestStateChanged(NewQuest, EQuestState::Active);
	}
}