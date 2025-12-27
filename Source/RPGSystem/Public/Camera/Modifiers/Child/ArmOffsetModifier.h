// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "ArmOffsetModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UArmOffsetModifier : public URPGCameraModifier
{
	GENERATED_BODY()
public:
	/**
	 * Called to give modifiers a chance to adjust arm offsets before they are applied.
	 * 
	 * @param DeltaTime - Frame time in seconds.
	 * @param InSocketOffset - The Current Socket Offset of the camera.
	 * @param InTargetOffset - The Current Target Offset of the camera.
	 * @param ViewLocation - The view location of the camera.
	 * @param ViewRotation - The view rotation of the camera.
	 * @param OutSocketOffset - New Socket Offset of the camera.
	 * @param OutTargetOffset - New Target Offset of the camera.
	 * 
	 * NOTE: This function is linked to BlueprintNativeEvent: URPG_CameraModifier::ProcessBoomOffsets
	 */
	UFUNCTION(BlueprintCallable, Category="RPG|Camera Modifier")
	void ProcessBoomOffsets_Implementation(float DeltaTime, FVector InSocketOffset, FVector InTargetOffset, FVector ViewLocation, FRotator ViewRotation, FVector& OutSocketOffset, FVector& OutTargetOffset);
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FVector SocketOffset;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FVector TargetOffset;

};
