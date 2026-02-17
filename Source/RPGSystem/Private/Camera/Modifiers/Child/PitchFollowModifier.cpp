// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/Child/PitchFollowModifier.h"

#include "Camera/RPGPlayerCameraManager.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

void UPitchFollowModifier::UpdatePitchFollowState(FRotator LocalControlRotation, bool& bCanTriggerFollow, EPitchFollowState& OutPitchFollowState)
{
	float Temp = 0.f;
	ComputeOwnerSlopeAngle(CurrentSlopeAngle, Temp);

	bool ShouldApplyAimOffset = !UKismetMathLibrary::NearlyEqual_FloatFloat(
		LocalControlRotation.Pitch * UKismetMathLibrary::SignOfFloat(ComputeOwnerLookAndMovementDot()), 2.f);
	if (IsOwnerMovingOnGround() &&
		FMath::Abs(CurrentSlopeAngle) >= FollowSettings.SlopeMinIncline &&
		ShouldApplyAimOffset &&
		bHasMovementInput)
	{
		// Conditions when moving on a slope. This has the highest priority.
		bCanTriggerFollow = true;
		OutPitchFollowState = EPitchFollowState::OnSlope;
	}
	else
	{
		// Conditions when just moving and not on a slope (or modifier doesn't trigger on slopes)
		bool IsPitchActive = !UKismetMathLibrary::NearlyEqual_FloatFloat(
			LocalControlRotation.Pitch, FollowSettings.PitchFollowAngleThreshold, FollowSettings.RestingPitch);
		bool IsOnFlatGround = UKismetMathLibrary::NearlyEqual_FloatFloat(CurrentSlopeAngle, 0, 0.000001);
		if (bHasMovementInput && IsOwnerMovingOnGround() && FollowSettings.bEnablePitchMovementFollow && IsPitchActive
			&& (IsOnFlatGround || !FollowSettings.bTriggerOnSlopes))
		{
			bCanTriggerFollow = true;
			OutPitchFollowState = EPitchFollowState::Moving;
		}
		else
		{
			bool OutFloorExists = false;
			float OutFloorDistance = 0.f;
			ComputeOwnerFloorDistance(FollowSettings.MinDistanceFromGroundToTriggerWhenFalling, GetCapsuleRadius(),
			                          OutFloorExists, OutFloorDistance);

			if (FollowSettings.bTriggerWhenFalling && IsOwnerFalling() && LocalControlRotation.Pitch >= RPGCameraManager
				->ViewPitchMin &&
				(!OutFloorExists || OutFloorDistance >= FollowSettings.MinDistanceFromGroundToTriggerWhenFalling))
			{
				bCanTriggerFollow = true;
				OutPitchFollowState = EPitchFollowState::Falling;
			}
			else
			{
				bCanTriggerFollow = false;
				OutPitchFollowState = EPitchFollowState::Disabled;
			}
		}
	}
}

double UPitchFollowModifier::GetFollowSpeed()
{
	return FollowSettings.PitchFollowSpeed *
		(IsOwnerFalling() ? FollowSettings.SpeedMultiplierWhenFalling : 1.f);
}

double UPitchFollowModifier::GetRotationIncrementationSign(EPitchFollowState InPitchFollowState,
                                                           FRotator LocalControlRotation)
{
	switch (InPitchFollowState)
	{
	case EPitchFollowState::Disabled:
		return 0.f;

	case EPitchFollowState::Moving:
		return FollowSettings.RestingPitch > LocalControlRotation.Pitch ? 1.f : -1.f;

	case EPitchFollowState::OnSlope:
	case EPitchFollowState::Falling:
		float RelativeSlopeAngle = UKismetMathLibrary::SignOfFloat(ComputeOwnerLookAndMovementDot()) * CurrentSlopeAngle;
		return RelativeSlopeAngle > LocalControlRotation.Pitch && !IsOwnerFalling() ? 1.f : -1.f;
	}
	return 0.f;
}

double UPitchFollowModifier::GetTimeThreshold()
{
	return IsOwnerFalling() ? FollowSettings.PitchFollowTimeThresholdWhenFalling : FollowSettings.PitchFollowTimeThreshold;
}

bool UPitchFollowModifier::ProcessControlRotation_Implementation(AActor* ViewTarget, float DeltaTime, FVector InViewLocation,
                                                  FRotator InViewRotation, FRotator InLocalControlRotation,
                                                  FRotator InDeltaRot, FRotator& OutDeltaRot)
{
	if(!RPGCameraManager || !OwnerPawn)
	{
		BlendTimer = 0.f;
		OutDeltaRot = InDeltaRot;
		return false;
	}
	
	bool CanTriggerFollow = false;
	UpdatePitchFollowState(InLocalControlRotation,CanTriggerFollow,PitchFollowState);
	if(!RPGCameraManager || !OwnerPawn || !CanTriggerFollow)
	{
		BlendTimer = 0.f;
		OutDeltaRot = InDeltaRot;
		return false;
	}

	if(!bHasRotationInput && TimeSinceRotationInput > GetTimeThreshold())
	{
		//Apply Modifier

		// Timer used so that the modifier's start isn't jarring
		BlendTimer = FMath::Clamp(BlendTimer + DeltaTime,0.f,AlphaInTime);

		float UpdatedPitchValue = InDeltaRot.Pitch + GetFollowSpeed() * DeltaTime * GetRotationIncrementationSign(PitchFollowState,InLocalControlRotation);
		// FRotator NewRotator(UpdatedPitchValue,InDeltaRot.Yaw,InDeltaRot.Pitch);
		FRotator NewRotator(UpdatedPitchValue, InDeltaRot.Yaw, 0.0f);
		float ModifierStrength = AlphaInTime > 0 ? BlendTimer / AlphaInTime : 1.f;
		OutDeltaRot = UKismetMathLibrary::RLerp(InDeltaRot,NewRotator,ModifierStrength,true);
		return false;
	}
	else
	{
		// Reset timer
		BlendTimer = 0.f;	
	}
	return false;
}

double UPitchFollowModifier::GetCapsuleRadius() const
{
	if(OwnerCharacter)
	{
		return OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	}
	return 250.f;
}
