// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Quest/Data/Objectives/QuestObjectiveBase.h"
#include "QuestObjective_Kill.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UQuestObjective_Kill : public UQuestObjectiveBase
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
	FGameplayTag TargetEnemyTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta = (ClampMin = "1"))
	int32 TargetAmount = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Runtime")
	int32 CurrentAmount = 0;

	virtual void ActivateObjective(URPGQuest* OwnerQuest) override;
	virtual void DeactivateObjective() override;
	virtual FString GetProgressString() const override;
	virtual TArray<FGameplayTag> GetListenedEventTags() const override;

private:
	virtual void OnGlobalEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata) override;
};
