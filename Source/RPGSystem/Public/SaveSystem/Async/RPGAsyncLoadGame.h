
#pragma once

#include "SaveSystem/Data/RPGSaveData.h"
#include "SaveSystem/Async/RPGActorLoader.h"
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "RPGAsyncLoadGame.generated.h"

class URPGSaveSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncLoadOutputPin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAsyncLoadFailedPin);

UCLASS()
class RPGSYSTEM_API URPGAsyncLoadGame : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

protected:
	URPGAsyncLoadGame();
	
public:

	/*Executes when at least one load operation(Level or Player) completes.*/
	UPROPERTY(BlueprintAssignable)
	FAsyncLoadOutputPin OnCompleted;

	/*Executes when all load operations have failed.*/
	UPROPERTY(BlueprintAssignable)
	FAsyncLoadFailedPin OnFailed;

	ESaveGameMode Mode;

private:

	TSharedPtr<FRPGSaveLevelLoader> Loader;

	UPROPERTY()
	TObjectPtr<URPGSaveSubsystem> RPGSaveSubsystem;

	uint32 Data;

	uint8 bIsActive : 1;
	uint8 bLoadFailed : 1;
	uint8 bFullReload : 1;
	uint8 bAutoLoadLevel : 1;

	FTimerHandle FailedTimerHandle;

public:
	
	/**
	* Main function for Loading the Game. Use the Data checkboxes to define what you want to load.
	*
	* @param Data - Check here what data you want to load.
	* @param bFullReload - If false, load only Actors that have not been loaded. Set to true if you want to reload all saved Actor Data.
	*/
	UFUNCTION(BlueprintCallable, Category = "RPG Save | Actors", meta = (DisplayName = "Load Game Actors", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"))
	static URPGAsyncLoadGame* AsyncLoadActors(UObject* WorldContextObject, UPARAM(meta = (Bitmask, BitmaskEnum = "/Script/RPGSystem.ELoadTypeFlags")) int32 Data, bool bFullReload);

	UFUNCTION()
	static void AutoLoadLevelActors(URPGSaveSubsystem* _RPGSaveSubsystem);

	virtual void Activate() override;
	inline bool IsActive() const { return bIsActive; }

	void LoadLevelActors(const FLoaderInitData& LoaderData);

	void FinishLoading();
	void ForceDestroy();

private:

	void PreLoading();
	void StartLoading();

	void PreparePlayer();
	void LoadPlayer();

	void PrepareLevel();
	void LoadGameMode();
	void LoadLevelScripts(); 
	void PrepareLevelActors();
	void StartLoadLevelActors();

	void CompleteLoadingTask();
	void FailLoadingTask();

	void ClearFailTimer();

	inline void SetLoadNotFailed()
	{
		ClearFailTimer();
		bLoadFailed = false;
	}
};
