// RPGSystem/Public/Quest/Data/QuestSpecialEventData.h

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "QuestSpecialEventData.generated.h"

UENUM(BlueprintType)
enum class ESpecialEventType : uint8
{
	Cutscene    UMETA(DisplayName = "Cutscene"),
	Dialogue    UMETA(DisplayName = "Dialogue"),
	Scripted    UMETA(DisplayName = "Scripted Event"),
	Custom      UMETA(DisplayName = "Custom")
};

/**
 * 게임 내 중요 사건(컷신 완료, 대화 종료, 특정 구역 발견 등)을 정의하는 데이터 에셋
 */
UCLASS(BlueprintType, Blueprintable)
class RPGSYSTEM_API UQuestSpecialEventData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 이벤트 이름 (UI 표시용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Config")
	FText EventName;

	// 이벤트 타입 (분류용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Config")
	ESpecialEventType EventType = ESpecialEventType::Custom;

	// [핵심] 시스템 내부 식별용 태그 (예: Event.Dialogue.Tutorial.Finish)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Event Config")
	FGameplayTag EventTag;

	// (옵션) 이 이벤트가 발생했을 때 함께 저장할 추가 정보가 필요하다면?
	// 보통은 필요 없습니다. "이벤트가 발생했다/안했다"만 체크하면 되기 때문입니다.
};