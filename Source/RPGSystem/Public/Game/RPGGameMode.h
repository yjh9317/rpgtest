// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RPGGameMode.generated.h"

class UCustomSaveGame;
/**
 * 
 */
UCLASS()
class RPGSYSTEM_API ARPGGameMode : public AGameMode
{
	GENERATED_BODY()


public:
	void BeginPlay() override;
	
	void TestFunc();

	UFUNCTION(BlueprintImplementableEvent)
	void LoadCustoms();
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCustomSaveGame> CustomSaveGameClass;
	
	FTimerHandle TestTimerHandle;
};
