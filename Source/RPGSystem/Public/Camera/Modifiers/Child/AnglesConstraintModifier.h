// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "AnglesConstraintModifier.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UAnglesConstraintModifier : public URPGCameraModifier
{
	GENERATED_BODY()
	public:
	/** Please add a function description */
	UFUNCTION(BlueprintCallable, Category="Internal")
	void ProcessSlowdownConstraintForAngle(double DeltaTime, double CurrentAngle, double AngleMinConstraint, double AngleMaxConstraint, double ConstraintTolerance, double AngleInput, bool bIsForYaw, double& NewTurnRate);

	/** Please add a function description */
	UFUNCTION(BlueprintPure, Category="Internal")
	bool CanProcessSlowdownConstraint(double CurrentAngle, double AngleMinConstraint, double AngleMaxConstraint, double ConstraintThreshold, double AngleInput, bool bIsForYaw, double& TargetConstraint) const;

	/** Please add a function description */
	UFUNCTION(BlueprintPure, Category="Internal")
	void GetTargetAngleSlowdownConstraint(double CurrentAngle, double AngleMinConstraint, double AngleMaxConstraint, double ConstraintThreshold, double AngleInput, bool bIsForYaw, double& TargetConstraint) const;

	/**
	 * Called to give modifiers a chance to adjust both the yaw turn rate and pitch turn rate. However the input for looking needs to have RPG_CameraTurnRateModifier.
	 * 
	 * @param DeltaTime - Frame time in seconds.
	 * @param InLocalControlRotation - The difference between the actor rotation and the control rotation.
	 * @param OutPitchTurnRate - Out. New value of the pitch turn rate (between 0 and 1).
	 * @param OutYawTurnRate - Out. New value of the yaw turn rate (between 0 and 1).
	 * @return Return true to prevent subsequent (lower priority) modifiers to further adjust rotation, false otherwise.
	 * 
	 * NOTE: This function is linked to BlueprintNativeEvent: URPG_CameraModifier::ProcessTurnRate
	 */
	UFUNCTION(BlueprintCallable, Category="RPG|Camera Modifier")
	virtual bool ProcessTurnRate_Implementation(float DeltaTime, FRotator InLocalControlRotation, float InPitchTurnRate, float InYawTurnRate, float& OutPitchTurnRate, float& OutYawTurnRate) override;
public:
	/** Curve used to slow down the input when approaching the view limits. This function has to be slowing down when approaching the zero value in order to work. */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	TObjectPtr<UCurveFloat> AlphaCurve;

	/** How close in degrees to the YawMin or YawMax should the camera start decelerating? */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	double YawConstraintTolerance;

	/** How close in degrees to the PitchMin or PitchMax should the camera start decelerating? */
	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category="Default")
	double PitchConstraintTolerance;

};
