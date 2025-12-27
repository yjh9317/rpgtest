// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "RPGPawnInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URPGPawnInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Pawn Interface used to get the movement/rotation input values.
 */
class RPGSYSTEM_API IRPGPawnInterface
{
	GENERATED_BODY()

public:
	// Get the value of the camera rotation input (usually the right thumbstick or the mouse).
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UGC|Camera Interface")
	FRotator GetRotationInput() const;

	// Get the value of the movement input (usually WASD or the left thumbstick).
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UGC|Camera Interface")
	FVector GetMovementInput() const;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URPGPawnMovementInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * This interface should only be used with player classes NOT using the default Unreal Movement Component or components inheriting from it.
 * This interface can be used to query important movement properties (velocity, falling; etc.) from your custom movement components.
 */
class RPGSYSTEM_API IRPGPawnMovementInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UGC|Movement Interface")
	FVector GetOwnerVelocity() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UGC|Movement Interface")
	bool IsOwnerFalling() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UGC|Movement Interface")
	bool IsOwnerStrafing() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "UGC|Movement Interface")
	bool IsOwnerMovingOnGround() const;
};
