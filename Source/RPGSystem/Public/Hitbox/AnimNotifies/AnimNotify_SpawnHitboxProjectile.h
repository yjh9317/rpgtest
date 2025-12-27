// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitBox/AnimNotifies/AnimNotify_HitboxBase.h"
#include "AnimNotify_SpawnHitboxProjectile.generated.h"

/**
 * Enum describing the different methods for spawning multiple projectiles
 */
UENUM(BlueprintType)
enum class ERHSSpawnType : uint8
{
	//Spawn randomly inside the extents of a bounding box
	RandomBoundingBox,
	//Spawn along vector describing both the direction and the distance between spawned instances.
	EvenlyAlongVector,
	EvenlyInCircle
};


/*
 * Settings governing projectiles' spawn distribution
 */
USTRUCT(BlueprintType)
struct FHitboxSpawnTypeSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Setup")
	ERHSSpawnType SpawnType = ERHSSpawnType::EvenlyAlongVector;

	/*
	 * If SpawnType is RandomBoundingBox, then this is the box's extent.
	 * If SpawnType is EvenlyAlongVector, then this describes both the direction along which projectiles are spawned, and the distance between each projectile instance.
	 * This means nothing when SpawnType is EvenlyInCircle.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Setup", meta = (UIMin = 0, ClampMin = 0, EditCondition = "SpawnType != ERHSSpawnType::EvenlyInCircle"))
	FVector SpawnLocationVariance = FVector(64.f, 0.f, 0.f);

	/*
	 * Only valid when Spawn Type is EvenlyInCircle.
	 * Radius of the circle
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Setup", meta = (UIMin = 0, ClampMin = 0, EditCondition = "SpawnType == ERHSSpawnType::EvenlyInCircle"))
	float CircleRadius = 64.f;

	/*
	 * Only valid when Spawn Type is EvenlyInCircle.
	 * The unit vector perpendicular to the circle and looking away from it.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Setup", meta = (UIMin = -1, UIMax = 1, ClampMin = -1, ClampMax = 1, EditCondition = "SpawnType == ERHSSpawnType::EvenlyInCircle"))
	FVector CircleRotationAxis = FVector::YAxisVector;

	/*
	 * Only valid when Spawn Type is EvenlyInCircle.
	 * Max circle angle, knowing that projectiles are spawned from each side of the 0 angle. This means that 360 will you give you a full circle, 180 a semi-circle, etc.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile Setup", meta = (UIMin = 0, UIMax = 360, ClampMin = 0, ClampMax = 360, EditCondition = "SpawnType == ERHSSpawnType::EvenlyInCircle"))
	float MaxCircleAngle = 92.f;
};

UCLASS(Abstract)
class RPGSYSTEM_API UAnimNotify_SpawnHitboxProjectile : public UAnimNotify_HitboxBase
{
	GENERATED_BODY()

protected:
	// Settings governing projectiles' spawn distribution
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile Setup")
	FHitboxSpawnTypeSettings ProjectileSpawnSettings;

	// Settings governing projectiles' bulk spawning and initial speed/velocity
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile Setup")
	FHitboxProjectileBulkSpawnSetting ProjectileBulkSpawnSettings;
};
