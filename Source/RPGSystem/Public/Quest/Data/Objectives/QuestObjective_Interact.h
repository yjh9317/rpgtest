// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quest/Data/Objectives/QuestObjectiveBase.h"
#include "GameplayTagContainer.h"
#include "QuestObjective_Interact.generated.h"

UCLASS(DisplayName = "Objective: Interact")
class RPGSYSTEM_API UQuestObjective_Interact : public UQuestObjectiveBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FGameplayTag TargetInteractableTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	int32 TargetCount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	int32 CurrentCount = 0;

	virtual void ActivateObjective(URPGQuest* OwnerQuest) override;
	virtual void DeactivateObjective() override;
	virtual FString GetProgressString() const override;

private:
	UFUNCTION()
	void OnInteracted(const FGameplayTag& InteractTag);
};
