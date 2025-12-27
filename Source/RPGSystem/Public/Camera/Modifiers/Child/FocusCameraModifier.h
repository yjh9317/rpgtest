// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/RPGCameraDataAsset.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "FocusCameraModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UFocusCameraModifier : public URPGCameraModifier
{
	GENERATED_BODY()
	public:
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

	/** Please add a function description */
	UFUNCTION(BlueprintPure)
	bool CanFocusTarget(FVector TargetLocation) const;

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void UpdateLOSTimerAndEvent(double DeltaTime, AController* PlayerController, AActor* FocusTarget);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void UpdateIgnoreLookInput(APlayerController* PlayerController);

	/** Please add a function description */
	UFUNCTION(BlueprintCallable)
	void SanitizeLocationIfTooClose(FVector TargetLocation, FVector InViewPointLocation, FVector InCameraLocation, FRotator InViewRotation, bool& bTooClose, FVector& OutLocation);

	/** Please add a function description */
	UFUNCTION(BlueprintPure)
	bool LOSBlockedAboveThreshold() const;
public:
	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	FRPGCameraFocusSettings FocusSettings;
private:
	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, Category="Internal")
	FRotator TargetRotation;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, Category="Internal")
	bool bValidConditions;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, Category="Internal")
	bool bWasIgnoringLookInput;

	/** Please add a variable description */
	UPROPERTY(EditDefaultsOnly, Category="Internal")
	double LOSBlockedTimer;
public:
	/** Event broadcast whern the line of sight has been of the focus camera modifier has been blocked for a duration above the threshold. */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBlockedFocusLineOfSight);
	UPROPERTY(BlueprintAssignable, EditDefaultsOnly, Category="Default")
	FOnBlockedFocusLineOfSight OnBlockedFocusLineOfSight;

};
