
#pragma once

#include "SaveSystem/Data/RPGSaveTypes.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RPGAsyncSaveGame.generated.h"

class URPGSaveSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncSaveOutputPin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncSaveFailedPin);

UCLASS()
class RPGSYSTEM_API URPGAsyncSaveGame : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	URPGAsyncSaveGame();

public:

	/*Executes when at least one save operation(Level or Player) completes.*/
	UPROPERTY(BlueprintAssignable)
	FAsyncSaveOutputPin OnCompleted;

	/*Executes when all save operations have failed.*/
	UPROPERTY(BlueprintAssignable)
	FAsyncSaveFailedPin OnFailed;

	ESaveGameMode Mode;

private:

	UPROPERTY()
	TObjectPtr<URPGSaveSubsystem> RPGSaveSubsystem;

	uint32 Data;

	uint8 bIsActive : 1;
	uint8 bFinishedStep : 1;
	uint8 bHasFailed : 1;
	uint8 bAutoSaveLevel : 1;
	uint8 bMemoryOnly : 1;

public:

	/**
	* Main function for Saving the Game. Use the Data checkboxes to define what you want to save. 
	*
	* @param Data - Check here what data you want to save. 
	*/
	UFUNCTION(BlueprintCallable, Category = "RPG Save | Actors", meta = (DisplayName = "Save Game Actors", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static URPGAsyncSaveGame* AsyncSaveActors(UObject* WorldContextObject, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/RPGSystem.ESaveTypeFlags")) int32 Data);

	UFUNCTION()
	static void AutoSaveLevelActors(URPGSaveSubsystem* _RPGSaveSubsystem);

	virtual void Activate() override;
	inline bool IsActive() const { return bIsActive; }

private:

	void StartSaving();

	void SavePlayer();
	void InternalSavePlayer();

	void SaveLevel();
	void InternalSaveLevel();

	void FinishSaving();
	void CompleteSavingTask();

	void TryMoveToNextStep(ENextStepType Step);

	bool CheckLevelStreaming();
};

