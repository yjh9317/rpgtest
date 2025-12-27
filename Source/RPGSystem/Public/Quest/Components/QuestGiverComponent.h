// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Quest/RPGQuest.h"
#include "Quest/Data/RPGQuestData.h"
#include "QuestGiverComponent.generated.h"

class UQuestMarkerWidget;
class UQuestManagerComponent;
class UQuestPlayerChannels;
class UWidgetComponent;

USTRUCT(BlueprintType)
struct FQuestGiverEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsQuestGiver = true; // 퀘스트를 주는가?

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsQuestReceiver = true; // 퀘스트 완료를 받는가?
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnStateChangedDelegate, EQuestState QuestState);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSleepStateChangedDelegate, bool bIsComponentSilent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UQuestGiverComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UQuestGiverComponent();
	void OnQuestInteracted(AActor* Interactor);
	//void OnQuestInteracted(const TScriptInterface<UQuest_PlayerChannels>& PlayerChannel);

protected:
	virtual void BeginPlay() override;
	void LateBeginPlay();
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Quest | Component")
	TMap<int, FQuestGiverEntry> QuestList;

	UFUNCTION(Category = "Quest | Events")
	void OnQuestStateChangedWrapper(URPGQuest* QuestUpdate, EQuestState NewState);
	
	void BindFunctionsToQuestDelegates();
	
	UPROPERTY(BlueprintReadOnly, Category = "Quest | Component")
	bool bIsComponentSilent = true;
	
	// Should the Quest Component be triggered before any other interaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest | Component")
	bool bHasPriority = true;
	
	/* Widgets */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | UI")
	TSubclassOf<UQuestMarkerWidget> QuestMarkerClass = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | UI")
	UMaterialInterface* QuestMarkerMaterial = nullptr; 
	// Put a two sided material for the widget to be two sided
	
	// You can adjust this value if the quest Marker looks too low or to high of the actor
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Quest | UI")
	float zOffset = 75.f;
	
	UFUNCTION(BlueprintCallable, Category = "Quest | UI")
	void SetQuestMarker(bool isMarkerVisible, bool isQuestValid, EQuestType QuestState = EQuestType::MainQuest);
	
	UFUNCTION(BlueprintCallable, Category = "Quest | UI")
	void UpdateQuestMarker();
	
	void CheckQuestTypes(TArray<URPGQuest*>& ValidQuests, EQuestType& currentType);

	void ShowVisualMarker();
	void RemoveVisualMarker();
protected:
	UPROPERTY()
	AActor* Owner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Quest | Component")
	UQuestManagerComponent* QuestManagerComponent = nullptr;

	/* Floating movement */
	float CurrentDelta = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Floating Movement")
	float FloatingSpeed = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest | Floating Movement")
	float FloatingAmplitude = 0.05f;

	
	FOnStateChangedDelegate OnQuestStateChanged;
	FOnSleepStateChangedDelegate OnSleepStateChanged;
private:
	UPROPERTY()
	UWidgetComponent* QuestMarkerWidget = nullptr;

	/* References */
	UPROPERTY()
	TObjectPtr<APlayerCameraManager> CameraManager = nullptr;

	UPROPERTY()
	TObjectPtr<UWidgetComponent> VisualWidget;
	
	UPROPERTY()
	TObjectPtr<UQuestMarkerWidget> VisualMarkWidget;
private:
	void CreateQuestMarkerWidget();
	void MarkerFloatingMovement(float DeltaTime);
	// UQuestPlayerChannels* GetPlayerChannelsFromActor(AActor* Actor) const;
	// UQuestManagerComponent* GetQuestManagerFromActor(AActor* Actor) const;
	// void ExecuteQuestGiverLogic(AActor* Interactor, UQuestPlayerChannels* PlayerChannels);
	/* DEBUG */
	UFUNCTION(BlueprintCallable, Category = "Quest | Debug")
	void RerunScript(); // Usefull only to see all the quest Marker when using the Tool
		
};
