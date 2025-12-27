// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputModifiers.h"
#include "RPGInputAccelerationModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGInputAccelerationModifier : public UInputModifier
{
	GENERATED_BODY()
public:
	virtual FInputActionValue ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime) override;

public:
	// The time it takes to reach full speed.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Settings, Config)
	float AccelerationTime = 1.f;

	// Apply an acceleration curve to your input so that it accelerates as time goes by. The given curve should be time-normalized, i.e., between 0 and 1.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Settings, meta = (DisplayThumbnail = "false"))
	TObjectPtr<UCurveFloat> AccelerationCurve;

private:
	float Timer = 0.f;
};
