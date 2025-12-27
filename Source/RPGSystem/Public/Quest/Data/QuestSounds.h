
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "QuestSounds.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UQuestSounds : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	/* If you want to use one Sound per Quest Type when a quest is accepted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest")
	bool bUseAdvancedSound = false;

	// DEFAULT SOUNDS
	/* Sound triggered when a quest is accepted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest", meta = (EditCondition = "!bUseAdvancedSound", EditConditionHides))
	USoundBase* QuestStart;

	/* Sound triggered when all the objective of a quest are completed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest")
	USoundBase* QuestValid;

	/* Sound triggered when a quest is ended */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest", meta = (EditCondition = "!bUseAdvancedSound", EditConditionHides))
	USoundBase* QuestEnd;

	/* Sound triggered when an objective of a quest is failed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest")
	USoundBase* QuestFailed;

	/* Sound triggered when a quest is abandonned */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest")
	USoundBase* AbandonQuest;

	/* Sound triggered when an objective of a quest is updated */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest")
	USoundBase* ObjectiveUpdate;



	// ADVANCED SOUNDS
	/* Sound triggered when a Main Quest is accepted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest|Quest Start", meta = (EditCondition = "bUseAdvancedSound", EditConditionHides))
	USoundBase* MainQuestStart;

	/* Sound triggered when a Side Quest is accepted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest|Quest Start", meta = (EditCondition = "bUseAdvancedSound", EditConditionHides))
	USoundBase* SideQuestStart;

	/* Sound triggered when a Daily Quest is accepted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest|Quest Start", meta = (EditCondition = "bUseAdvancedSound", EditConditionHides))
	USoundBase* DailyQuestStart;

	/* Sound triggered when a Weekly Quest is accepted */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest|Quest Start", meta = (EditCondition = "bUseAdvancedSound", EditConditionHides))
	USoundBase* WeeklyQuestStart;

	/* Sound triggered when a Main Quest is ended */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest|Quest End", meta = (EditCondition = "bUseAdvancedSound", EditConditionHides))
	USoundBase* MainQuestEnd;

	/* Sound triggered when a Side Quest is ended */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest|Quest End", meta = (EditCondition = "bUseAdvancedSound", EditConditionHides))
	USoundBase* SideQuestEnd;

	/* Sound triggered when a Daily Quest is ended */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest|Quest End", meta = (EditCondition = "bUseAdvancedSound", EditConditionHides))
	USoundBase* DailyQuestEnd;

	/* Sound triggered when a Weekly Quest is ended */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Advanced Quest|Quest End", meta = (EditCondition = "bUseAdvancedSound", EditConditionHides))
	USoundBase* WeeklyQuestEnd;
};