// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Net/UnrealNetwork.h"
#include "HitboxCollisionDataAsset.generated.h"

class UHitboxCollisionShape;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class RPGSYSTEM_API UHitboxCollisionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	// Collision shapes to trace against the world. Each notify detects each hit actor only once.
	UPROPERTY(EditAnywhere, Instanced, Category = "HitBox", BlueprintReadOnly, Replicated)
	TArray<UHitboxCollisionShape*> CollisionShapes;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
	{
		Super::GetLifetimeReplicatedProps(OutLifetimeProps);
		DOREPLIFETIME(UHitboxCollisionDataAsset, CollisionShapes);
	}
};
