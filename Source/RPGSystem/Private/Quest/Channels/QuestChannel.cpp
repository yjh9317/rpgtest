#include "Quest/Channels/QuestChannel.h"
#include "Quest/RPGQuest.h"
#include "Quest/Data/QuestSounds.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void UQuestChannel::Initialize(APlayerController* PC)
{
	OwnerPC = PC;

	// 위젯 생성 (아직 뷰포트에는 안 띄움)
	if (OwnerPC && QuestWidgetClass)
	{
		QuestMainWidget = CreateWidget<UUserWidget>(OwnerPC, QuestWidgetClass);
	}
}

void UQuestChannel::ToggleQuestJournal()
{
	if (!QuestMainWidget) return;

	if (QuestMainWidget->IsInViewport())
	{
		QuestMainWidget->RemoveFromParent();
		// 입력 모드 게임으로 복귀 등...
	}
	else
	{
		QuestMainWidget->AddToViewport();
		// 입력 모드 UI로 전환 등...
	}
}

void UQuestChannel::OnQuestStateChanged(URPGQuest* Quest, EQuestState NewState)
{
	if (!Quest) return;

	// 1. 사운드 재생 (기존 AudioChannel 기능 흡수)
	if (SoundBank)
	{
		switch (NewState)
		{
		case EQuestState::Active:
			// PlaySound(SoundBank->QuestAcceptedSound);
			break;
		case EQuestState::Valid: // 보상 수령 가능 상태 (목표 달성)
			// PlaySound(SoundBank->ObjectiveCompletedSound); 
			break;
		case EQuestState::Archive: // 완전 종료
			// PlaySound(SoundBank->QuestCompletedSound);
			break;
		case EQuestState::Failed:
			// PlaySound(SoundBank->QuestFailedSound);
			break;
		}
	}

	// 2. UI 업데이트 (위젯에 알림)
	// 예: if (QuestMainWidget) QuestMainWidget->RefreshList();
	// 예: ShowNotification(Quest->QuestData->Title, NewState);
}

void UQuestChannel::OnQuestProgressUpdated(URPGQuest* Quest)
{
	// 진행도 갱신 소리
	if (SoundBank)
	{
		// PlaySound(SoundBank->ObjectiveUpdateSound);
	}

	// UI 알림 (예: "고블린 처치 1/5")
}

void UQuestChannel::PlaySound(USoundBase* Sound)
{
	if (Sound && OwnerPC)
	{
		UGameplayStatics::PlaySound2D(OwnerPC, Sound);
	}
}