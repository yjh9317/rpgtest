// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Data/RPGCameraDataAsset.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "PitchToArmLengthAndFOVModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UPitchToArmLengthAndFOVModifier : public URPGCameraModifier
{
	GENERATED_BODY()
public:
	/**
	 * Called to give modifiers a chance to adjust arm length and FOV before they are applied.
	 * 
	 * @param DeltaTime - Frame time in seconds.
	 * @param InFOV - The Current FOV of the camera.
	 * @param InArmLength - The Current Arm Length of the camera.
	 * @param ViewLocation - The view location of the camera.
	 * @param ViewRotation - The view rotation of the camera.
	 * @param OutFOV - The New FOV of the camera.
	 * @param OutArmLength - The New Arm Length of the camera.
	 * 
	 * NOTE: This function is linked to BlueprintNativeEvent: URPG_CameraModifier::ProcessBoomLengthAndFOV
	 */
	virtual void OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV, bool bWasImmediate) override;
	virtual void OnModifierEnabled_Implementation(FMinimalViewInfo const& LastPOV) override;
	virtual void OnAnyLevelSequenceStarted_Implementation() override;
	virtual void OnAnyLevelSequenceEnded_Implementation() override;
	virtual void OnSetViewTarget_Implementation(bool bImmediate, bool bNewTargetIsOwner) override;
	
	UFUNCTION(BlueprintCallable, Category="RPG|Camera Modifier")
	void ProcessBoomLengthAndFOV_Implementation(float DeltaTime, float InFOV, float InArmLength, FVector ViewLocation, FRotator ViewRotation, float& OutFOV, float& OutArmLength);
public:
	/** This makes the Arm Length and FOV change when the character is looking up/down (Used by many AAA games like all GTA games, Red Dead Redemption Assassin's Creed, Hogwarts Legacy, etc.). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Default", meta=(ExposeOnSpawn="true"))
	FCameraPitchToArmAndFOVCurveSettings CurveSettings;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	float PreEnabledFOV;

	/** Please add a variable description */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	float PreDisabledArmLength;

	bool bCanProcessViewTargetChanges = false;
};
