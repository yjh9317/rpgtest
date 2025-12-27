// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestManagerComponent.generated.h"

class URPGQuestData;
class URPGQuest;

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
	void LateBeginPlay();


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
	bool CanAcceptQuest(const URPGQuestData* QuestData, class ARPGPlayerController* Player);
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Quest | Quest")
	TMap<int, URPGQuest*> QuestDataCenter;

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest | Save-Load")
	void LoadQuestData();
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Quest | Save-Load")
	void SaveQuestData();

	UFUNCTION(BlueprintCallable, Category = "Quest | Query")
	const URPGQuestData* GetQuestDataByID(int32 QuestID) const;		
};
