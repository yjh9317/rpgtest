// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/RPGCameraDataAsset.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "ArmLagModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UArmLagModifier : public URPGCameraModifier
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void UpdateLagSettings();
public:
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FCameraArmLagSettings ArmLagSettings;
};
