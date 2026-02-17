// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Quest/Data/Objectives/QuestObjectiveBase.h"
#include "GameplayTagContainer.h"
#include "QuestObjective_Location.generated.h"

UCLASS(DisplayName = "Objective: Reach Location")
class RPGSYSTEM_API UQuestObjective_Location : public UQuestObjectiveBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FGameplayTag TargetLocationTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FText LocationName;

	virtual void ActivateObjective(URPGQuest* OwnerQuest) override;
	virtual void DeactivateObjective() override;
	virtual TArray<FGameplayTag> GetListenedEventTags() const override;
	
private:
	UFUNCTION()
	void OnLocationEntered(const FGameplayTag& LocationTag);

	virtual void OnGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata) override;
};
