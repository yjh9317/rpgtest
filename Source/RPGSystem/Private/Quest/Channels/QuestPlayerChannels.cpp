#include "Quest/Channels/QuestPlayerChannels.h"

#include "Player/RPGPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Quest/Channels/QuestChannel.h"
#include "Quest/Components/QuestManagerComponent.h"
#include "Quest/RPGQuest.h"

UQuestPlayerChannels::UQuestPlayerChannels()
{
	PrimaryComponentTick.bCanEverTick = false;

	QuestChannel = CreateDefaultSubobject<UQuestChannel>(TEXT("QuestChannel"));
}

void UQuestPlayerChannels::BeginPlay()
{
	Super::BeginPlay();

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
			if (ARPGPlayerController* PawnPC = Cast<ARPGPlayerController>(Pawn->GetController()))
			{
				QuestChannel->Initialize(PawnPC);
			}
		}
	}

	QuestManager = GetOwner()->FindComponentByClass<UQuestManagerComponent>();
	if (QuestManager)
	{
		QuestManager->OnQuestAccepted.AddDynamic(this, &UQuestPlayerChannels::HandleQuestStarted);

		TArray<URPGQuest*> AllQuests;
		QuestManager->GetAllQuests(AllQuests);
		for (URPGQuest* Quest : AllQuests)
		{
			if (!Quest)
			{
				continue;
			}

			if (Quest->QuestState == EQuestState::Active || Quest->QuestState == EQuestState::Valid)
			{
				HandleQuestStarted(Quest);
			}
		}
	}
}

void UQuestPlayerChannels::HandleQuestStarted(URPGQuest* NewQuest)
{
	if (!NewQuest || !QuestChannel)
	{
		return;
	}

	NewQuest->OnQuestStateChanged.AddUniqueDynamic(QuestChannel, &UQuestChannel::OnQuestStateChanged);
	NewQuest->OnQuestProgressUpdated.AddUniqueDynamic(QuestChannel, &UQuestChannel::OnQuestProgressUpdated);

	if (NewQuest->QuestState == EQuestState::Active)
	{
		QuestChannel->OnQuestStateChanged(NewQuest, EQuestState::Active);
	}
}
