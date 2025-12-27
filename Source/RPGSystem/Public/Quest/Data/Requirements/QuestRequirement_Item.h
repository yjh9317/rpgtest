#pragma once

#include "CoreMinimal.h"
#include "Quest/Data/Requirements/QuestRequirementBase.h"
#include "QuestRequirement_Item.generated.h"

class UItemDefinition;

UCLASS(DisplayName = "Requirement: Item")
class RPGSYSTEM_API UQuestRequirement_Item : public UQuestRequirementBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	TObjectPtr<UItemDefinition> RequiredItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	virtual bool IsMet(const UQuestManagerComponent* Manager, const ARPGPlayerController* Player) const override;
	virtual FText GetFailReason() const override;
};