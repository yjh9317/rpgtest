#include "Quest/Channels/QuestChannel.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Quest/Data/QuestSounds.h"
#include "Quest/RPGQuest.h"

void UQuestChannel::Initialize(APlayerController* PC)
{
	OwnerPC = PC;

	if (OwnerPC && QuestWidgetClass)
	{
		QuestMainWidget = CreateWidget<UUserWidget>(OwnerPC, QuestWidgetClass);
	}
}

void UQuestChannel::ToggleQuestJournal()
{
	if (!QuestMainWidget)
	{
		return;
	}

	if (QuestMainWidget->IsInViewport())
	{
		QuestMainWidget->RemoveFromParent();
	}
	else
	{
		QuestMainWidget->AddToViewport();
	}
}

void UQuestChannel::OnQuestStateChanged(URPGQuest* Quest, EQuestState NewState)
{
	if (!Quest)
	{
		return;
	}

	if (SoundBank)
	{
		switch (NewState)
		{
		case EQuestState::Active:
			PlaySound(SoundBank->QuestStart);
			break;
		case EQuestState::Valid:
			PlaySound(SoundBank->QuestValid);
			break;
		case EQuestState::Archive:
			PlaySound(SoundBank->QuestEnd);
			break;
		case EQuestState::Failed:
			PlaySound(SoundBank->QuestFailed);
			break;
		default:
			break;
		}
	}

	// UI refresh hook can be added here.
}

void UQuestChannel::OnQuestProgressUpdated(URPGQuest* Quest)
{
	if (SoundBank)
	{
		PlaySound(SoundBank->ObjectiveUpdate);
	}

	// UI progress notification hook can be added here.
}

void UQuestChannel::PlaySound(USoundBase* Sound)
{
	if (Sound && OwnerPC)
	{
		UGameplayStatics::PlaySound2D(OwnerPC, Sound);
	}
}
