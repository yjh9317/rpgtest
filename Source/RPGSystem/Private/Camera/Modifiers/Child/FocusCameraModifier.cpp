// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/Child/FocusCameraModifier.h"

#include "Camera/RPGPlayerCameraManager.h"
#include "Camera/Methods/RPGFocusTargetMethod.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetMathLibrary.h"

bool UFocusCameraModifier::ProcessControlRotation_Implementation(AActor* ViewTarget, float DeltaTime, FVector InViewLocation,
                                                  FRotator InViewRotation, FRotator InLocalControlRotation, FRotator InDeltaRot, FRotator& OutDeltaRot)
{
	if(!OwnerPawn || !RPGCameraManager || !OwnerController)
	{
		return false;
	}

	// Handle ignoring and resetting look input
	UpdateIgnoreLookInput(OwnerController);

	if(FocusSettings.bEnabled)
	{
		if(FocusSettings.FocusTargetMethod)
		{
			if(RPGCameraManager->IsYawMovementFollowEnabled() || RPGCameraManager->IsPitchMovementFollowEnabled())
          {
             // DebugHeader::~
          }
          else
          {
             // Get Target Location
             FVector OutTargetLocation = FVector::ZeroVector;
             
             if(AActor* Target = FocusSettings.FocusTargetMethod->GetTargetLocation(OwnerPawn,OwnerPawn->GetActorLocation(),InViewLocation,InViewRotation,OutTargetLocation))
             {
                // Update timer to know if Line of Sight is blocked
                UpdateLOSTimerAndEvent(DeltaTime,OwnerController,Target);
             }

             bValidConditions = CanFocusTarget(OutTargetLocation);

             if(bValidConditions)
             {
                // Rotate camera to target
                FVector OwnerPawnViewLoc;
                FRotator OwnerPawnViewRot;
                OwnerPawn->GetActorEyesViewPoint(OwnerPawnViewLoc,OwnerPawnViewRot);

                FRotator PawnToTargetRot = UKismetMathLibrary::FindLookAtRotation(OwnerPawnViewLoc,OutTargetLocation);
                FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(PawnToTargetRot,InViewRotation);
                
                if(FocusSettings.bRotateYawOnly)
                {
                   TargetRotation = FRotator(0.f,-InViewRotation.Pitch,RotationDelta.Yaw);
                }
                else
                {
                   TargetRotation = FRotator(0.f,RotationDelta.Pitch,RotationDelta.Yaw);
                }
             }

             FRotator CombinedRotation = UKismetMathLibrary::ComposeRotators(InDeltaRot,TargetRotation);
             FRotator FinalRotation = UKismetMathLibrary::ComposeRotators(CombinedRotation,FocusSettings.RotationOffset);
             FRotator InterpolatedRotation = UKismetMathLibrary::RInterpTo(InDeltaRot, FinalRotation, DeltaTime, FocusSettings.InterpSpeed);
             
             OutDeltaRot.Roll = InDeltaRot.Roll;
             OutDeltaRot.Pitch = InterpolatedRotation.Pitch;
             OutDeltaRot.Yaw = InterpolatedRotation.Yaw;
             
             return false;
          }
       }
	}
	return false;
}

bool UFocusCameraModifier::CanFocusTarget(FVector TargetLocation) const
{
	const float Distance = UKismetMathLibrary::Vector_Distance(TargetLocation, OwnerPawn->GetActorLocation());

	return FocusSettings.bEnabled &&
		Distance <= FocusSettings.MaxDistanceFromTarget &&
		!LOSBlockedAboveThreshold();
}

void UFocusCameraModifier::UpdateLOSTimerAndEvent(double DeltaTime, AController* PlayerController, AActor* FocusTarget)
{
	if(FocusSettings.bStopIfBlockedLOS)
	{
		bool bWasBlockedAboveThreshold = LOSBlockedAboveThreshold();
		if(PlayerController)
		{
			bool IsTargetInSight = PlayerController->LineOfSightTo(FocusTarget,{0.f,0.f,0.f},false);
			LOSBlockedTimer = IsTargetInSight ? 0.f : LOSBlockedTimer + DeltaTime;

			// Broadcast an event when the Line of sight to the focus target has been blocked for too long in order to disable the Focus modifier.
			// The modifier will need to get reactivated again.
			if(LOSBlockedAboveThreshold() && !bWasBlockedAboveThreshold)
			{
				OnBlockedFocusLineOfSight.Broadcast();
			}
		}
	}
}

void UFocusCameraModifier::UpdateIgnoreLookInput(APlayerController* PlayerController)
{
	if(!PlayerController)
	{
		return;
	}

	if(bValidConditions && FocusSettings.bEnabled)
	{
		if(FocusSettings.bIgnoreCameraInput && !PlayerController->IsLookInputIgnored())
		{
			bWasIgnoringLookInput = true;
			PlayerController->SetIgnoreLookInput(true);
		}
	}
	else
	{
		if(bWasIgnoringLookInput && PlayerController->IsLookInputIgnored())
		{
			bWasIgnoringLookInput = false;
			PlayerController->SetIgnoreLookInput(false);
		}
	}
}

void UFocusCameraModifier::SanitizeLocationIfTooClose(FVector TargetLocation, FVector InViewPointLocation,
	FVector InCameraLocation, FRotator InViewRotation, bool& bTooClose, FVector& OutLocation)
{
	bool IsOutOfFocusRange = UKismetMathLibrary::Vector_Distance(TargetLocation,InViewPointLocation) >= FocusSettings.MinDistanceFromTarget;
	
	FVector DistBetweenTargetAndCamera = TargetLocation - InCameraLocation;
	FVector StartToTarget = TargetLocation + DistBetweenTargetAndCamera.GetSafeNormal2D() * FocusSettings.MinDistanceFromTarget;

	bTooClose = !IsOutOfFocusRange;
	OutLocation = IsOutOfFocusRange ? TargetLocation : StartToTarget;
	
}

bool UFocusCameraModifier::LOSBlockedAboveThreshold() const
{
	return FocusSettings.bStopIfBlockedLOS && LOSBlockedTimer > FocusSettings.BlockedLOSTimeThreshold;
}
