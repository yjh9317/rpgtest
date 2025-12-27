// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Methods/RPGFocusTargetMethod.h"

AActor* URPGFocusTargetMethod::GetTargetLocation_Implementation(class AActor* InOwner, FVector OwnerLocation,
	FVector ViewPointLocation, FRotator ViewPointRotation, FVector& OutTargetLocation)
{
	return nullptr;
}

UWorld* URPGFocusTargetMethod::GetWorld() const
{
	if (GWorld && GWorld->IsGameWorld() && GWorld->HasBegunPlay())
	{
		return GWorld;
	}
	return nullptr;
}
