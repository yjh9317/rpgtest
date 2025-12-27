// Fill out your copyright notice in the Description page of Project Settings.


#include "HitBox/Actor/Hitbox_AdaptableCollisionActor.h"

#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/ShapeComponent.h"
#include "Components/SphereComponent.h"
#include "Hitbox/HitboxCollisionShape.h"

AHitbox_AdaptableCollisionActor::AHitbox_AdaptableCollisionActor()
{
	// This scene component should be set as root and never deleted. This will be changed in runtime and replaced by the collision component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("NEVERDELETETHIS"));
    RootComponent->SetIsReplicated(true);

    MainSFX = CreateDefaultSubobject<UAudioComponent>(TEXT("MainSFX"));
    MainSFX->SetupAttachment(RootComponent);
    MainSFX->SetIsReplicated(true);

	// Replication
    bReplicates = true;
    AActor::SetReplicateMovement(true);
}

void AHitbox_AdaptableCollisionActor::SetupCollisionComponent(UShapeComponent* Collision, const FTransform& Transform)
{
    if (!Collision)
        return;

	// Register new collision shape and set replication
    CollisionComponent = Collision;
    CollisionComponent->RegisterComponent();
	CollisionComponent->SetIsReplicated(true);


    // Set up new collision shape as the new root component
    USceneComponent* OldRootComponent = RootComponent;
    RootComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    RootComponent = nullptr;
    SetRootComponent(CollisionComponent);

	if (OldRootComponent)
	{
		// If old root was another collision shape, destroy it from world
		if (OldRootComponent->IsA(UShapeComponent::StaticClass()))
		    OldRootComponent->DestroyComponent(false);
	    else // Otherwise, attach the NEVERDELETETHIS root component to the collision shape. This helps avoid a lot of errors regarding attachments, null parents etc
			OldRootComponent->AttachToComponent(CollisionComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	}

	// Reset transform
    SetActorTransform(Transform);
}

void AHitbox_AdaptableCollisionActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (!ShapeType)
        return;
	
    // If the newly selected shape type is a sphere
    UHitboxCollisionShape_Sphere* SphereShape = Cast<UHitboxCollisionShape_Sphere>(ShapeType);
    if (SphereShape)
    {
        USphereComponent* SphereComponent = Cast<USphereComponent>(CollisionComponent);
    	// If the previously selected collision component wasn't a sphere
        if (!SphereComponent)
        {
        	// Create and set up new sphere collision
            USphereComponent* SphereCollision = NewObject<USphereComponent>(this, USphereComponent::StaticClass(), TEXT("SphereCollision"), RF_NoFlags);
            SphereCollision->SetSphereRadius(SphereShape->Radius);

            SetupCollisionComponent(SphereCollision, Transform);
        }
        else
        {
        	// Update sphere radius
            SphereComponent->SetSphereRadius(SphereShape->Radius);
        }

        return;
    }

    // If the newly selected shape type is a capsule
    UHitboxCollisionShape_Capsule* CapsuleShape = Cast<UHitboxCollisionShape_Capsule>(ShapeType);
    if (CapsuleShape)
    {
        UCapsuleComponent* CapsuleComponent = Cast<UCapsuleComponent>(CollisionComponent);
        // If the previously selected collision component wasn't a capsule
        if (!CapsuleComponent)
        {
            // Create and set up new capsule collision
            UCapsuleComponent* CapsuleCollision = NewObject<UCapsuleComponent>(this, UCapsuleComponent::StaticClass(), TEXT("CapsuleCollision"), RF_NoFlags);
            CapsuleCollision->SetCapsuleSize(CapsuleShape->Radius, CapsuleShape->HalfHeight, true);

            SetupCollisionComponent(CapsuleCollision, Transform);
        }
        else
        {
            // Update capsule radius and half height
            CapsuleComponent->SetCapsuleSize(CapsuleShape->Radius, CapsuleShape->HalfHeight, true);
        }
        return;
    }

    // If the newly selected shape type is a box
    UHitboxCollisionShape_Box* BoxShape = Cast<UHitboxCollisionShape_Box>(ShapeType);
    if (BoxShape)
    {
        UBoxComponent* BoxComponent = Cast<UBoxComponent>(CollisionComponent);
        // If the previously selected collision component wasn't a box
        if (!BoxComponent)
        {
        	// Create and set up new box collision
            UBoxComponent* BoxCollision = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass(), TEXT("BoxCollision"), RF_NoFlags);
            BoxCollision->SetBoxExtent(BoxShape->Extent/2.f);
            SetupCollisionComponent(BoxCollision, Transform);
        }
        else
        {
        	// Update box collision
            BoxComponent->SetBoxExtent(BoxShape->Extent/2.f);
        }
        return;
    }
}

void AHitbox_AdaptableCollisionActor::SetCanAffectNavigation(const bool canAffect)
{
    if (CollisionComponent)
    {
        CollisionComponent->SetCanEverAffectNavigation(canAffect);
    }
}

void AHitbox_AdaptableCollisionActor::DrawClassDefaultCollisionShape(const UObject* WorldContextObject, const UClass* HitboxAdaptableCollisionActorClass, const FVector& Location, const FQuat& Rotation, const FLinearColor Color, const float Duration, const float Thickness)
{
	// Check if input is correct
    if (!HitboxAdaptableCollisionActorClass || !HitboxAdaptableCollisionActorClass->IsChildOf(AHitbox_AdaptableCollisionActor::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid class. Should be child of AHitbox_AdaptableCollisionActor"));
        return;
    }

	// Draw class default debug shape
    UHitboxCollisionShape* Shape = HitboxAdaptableCollisionActorClass->GetDefaultObject<AHitbox_AdaptableCollisionActor>()->ShapeType;
	if (Shape)
	    Shape->DrawDebugShape(WorldContextObject, Location, Rotation, Color.ToFColor(true), Duration, Thickness);
}

FVector AHitbox_AdaptableCollisionActor::GetClassDefaultShapeExtent(const UClass* HitboxAdaptableCollisionActorClass)
{
    if (!HitboxAdaptableCollisionActorClass || !HitboxAdaptableCollisionActorClass->IsChildOf(AHitbox_AdaptableCollisionActor::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid class. Should be child of AHitbox_AdaptableCollisionActor"));
        return FVector::ZeroVector;
    }

	UHitboxCollisionShape* Shape = HitboxAdaptableCollisionActorClass->GetDefaultObject<AHitbox_AdaptableCollisionActor>()->ShapeType;
    return Shape? Shape->GetShapeExtentVector() : FVector::ZeroVector;
}


