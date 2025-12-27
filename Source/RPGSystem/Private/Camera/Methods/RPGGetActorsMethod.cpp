// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Methods/RPGGetActorsMethod.h"

void URPGGetActorsMethod::GetActors_Implementation(class AActor* InOwner, FVector OwnerLocation,
	FVector ViewPointLocation, FRotator ViewPointRotation, TArray<AActor*>& OutActors)
{
}

UWorld* URPGGetActorsMethod::GetWorld() const
{
	return UObject::GetWorld();
}
