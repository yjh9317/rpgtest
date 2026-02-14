// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "SaveSystem/Data/RPGSaveTypes.h"
#include "RPGAsyncCheck.generated.h"
/**
 * 
 */

class URPGSaveSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCheckCompletedPin);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCheckFailedPin);

UCLASS()
class RPGSYSTEM_API URPGAsyncCheck : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	URPGAsyncCheck();
	
	UPROPERTY(BlueprintAssignable)
	FCheckCompletedPin OnCompleted;

	UPROPERTY(BlueprintAssignable)
	FCheckFailedPin OnFailed;

private:

	UPROPERTY()
	TObjectPtr<URPGSaveSubsystem> RPGSaveSubsystem;

	ESaveFileCheckType Type;
	FString SaveFileName;

	uint8 bCheckSuccess : 1;
	uint8 bCheckGameVersion : 1;

public:

	/**
	* Check the integrity of available files for the current Save Slot. See log for further output information.
	* 
	* @param CheckType - The type of integrity check to perform.
	* @param CustomSaveName - Only relevant when checking for a Custom Save Game.
	* @param bComplexCheck - Loads the complete data and compares save files against 'Save Game Version' from the plugin settings. 
	*/
	UFUNCTION(BlueprintCallable, Category = "RPG Save | Files", meta = (DisplayName = "Check Save File Integrity", BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", AdvancedDisplay = "CustomSaveName, bComplexCheck"))
	static URPGAsyncCheck* CheckSaveFiles(UObject* WorldContextObject, ESaveFileCheckType CheckType, FString CustomSaveName, bool bComplexCheck);

	virtual void Activate() override;

private:

	void StartCheck();
	void CheckPlayer();
	void CheckLevel();
	void CheckCustom();
	void CheckCustomSlot();
	void CompleteCheck();
};
