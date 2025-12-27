// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "CustomSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UCustomSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:

	/** The actual name of the custom save file. If empty it will use the class name. */
	UPROPERTY(EditDefaultsOnly, Category = "Files", meta = (DisplayName = "File Name"))
	FString SaveGameName;
	
	/** If you want to use the current save slot for the custom save file. */
	UPROPERTY(EditDefaultsOnly, Category = "Files")
	bool bUseSaveSlot = false;

public:

	//Temporary storage, used to access custom save objects from other slots than the current one
	FString SlotName;
};
