// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/RPGQuest.h"
#include "QuestListenerComponent.generated.h"

class UQuestManagerComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChangedListenerDelegate, URPGQuest*, questUpdate, EQuestState,
                                             QuestState);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UQuestListenerComponent : public UActorComponent
{
	GENERATED_BODY()


public:
	UQuestListenerComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Quest")
	TArray<int> QuestToFollow;

protected:
	virtual void BeginPlay() override;

	void ListenToQuests();

	/* References */
	UPROPERTY(BlueprintReadOnly, Category = "Quest | Quest")
	UQuestManagerComponent* QuestManagerComponent = nullptr;

	UFUNCTION(Category = "Quest | Events")
	void OnQuestStateChangedWrapper(URPGQuest* questUpdate, EQuestState QuestState);

	// Use this delegate to be able to update actors in blueprint
	// when OnQuestStateChanged is triggered
	UPROPERTY(BlueprintAssignable, Category = "Quest | Events")
	FOnQuestStateChangedListenerDelegate OnQuestStateChanged;

	UFUNCTION(BlueprintCallable, Category = "Quest | Events")
	void BindFunctionsToQuestDelegates(URPGQuest* quest);

		
};
