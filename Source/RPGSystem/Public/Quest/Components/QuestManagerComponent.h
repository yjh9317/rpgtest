// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "QuestManagerComponent.generated.h"

class URPGQuestData;
class URPGQuest;
class UQuestEventMediator;
class UQuestSpecialEventData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestAccepted, URPGQuest*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, URPGQuest*, Quest);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, URPGQuest*, Quest);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UQuestManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UQuestManagerComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void LateBeginPlay();
	void RegisterQuestObjectives(URPGQuest* Quest);
	void UnregisterQuestObjectives(URPGQuest* Quest);
	void InitializeSpecialEventTracking();

	UFUNCTION()
	void HandleTrackedSpecialEvent(UObject* Publisher, UObject* Payload, const TArray<FString>& Metadata);


public:
	UFUNCTION(BlueprintCallable, Category = "Quest | Quest")
	URPGQuest* QueryQuest(int QuestID);
	
	/* Delegates */
	// UPROPERTY(BlueprintCallable, Category = "Quest | Date")
	// FOnNewDayDelegate OnNewDayDelegate;
	//
	// UPROPERTY(BlueprintCallable, Category = "Quest | Date")
	// FOnNewWeekDelegate OnNewWeekDelegate;
	
	bool IsQuestCompleted(int32 QuestID) const;
	bool IsQuestActive(int32 QuestID) const;
	UFUNCTION(BlueprintCallable, Category = "Quest | Quest")
	bool CanAcceptQuest(const URPGQuestData* QuestData, class ARPGPlayerController* Player);

	UFUNCTION(BlueprintCallable, Category = "Quest | Quest")
	bool AcceptQuestByID(int32 QuestID, class ARPGPlayerController* Player);

	UFUNCTION(BlueprintCallable, Category = "Quest | Quest")
	bool CompleteQuestByID(int32 QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest | Quest")
	bool FailQuestByID(int32 QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest | Query")
	void GetAllQuests(TArray<URPGQuest*>& OutQuests) const;

	UFUNCTION(BlueprintCallable, Category = "Quest | Events")
	void RecordQuestEventTag(FGameplayTag EventTag);

	UFUNCTION(BlueprintCallable, Category = "Quest | Events")
	void RecordQuestSpecialEvent(const UQuestSpecialEventData* EventData);

	UFUNCTION(BlueprintPure, Category = "Quest | Events")
	bool HasEventTagOccurred(FGameplayTag EventTag) const;

	UFUNCTION(BlueprintPure, Category = "Quest | Events")
	bool HasSpecialEventOccurred(const UQuestSpecialEventData* EventData) const;
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Quest | Quest")
	TMap<int, URPGQuest*> QuestDataCenter;

	UPROPERTY(Transient)
	TObjectPtr<UQuestEventMediator> QuestEventMediator;

	UPROPERTY(Transient)
	TSet<FGameplayTag> TrackedSpecialEventTags;

	UPROPERTY(Transient)
	TSet<FGameplayTag> TriggeredEventTags;

	UPROPERTY(Transient)
	TSet<FName> TriggeredSpecialEventAssets;

public:
	UPROPERTY(BlueprintAssignable, Category = "Quest | Events")
	FOnQuestAccepted OnQuestAccepted;

	UPROPERTY(BlueprintAssignable, Category = "Quest | Events")
	FOnQuestCompleted OnQuestCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Quest | Events")
	FOnQuestFailed OnQuestFailed;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest | Save-Load")
	void LoadQuestData();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest | Save-Load")
	void SaveQuestData();

	UFUNCTION(BlueprintCallable, Category = "Quest | Query")
	const URPGQuestData* GetQuestDataByID(int32 QuestID) const;		
};
