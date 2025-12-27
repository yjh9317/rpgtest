
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RPGQuestData.generated.h"

class UQuestRequirementBase;
class UItemDefinition;
class UQuestObjectiveBase;

UENUM()
enum class EQuestType : uint8
{
	Daily					UMETA(DisplayName = "Daily Quest"),
	Weekly					UMETA(DisplayName = "Weekly Quest"),
	MainQuest UMETA( DisplayName = "MainQuest"),
	SubQuest UMETA( DisplayName = "SubQuest")
};


UCLASS()
class RPGSYSTEM_API URPGQuestData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/* Quest properties */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	EQuestType QuestType = EQuestType::MainQuest;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	FString QuestLocation = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	FString QuestTitle = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	FString QuestDescription = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	FString QuestSummary = FString();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	int QuestID = 0;

	/* Requirements */
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Quest | Requirements")
	TArray<TObjectPtr<UQuestRequirementBase>> Requirements;

	/* Objectives */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	TArray<UQuestObjectiveBase*> Objectives;


	/** Rewards */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	int XpReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	int GoldReward = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest Overview")
	TMap<UItemDefinition*, int> ItemsReward;
};
