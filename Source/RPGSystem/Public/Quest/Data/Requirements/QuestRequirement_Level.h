#pragma once

#include "CoreMinimal.h"
#include "Quest/Data/Requirements/QuestRequirementBase.h"
#include "QuestRequirement_Level.generated.h"

UCLASS(DisplayName = "Requirement: Player Level")
class RPGSYSTEM_API UQuestRequirement_Level : public UQuestRequirementBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement", meta = (ClampMin = "1"))
	int32 RequiredLevel = 1;

	virtual bool IsMet(const UQuestManagerComponent* Manager, const ARPGPlayerController* Player) const override;
	virtual FText GetFailReason() const override;
};