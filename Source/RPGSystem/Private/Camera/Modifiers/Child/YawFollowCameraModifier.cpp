// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/Child/YawFollowCameraModifier.h"
#include "Kismet/KismetMathLibrary.h"

bool UYawFollowCameraModifier::CanTriggerFollow(FRotator LocalControlRotation)
{
	// Don't use follow yaw if character is strafing
	// !IsOwnerStrafing();

	//Character isn't looking forward or backward (+- threshold)
	bool bIsLookingSideways = FMath::Abs(LocalControlRotation.Yaw) >= FollowSettings.YawFollowAngleThreshold && (180 -
		FMath::Abs(LocalControlRotation.Yaw)) >= FollowSettings.YawFollowAngleThreshold;

	//Do not follow the yaw if pure input is forward or backward (i.e., only W or S are pressed)
	bool bIsNotStrafing = !UKismetMathLibrary::EqualEqual_VectorVector(
		UKismetMathLibrary::Vector_GetAbs(MovementInput.GetSafeNormal()),
		FVector{0.f, 1.f, 0.f}, 0.1);
		
	return bHasMovementInput && FollowSettings.bEnableYawMovementFollow && !IsOwnerStrafing() && bIsLookingSideways &&
		bIsNotStrafing;
}

bool UYawFollowCameraModifier::ProcessControlRotation_Implementation(AActor* ViewTarget, float DeltaTime, FVector InViewLocation,
                                                      FRotator InViewRotation, FRotator InLocalControlRotation,
                                                      FRotator InDeltaRot, FRotator& OutDeltaRot)
{
	if(!OwnerPawn || !RPGCameraManager || !CanTriggerFollow(InLocalControlRotation))
	{
		BlendTimer = 0.f;
		return false;
	}

	if(!bHasRotationInput && TimeSinceRotationInput > FollowSettings.YawFollowTimeThreshold)
	{
		// Apply Modifier

		// Timer used so that the modifier's start isn't jarring
		BlendTimer = FMath::Clamp(BlendTimer + DeltaTime,0.f,AlphaInTime);

		// Scale so that small movement input stick deflections don't rotate the camera aggressively
		const float RotationRateScale = FMath::Clamp(FMath::Abs(InLocalControlRotation.Yaw) / 90,0.001f,1.f);
		
		// Calculate the yaw adjustment for this frame
		const float YawAdjustment = FollowSettings.YawFollowSpeed * DeltaTime * UKismetMathLibrary::SignOfFloat(InLocalControlRotation.Yaw) * -1 * RotationRateScale;
		FRotator NewRotation(InDeltaRot.Pitch,InDeltaRot.Yaw + YawAdjustment,InDeltaRot.Roll);

		float ModifierStrength = AlphaInTime > 0 ? BlendTimer/AlphaInTime : 1.f;
		OutDeltaRot = UKismetMathLibrary::RLerp(InDeltaRot, NewRotation,ModifierStrength,true);
	}
	else
	{
		BlendTimer = 0.f;
	}
	return false;
}
