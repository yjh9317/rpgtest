// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/Child/AnglesConstraintModifier.h"
#include "Camera/RPGPlayerCameraManager.h"
#include "Kismet/KismetMathLibrary.h"


void UAnglesConstraintModifier::ProcessSlowdownConstraintForAngle(double DeltaTime, double CurrentAngle,
                                                                  double AngleMinConstraint, double AngleMaxConstraint, double ConstraintTolerance, double AngleInput, bool bIsForYaw,
                                                                  double& NewTurnRate)
{
	double TargetAngle = 0.f;
	if(CanProcessSlowdownConstraint(CurrentAngle,AngleMinConstraint,AngleMaxConstraint,ConstraintTolerance,AngleInput,bIsForYaw,TargetAngle))
	{
		float AngleMagnitudeDiff = FMath::Abs(TargetAngle) - FMath::Abs(CurrentAngle);
		float NormalizedError = UKismetMathLibrary::SafeDivide(AngleMagnitudeDiff,ConstraintTolerance);
		NewTurnRate = AlphaCurve->GetFloatValue(NormalizedError);
	}
	else
	{
		NewTurnRate = 1.f;
	}
}

bool UAnglesConstraintModifier::CanProcessSlowdownConstraint(double CurrentAngle, double AngleMinConstraint,
	double AngleMaxConstraint, double ConstraintThreshold, double AngleInput, bool bIsForYaw,
	double& TargetConstraint) const
{
	GetTargetAngleSlowdownConstraint(CurrentAngle,AngleMinConstraint,AngleMaxConstraint,ConstraintThreshold,AngleInput,bIsForYaw,TargetConstraint);
	return TargetConstraint != 0; 
}

void UAnglesConstraintModifier::GetTargetAngleSlowdownConstraint(double CurrentAngle, double AngleMinConstraint,
	double AngleMaxConstraint, double ConstraintThreshold, double AngleInput, bool bIsForYaw,
	double& TargetConstraint) const
{
	bool HasReachedAngleThreshold = CurrentAngle >= AngleMaxConstraint - ConstraintThreshold;
	bool IsRotatingClockwise = bIsForYaw ? AngleInput > 0 : AngleInput < 0;
	if(!HasReachedAngleThreshold || !IsRotatingClockwise)
	{
		AngleMaxConstraint = 0;
	}
	
	bool HasReachedMinThreshold = CurrentAngle <= AngleMinConstraint + ConstraintThreshold;
	bool IsRotatingCounterClockwise = bIsForYaw ? AngleInput < 0 : AngleInput > 0;

	if(HasReachedMinThreshold && IsRotatingCounterClockwise)
	{
		TargetConstraint = AngleMinConstraint;	
	}
	else
	{
		TargetConstraint = AngleMaxConstraint;
	}
}

bool UAnglesConstraintModifier::ProcessTurnRate_Implementation(float DeltaTime, FRotator InLocalControlRotation, float InPitchTurnRate, float InYawTurnRate, float& OutPitchTurnRate, float& OutYawTurnRate)
{
	if(!RPGCameraManager)
	{
		return false;
	}
	float PitchInput = InPitchTurnRate;
	float YawInput = InYawTurnRate;

	double NewPitchTurnRate = 0.f;
	// Apply pitch constraints
	ProcessSlowdownConstraintForAngle(
		DeltaTime,
		InLocalControlRotation.Pitch,
		RPGCameraManager->ViewPitchMin,
		RPGCameraManager->ViewPitchMax,
		PitchConstraintTolerance,
		PitchInput,
		false,
		NewPitchTurnRate);
	
	// Apply yaw constraints. Do not use this while the character is moving.
	double NewYawTurnRate = 0.f;
	if(!bHasMovementInput || TimeSinceMovementInput > 1.f)
	{
		ProcessSlowdownConstraintForAngle(
			DeltaTime,
			InLocalControlRotation.Yaw,
			RPGCameraManager->ViewYawMin,
			RPGCameraManager->ViewYawMax,
			YawConstraintTolerance,
			YawInput,
			true,
			NewYawTurnRate);
	}
	
	
	OutPitchTurnRate = NewPitchTurnRate;
	OutYawTurnRate = NewYawTurnRate;
	return false;
}
