#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestChannel.generated.h"

class UUserWidget;
class URPGQuest;
class UQuestSounds;
class APlayerController;

/**
 * 퀘스트 시스템의 "연출(Presentation)"을 담당하는 채널.
 * UI 표시, 효과음 재생, 알림 메시지 등을 처리합니다.
 */
UCLASS(BlueprintType)
class RPGSYSTEM_API UQuestChannel : public UObject
{
	GENERATED_BODY()

public:
	// --- Config ---
	// 퀘스트 메인 위젯 클래스 (예: WBP_QuestJournal)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | UI")
	TSubclassOf<UUserWidget> QuestWidgetClass;

	// 퀘스트 관련 사운드 데이터 에셋
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Audio")
	TObjectPtr<UQuestSounds> SoundBank;

	// --- Runtime ---
	UPROPERTY(BlueprintReadOnly, Category = "Quest | UI")
	TObjectPtr<UUserWidget> QuestMainWidget;

	UPROPERTY(BlueprintReadOnly, Category = "Quest | Context")
	TObjectPtr<APlayerController> OwnerPC;

	// --- Functions ---
	void Initialize(APlayerController* PC);

	// UI 열기/닫기 (저널 등)
	UFUNCTION(BlueprintCallable, Category = "Quest | UI")
	void ToggleQuestJournal();

	// --- Event Handlers (퀘스트 시스템에서 호출) ---
	
	// 퀘스트 상태 변경 시 (수락, 완료, 실패) -> UI 갱신 & 사운드
	UFUNCTION()
	void OnQuestStateChanged(URPGQuest* Quest, EQuestState NewState);

	// 퀘스트 진행도 변경 시 (목표 달성) -> UI 알림
	UFUNCTION()
	void OnQuestProgressUpdated(URPGQuest* Quest);

	// --- Audio Helpers ---
	void PlaySound(USoundBase* Sound);
};