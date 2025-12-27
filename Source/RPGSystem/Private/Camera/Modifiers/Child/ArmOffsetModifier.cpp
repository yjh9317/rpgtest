// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/Child/ArmOffsetModifier.h"

#include "Camera/RPGPlayerCameraManager.h"


void UArmOffsetModifier::ProcessBoomOffsets_Implementation(float DeltaTime, FVector InSocketOffset, FVector InTargetOffset,
                                                           FVector ViewLocation, FRotator ViewRotation, FVector& OutSocketOffset, FVector& OutTargetOffset)
{
	Super::ProcessBoomOffsets_Implementation(DeltaTime, InSocketOffset, InTargetOffset, ViewLocation, ViewRotation, OutSocketOffset, OutTargetOffset);
	
	if(RPGCameraManager)
	{
		RPGCameraManager->GetCurrentArmOffsets(OutSocketOffset,OutTargetOffset);	
	}
}
