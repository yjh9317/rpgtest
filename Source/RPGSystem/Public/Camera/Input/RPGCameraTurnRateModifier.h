// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "RPGCameraTurnRateModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGCameraTurnRateModifier : public UInputModifier
{
	GENERATED_BODY()
public:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

private:
	TObjectPtr<class ARPGPlayerCameraManager> PlayerCameraManager;
};
