// Fill out your copyright notice in the Description page of Project Settings.


#include "HitBox/AnimNotifies/AnimNotifyState_HitboxBase.h"

#include "Net/UnrealNetwork.h"

void UAnimNotifyState_HitboxBase::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	
	DOREPLIFETIME(UAnimNotifyState_HitboxBase, DebugSettings);
	DOREPLIFETIME(UAnimNotifyState_HitboxBase, ValidPreviousLocs);
	DOREPLIFETIME(UAnimNotifyState_HitboxBase, PreviousLocs);
}


void UAnimNotifyState_HitboxBase::ServerDrawHitShapeDebug_Implementation(class USkeletalMeshComponent* MeshComp,
                                                                         class UHitboxCollisionShape* CollisionShape, const FVector& Location, const FQuat& Rotation,
                                                                         const float FrameDeltaTime) const
{
	MulticastDrawHitShapeDebug_Implementation(MeshComp, CollisionShape, Location, Rotation, FrameDeltaTime);
}

void UAnimNotifyState_HitboxBase::MulticastDrawHitShapeDebug_Implementation(class USkeletalMeshComponent* MeshComp,
	class UHitboxCollisionShape* CollisionShape, const FVector& Location, const FQuat& Rotation,
	const float FrameDeltaTime) const
{
	DrawHitShapeDebug(MeshComp, CollisionShape, Location, Rotation, FrameDeltaTime);
}

void UAnimNotifyState_HitboxBase::ServerApplyShapeHit_Implementation(int index, class USkeletalMeshComponent* MeshComp,
	class UHitboxCollisionShape* CollisionShape, const FVector& Location, const FQuat& Rotation) const
{
	ApplyShapeHit(index, MeshComp, CollisionShape, Location, Rotation);
}

void UAnimNotifyState_HitboxBase::SetPreviousLoc_Implementation(int index, const FVector& Location) const
{
	if (PreviousLocs.IsEmpty() || index < 0 || index >= PreviousLocs.Num())
	{
		return;
	}

	PreviousLocs[index] = Location;
}

void UAnimNotifyState_HitboxBase::SetPreviousLocValid_Implementation(int index, bool Valid) const
{
	if (ValidPreviousLocs.IsEmpty() || index < 0 || index >= ValidPreviousLocs.Num())
	{
		return;
	}
	ValidPreviousLocs[index] = Valid;
}

void UAnimNotifyState_HitboxBase::SetupPreviousLocs_Implementation(const AActor* Owner) const
{
	auto const& CollisionShapes = GetCollisionShapes(Owner);
	if (CollisionShapes.IsEmpty())
	{
		return;
	}

	PreviousLocs.Empty();
	ValidPreviousLocs.Empty();

	PreviousLocs.AddDefaulted(CollisionShapes.Num());
	ValidPreviousLocs.AddDefaulted(CollisionShapes.Num());

	check(PreviousLocs.Num() == CollisionShapes.Num());
	check(PreviousLocs.Num() == ValidPreviousLocs.Num());
}


TArray<class UHitboxCollisionShape*> UAnimNotifyState_HitboxBase::GetCollisionShapes_Implementation(
	const AActor* Owner) const
{
	return {};
}

void UHitboxAnimNotify::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHitboxAnimNotify, CollisionShapes);
}

TArray<class UHitboxCollisionShape*> UHitboxAnimNotify::GetCollisionShapes_Implementation(const AActor* Owner) const
{
	return CollisionShapes;
}
