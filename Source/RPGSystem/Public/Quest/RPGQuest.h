#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RPGQuest.generated.h"

class URPGQuestData;
class UQuestObjectiveBase;

UENUM(BlueprintType)
enum class EQuestState : uint8
{
	Pending				UMETA(DisplayName = "Pending"), // 수락 전
	Active				UMETA(DisplayName = "Active"),  // 진행 중
	Valid				UMETA(DisplayName = "Valid"),   // 목표 달성 (보상 수령 대기)
	Archive				UMETA(DisplayName = "Archive"), // 완료됨 (보상 받음)
	Failed				UMETA(DisplayName = "Failed"),  // 실패
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FQuestStateChangedDelegate, URPGQuest*, Quest, EQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestProgressUpdated, URPGQuest*, Quest);

UCLASS(BlueprintType)
class RPGSYSTEM_API URPGQuest : public UObject
{
	GENERATED_BODY()

public:
	URPGQuest();

	// --- Data ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest")
	URPGQuestData* QuestData;

	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	EQuestState QuestState = EQuestState::Pending;

	// [핵심] 실제 런타임에서 돌아가는 목표 인스턴스들
	UPROPERTY(BlueprintReadOnly, Category = "Quest")
	TArray<TObjectPtr<UQuestObjectiveBase>> RuntimeObjectives;

	// --- Control Functions ---
	
	// 1. 퀘스트 생성 및 데이터 복제 (Manager에서 호출)
	void InitializeFromData(URPGQuestData* Data);

	// 2. 퀘스트 시작 (수락)
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void EnableQuest();

	// 3. 퀘스트 완료 (보상 지급 후 아카이브)
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void CompleteQuest();

	// 4. 퀘스트 포기/실패
	UFUNCTION(BlueprintCallable, Category = "Quest")
	void FailQuest();

	// --- Delegates ---
	UPROPERTY(BlueprintAssignable)
	FQuestStateChangedDelegate OnQuestStateChanged;

	UPROPERTY(BlueprintAssignable)
	FOnQuestProgressUpdated OnQuestProgressUpdated;

protected:
	// 내부적으로 목표 상태가 변했을 때 호출됨
	UFUNCTION()
	void OnObjectiveCompleted(UQuestObjectiveBase* Objective);

	UFUNCTION()
	void OnObjectiveProgress(UQuestObjectiveBase* Objective);

	void UpdateQuestState(EQuestState NewState);
	bool CheckAllObjectivesCompleted() const;
};