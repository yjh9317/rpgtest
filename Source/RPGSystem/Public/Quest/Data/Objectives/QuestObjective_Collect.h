// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quest/Data/Objectives/QuestObjectiveBase.h"
#include "QuestObjective_Collect.generated.h"

class UItemDefinition;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UQuestObjective_Collect : public UQuestObjectiveBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	TObjectPtr<UItemDefinition> TargetItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (ClampMin = "1"))
	int32 TargetAmount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	int32 CurrentAmount = 0;

	virtual void ActivateObjective(URPGQuest* OwnerQuest) override;
	virtual void DeactivateObjective() override;
	virtual FString GetProgressString() const override;

private:
	void CheckInventory();

	UFUNCTION()
	void OnInventoryUpdated(const UItemDefinition* Item, int32 NewCount);
};
