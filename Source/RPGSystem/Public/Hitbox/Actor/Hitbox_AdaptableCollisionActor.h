// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Hitbox_AdaptableCollisionActor.generated.h"

UCLASS()
class RPGSYSTEM_API AHitbox_AdaptableCollisionActor : public AActor
{
	GENERATED_BODY()
	
public:
	AHitbox_AdaptableCollisionActor();

	/*
	 * The equivalent of the construction script in blueprint. This is used to update the adaptable collision shapes and set them as the root component.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/*
	 * Set whether the dynamic collision component of this actor can affect navmesh that gets generated in run-time.
	 */
	UFUNCTION(BlueprintCallable, Category = "Projectile Settings")
	virtual void SetCanAffectNavigation(const bool canAffect);

	/*
	 * Draw this class' default CollisionComponent in the world.
	 */
	UFUNCTION(BlueprintCallable, Category = "Collision Shape", meta = (WorldContext = "WorldContextObject"))
	static void DrawClassDefaultCollisionShape(const UObject* WorldContextObject, const UClass* RHS_AdaptableCollisionActorClass, const FVector& Location, const FQuat& Rotation, const FLinearColor Color, const float Duration, const float Thickness);

	/*
	 * Get this class' default shape extent.
	 */
	UFUNCTION(BlueprintPure, Category = "Collision Shape")
	static FVector GetClassDefaultShapeExtent(const UClass* RHS_AdaptableCollisionActorClass);
protected:
	// SFX playing throughout the lifespan of the actor
	UPROPERTY(VisibleAnywhere, Category = "SFX", BlueprintReadWrite)
	class UAudioComponent* MainSFX;

	/*
	 * The root component of the actor. The shape can be changed in the ShapeType settings anywhere except for runtime.
	 */
	UPROPERTY(VisibleAnywhere, Category = "Collision Shape", BlueprintReadWrite)
	class UShapeComponent* CollisionComponent;

	// Instance of collision shape to set up (Sphere, Box or Capsule) and apply as the root of the actor in the construction script.
	UPROPERTY(EditAnywhere, Instanced, Category = "Collision Shape", BlueprintReadOnly)
	class UHitboxCollisionShape* ShapeType;

private:
	void SetupCollisionComponent(class UShapeComponent* Collision, const FTransform& Transform);
};
