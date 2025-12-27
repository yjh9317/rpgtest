// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RPGCameraManagerProvider.generated.h"

class ARPGPlayerCameraManager;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URPGCameraManagerProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class RPGSYSTEM_API IRPGCameraManagerProvider
{
	GENERATED_BODY()

public:
	// BlueprintCallable을 붙여 BP에서도 호출 가능하게 함
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "RPG Camera")
	ARPGPlayerCameraManager* GetRPGPlayerCameraManager();
};
