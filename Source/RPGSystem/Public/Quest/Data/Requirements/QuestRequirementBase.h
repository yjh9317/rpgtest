// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuestRequirementBase.generated.h"

class ARPGPlayerController;
class UQuestManagerComponent;
/**
 * 
 */
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced, DisplayName = "Quest Requirement Base")
class RPGSYSTEM_API UQuestRequirementBase : public UObject
{
	GENERATED_BODY()
public:
    // Manager: 퀘스트 상태 확인용, Player: 레벨/인벤토리 확인용
    virtual bool IsMet(const UQuestManagerComponent* Manager,const ARPGPlayerController* Player) const
	{
    	return true;
    }
    
    // 실패 시 UI에 띄울 메시지 (예: "레벨 10이 필요합니다")
    virtual FText GetFailReason() const
    {
    	return FText::FromString(TEXT("Requirements not met."));
    }
};
