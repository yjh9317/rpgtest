#pragma once

#include "CoreMinimal.h"
#include "Quest/Data/Requirements/QuestRequirementBase.h"
#include "QuestRequirement_QuestState.generated.h"

enum class EQuestState : uint8; 

UCLASS(DisplayName = "Requirement: Quest State")
class RPGSYSTEM_API UQuestRequirement_QuestState : public UQuestRequirementBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	int32 TargetQuestID;

	// 요구되는 상태 (예: Completed)
	// UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	// EQuestState RequiredState; 
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	bool bMustBeCompleted = true;

	virtual bool IsMet(const UQuestManagerComponent* Manager, const ARPGPlayerController* Player) const override;
	virtual FText GetFailReason() const override;
};