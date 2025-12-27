// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RPGGetActorsMethod.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API URPGGetActorsMethod : public UObject
{
	GENERATED_BODY()
public:
	/*
	 * Get the all actors relevant for this method.
	 * @param   Owner               The owner of the camera.
	 * @param   OwnerLocation       The world location of the owner of camera.
	 * @param   ViewPointLocation   Camera's location.
	 * @param   ViewPointRotation   Camera's rotation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "UGC|Methods")
	void GetActors(class AActor* InOwner, FVector OwnerLocation, FVector ViewPointLocation, FRotator ViewPointRotation, TArray<AActor*>& OutActors);

private:
	/** Getter for the cached world pointer, will return null if the actor is not actually spawned in a level */
	virtual UWorld* GetWorld() const override;
};
