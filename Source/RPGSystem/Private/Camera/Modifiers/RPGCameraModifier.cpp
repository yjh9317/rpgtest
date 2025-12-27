// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/RPGCameraModifier.h"

#include "Camera/RPGPlayerCameraManager.h"
#include "GameFramework/SpringArmComponent.h"


void URPGCameraModifier::EnableModifier()
{
	Super::EnableModifier();
	OnModifierEnabled(CameraOwner->GetCameraCacheView());
}

void URPGCameraModifier::DisableModifier(bool bImmediate)
{
	Super::DisableModifier(bImmediate);

	if (bImmediate && bDisabled && !bPendingDisable)
	{
		Alpha = 0.f;
	}
	OnModifierDisabled(CameraOwner->GetCameraCacheView(), bImmediate);
}

void URPGCameraModifier::OnModifierEnabled_Implementation(FMinimalViewInfo const& LastPOV)
{
}

void URPGCameraModifier::OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV, bool bImmediate)
{
}

void URPGCameraModifier::ProcessBoomLengthAndFOV_Implementation(float DeltaTime, float InFOV, float InArmLength, FVector ViewLocation, FRotator ViewRotation, float& OutFOV, float& OutArmLength)
{
	OutFOV = InFOV;
	OutArmLength = InArmLength;
}

void URPGCameraModifier::ProcessBoomOffsets_Implementation(float DeltaTime, FVector InSocketOffset, FVector InTargetOffset, FVector ViewLocation, FRotator ViewRotation, FVector& OutSocketOffset, FVector& OutTargetOffset)
{
	OutSocketOffset = InSocketOffset;
	OutTargetOffset = InTargetOffset;
}

void URPGCameraModifier::OnAnyLevelSequenceStarted_Implementation()
{
}

void URPGCameraModifier::OnAnyLevelSequenceEnded_Implementation()
{
}

void URPGCameraModifier::OnSetViewTarget_Implementation(bool bImmediate, bool bNewTargetIsOwner)
{
}

void URPGCameraModifier::PostUpdate_Implementation(float DeltaTime, FVector ViewLocation, FRotator ViewRotation)
{
}

bool URPGCameraModifier::IsDebugEnabled() const
{
	return bDebug;
}

void URPGCameraModifier::ToggleDebug(bool const bEnabled)
{
	bDebug = bEnabled;
}

bool URPGCameraModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	return Super::ModifyCamera(DeltaTime, InOutPOV);
}

void URPGCameraModifier::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& OutViewLocation, FRotator& OutViewRotation, float& OutFOV)
{
	Super::ModifyCamera(DeltaTime, ViewLocation, ViewRotation, FOV, OutViewLocation, OutViewRotation, OutFOV);

	OutViewLocation = ViewLocation;
	OutViewRotation = ViewRotation;
	OutFOV = FOV;

	UpdateOwnerReferences();

	if (!RPGCameraManager)
	{
		return;
	}

	if (!bPlayDuringCameraAnimations)
	{
		if (RPGCameraManager->IsPlayingAnyCameraAnimation())
		{
			return;
		}
	}

	if (OwnerController && OwnerPawn)
	{
		UpdateInternalVariables(DeltaTime);
		if (SpringArm)
		{
			ProcessBoomLengthAndFOV(DeltaTime, FOV, CurrentArmLength, ViewLocation, ViewRotation, OutFOV, SpringArm->TargetArmLength);
			ProcessBoomOffsets(DeltaTime, CurrentSocketOffset, CurrentTargetOffset, ViewLocation, ViewRotation, SpringArm->SocketOffset, SpringArm->TargetOffset);
		}
		else
		{
			// RPGLOG_ONCE(InvalidSpringArm, Error, TEXT("%s uses RPG but doesn't have a valid Spring Arm Component."), *GetNameSafe(OwnerPawn));
		}
		PostUpdate(DeltaTime, ViewLocation, ViewRotation);
	}
}

bool URPGCameraModifier::ProcessViewRotation(AActor* ViewTarget, float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
	bool bResult = Super::ProcessViewRotation(ViewTarget, DeltaTime, OutViewRotation, OutDeltaRot);

	if (!bPlayDuringCameraAnimations)
	{
		if (RPGCameraManager && RPGCameraManager->IsPlayingAnyCameraAnimation())
		{
			return bResult;
		}
	}

	if (ViewTarget && CameraOwner && CameraOwner->GetOwningPlayerController())
	{
		// TO DO #GravityCompatibility
		FVector const CameraLocation = CameraOwner->GetCameraLocation();
		FRotator const ControlRotation = CameraOwner->GetOwningPlayerController()->GetControlRotation();
		FRotator const OwnerRotation = ViewTarget ? ViewTarget->GetActorRotation() : FRotator::ZeroRotator;
		FRotator InLocalControlRotation = ControlRotation - OwnerRotation;
		InLocalControlRotation.Normalize();
		bResult = ProcessControlRotation(ViewTarget, DeltaTime, CameraLocation, OutViewRotation, InLocalControlRotation, OutDeltaRot, OutDeltaRot);
	}

	return bResult;
}

void URPGCameraModifier::UpdateOwnerReferences()
{
	RPGCameraManager = Cast<ARPGPlayerCameraManager>(CameraOwner);
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	if (!RPGCameraManager)
	{
		return;
	}

	OwnerController = RPGCameraManager->GetOwningPlayerController();
	OwnerCharacter = RPGCameraManager->OwnerCharacter;
	OwnerPawn = RPGCameraManager->OwnerPawn;
	if (OwnerPawn && OwnerPawn->IsLocallyControlled())
	{
		SpringArm = RPGCameraManager->CameraArm;
		MovementComponent = RPGCameraManager->MovementComponent;
	}
}

void URPGCameraModifier::UpdateInternalVariables(float DeltaTime)
{
	if (!RPGCameraManager)
	{
		return;
	}

	bHasMovementInput = RPGCameraManager->bHasMovementInput;
	PreviousMovementInput = MovementInput;
	MovementInput = RPGCameraManager->MovementInput;
	TimeSinceMovementInput = RPGCameraManager->TimeSinceMovementInput;
	bHasRotationInput = RPGCameraManager->bHasRotationInput;
	RotationInput = RPGCameraManager->RotationInput;
	TimeSinceRotationInput = RPGCameraManager->TimeSinceRotationInput;

	if (SpringArm)
	{
		CurrentSocketOffset = SpringArm->SocketOffset;
		CurrentTargetOffset = SpringArm->TargetOffset;
		CurrentArmLength = SpringArm->TargetArmLength;
	}
}

bool URPGCameraModifier::ProcessTurnRate_Implementation(float DeltaTime, FRotator InLocalControlRotation, float InPitchTurnRate, float InYawTurnRate, float& OutPitchTurnRate, float& OutYawTurnRate)
{
	return false;
}

bool URPGCameraModifier::ProcessControlRotation_Implementation(AActor* ViewTarget, float DeltaTime, FVector InViewLocation, FRotator InViewRotation, FRotator InLocalControlRotation, FRotator InDeltaRot, FRotator& OutDeltaRot)
{
	return false;
}

FVector URPGCameraModifier::GetOwnerVelocity() const
{
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	return RPGCameraManager->GetOwnerVelocity();
}

bool URPGCameraModifier::IsOwnerFalling() const
{
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	return RPGCameraManager->IsOwnerFalling();
}

bool URPGCameraModifier::IsOwnerStrafing() const
{
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	return RPGCameraManager->IsOwnerStrafing();
}

bool URPGCameraModifier::IsOwnerMovingOnGround() const
{
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	return RPGCameraManager->IsOwnerMovingOnGround();
}

void URPGCameraModifier::ComputeOwnerFloorDistance(float SweepDistance, float CapsuleRadius, bool& bOutFloorExists, float& OutFloorDistance) const
{
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	return RPGCameraManager->ComputeOwnerFloorDist(SweepDistance, CapsuleRadius, bOutFloorExists, OutFloorDistance);
}

void URPGCameraModifier::ComputeOwnerFloorNormal(float SweepDistance, float CapsuleRadius, bool& bOutFloorExists, FVector& OutFloorNormal) const
{
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	return RPGCameraManager->ComputeOwnerFloorNormal(SweepDistance, CapsuleRadius, bOutFloorExists, OutFloorNormal);
}

void URPGCameraModifier::ComputeOwnerSlopeAngle(float& OutSlopePitchDegrees, float& OutSlopeRollDegrees)
{
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	return RPGCameraManager->ComputeOwnerSlopeAngle(OutSlopePitchDegrees, OutSlopeRollDegrees);
}

float URPGCameraModifier::ComputeOwnerLookAndMovementDot()
{
	ensureMsgf(RPGCameraManager, TEXT("Please use RPG Camera Modifiers only with a player camera manager inheriting from RPGPlayerCameraManager."));
	return RPGCameraManager->ComputeOwnerLookAndMovementDot();
}

void URPGCameraAddOnModifier::SetSettings_Implementation(class URPGCameraAddOnModifierSettings* InSettings)
{
	Settings = InSettings;
}