// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/Child/ArmLagModifier.h"
#include "GameFramework/SpringArmComponent.h"

void UArmLagModifier::UpdateLagSettings()
{
	if (!SpringArm)
	{
		return;
	}
	
	if (ArmLagSettings.bOverrideSpringArmComponentSettings)
	{
		SpringArm->bEnableCameraLag = ArmLagSettings.bEnableCameraLag;
		SpringArm->bEnableCameraRotationLag = ArmLagSettings.bEnableCameraRotationLag;
		SpringArm->CameraLagSpeed = ArmLagSettings.CameraLagSpeed;
		SpringArm->CameraRotationLagSpeed = ArmLagSettings.CameraRotationLagSpeed;
		SpringArm->CameraLagMaxDistance = ArmLagSettings.CameraLagMaxDistance;
	}
}
