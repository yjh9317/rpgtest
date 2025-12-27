// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SaveSystem/Data/RPGSaveData.h"
#include "SaveSystem/Data/RPGSaveTypes.h"
#include "InfoSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UInfoSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Slots")
	FSaveSlotInfo SlotInfo;

public:

	inline void UpdateMultiLevelSave(const FName& LevelName, TArray<FName>& OutLevels)
	{
		SlotInfo.Levels.AddUnique(LevelName);
		OutLevels = SlotInfo.Levels;
	}

	inline void KeepMultiLevelSave(const TArray<FName>& InLevels)
	{
		if (!RPGSave::ArrayEmpty(InLevels) && RPGSave::ArrayEmpty(SlotInfo.Levels))
		{
			SlotInfo.Levels = InLevels;
		}
	}
};
