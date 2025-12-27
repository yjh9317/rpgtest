// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "Camera/Data/RPGCameraDataAsset.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "YawFollowCameraModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UYawFollowCameraModifier : public URPGCameraModifier
{
	GENERATED_BODY()
public:
	/** Please add a function description */
	UFUNCTION(BlueprintPure, Category="Internal")
	bool CanTriggerFollow(FRotator LocalControlRotation);

	/**
	 * Called to give modifiers a chance to adjust view rotation updates before they are applied.
	 * 
	 * Default just returns ViewRotation unchanged
	 * @param ViewTarget - Current view target.
	 * @param InLocalControlRotation - The difference between the actor rotation and the control rotation.
	 * @param DeltaTime - Frame time in seconds.
	 * @param InViewLocation - In. The view location of the camera.
	 * @param InViewRotation - In. The view rotation of the camera.
	 * @param InDeltaRot - In/out. How much the rotation changed this frame.
	 * @param OutDeltaRot - Out. How much the control rotation should change this frame.
	 * @return Return true to prevent subsequent (lower priority) modifiers to further adjust rotation, false otherwise.
	 * 
	 * NOTE: This function is linked to BlueprintNativeEvent: URPG_CameraModifier::ProcessControlRotation
	 */
	UFUNCTION(BlueprintCallable, Category="RPG|Camera Modifier")
	bool ProcessControlRotation_Implementation(AActor* ViewTarget, float DeltaTime, FVector InViewLocation, FRotator InViewRotation, FRotator InLocalControlRotation, FRotator InDeltaRot, FRotator& OutDeltaRot);
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Internal", meta=(MultiLine="true"))
	double BlendTimer;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FCameraYawFollowSettings FollowSettings;

};
