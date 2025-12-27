// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Quest/Data/QuestSpecialEventData.h"
#include "Quest/Data/Requirements/QuestRequirementBase.h"
#include "QuestRequirement_SpecialEvent.generated.h"

class UQuestManagerComponent;
class ARPGPlayerController;

UCLASS(DisplayName = "Requirement: Special Event Or Tag")
class RPGSYSTEM_API UQuestRequirement_SpecialEvent : public UQuestRequirementBase
{
	GENERATED_BODY()

public:
	// 1. 단순 이벤트 에셋 체크 (예: 튜토리얼 완료 이벤트 에셋)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	TObjectPtr<UQuestSpecialEventData> TargetEventAsset;

	// 2. (옵션) 좀 더 세밀한 태그 체크 (예: Event.Dialogue.Meeting.BadEnding)
	// 에셋을 만들지 않고 태그만으로 체크하고 싶을 때 유용
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FGameplayTag TargetEventTag;

	virtual bool IsMet(const UQuestManagerComponent* Manager, const ARPGPlayerController* Player) const override;
};