// Fill out your copyright notice in the Description page of Project Settings.


#include "Hitbox/HitboxFunctionLibrary.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Engine/Engine.h"
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 4
#include "Engine/OverlapResult.h"
#endif
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "GameFramework/PlayerController.h"

namespace
{
	/**
	 * Configure the given query params to perform traces in the world
	 *  @param  WorldContextObject	The world context object in which this call is taking place
	 *  @param	bTraceComplex		Whether we should trace against complex collision
	 *  @param  ActorsToIgnore		Actors to be ignored by ray
	 * 	@param 	bIgnoreSelf			Whether ray should ignore self/owner
	 * 	@param	InTraceTag			Name/Tag of the trace we are about to perform
	 * 	@param	OutParams			Params passed a reference (out parameter) to be configured
	 */
	void ConfigureQueryParams(const UObject* WorldContextObject, const bool bTraceComplex, const TArray<AActor*>& ActorsToIgnore, const bool bIgnoreSelf, const FName& InTraceTag, FCollisionQueryParams& OutParams)
	{
		if (!WorldContextObject)
			return;

		OutParams = FCollisionQueryParams(InTraceTag, SCENE_QUERY_STAT_ONLY(KismetTraceUtils), bTraceComplex);
		OutParams.bReturnPhysicalMaterial = true;
		OutParams.bReturnFaceIndex = !UPhysicsSettings::Get()->bSuppressFaceRemapTable; // Ask for face index, as long as we didn't disable globally
		OutParams.AddIgnoredActors(ActorsToIgnore);
		if (bIgnoreSelf)
		{
			const AActor* IgnoreActor = Cast<AActor>(WorldContextObject);
			if (IgnoreActor)
			{
				OutParams.AddIgnoredActor(IgnoreActor);
			}
			else
			{
				// find owner
				const UObject* CurrentObject = WorldContextObject;
				while (CurrentObject)
				{
					CurrentObject = CurrentObject->GetOuter();
					IgnoreActor = Cast<AActor>(CurrentObject);
					if (IgnoreActor)
					{
						OutParams.AddIgnoredActor(IgnoreActor);
						break;
					}
				}
			}
		}
	}
}

void UHitboxFunctionLibrary::BPGetPlayerViewPoint(const class APlayerController* PlayerController, FVector& OutLocation,
	FRotator& OutRotation)
{
	if (!PlayerController)
		return;

	PlayerController->GetPlayerViewPoint(OutLocation, OutRotation);
}

bool UHitboxFunctionLibrary::IsInEditorWorld(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World && World->WorldType == EWorldType::EditorPreview;
}

bool UHitboxFunctionLibrary::IsInGameWorld(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	return World && World->IsGameWorld();
}

bool UHitboxFunctionLibrary::CapsuleOverlapMultiByProfile(const UObject* WorldContextObject, const FVector& Pos,
	const FQuat& Rot, const float Radius, const float HalfHeight, const FName ProfileName, const bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore, const bool bIgnoreSelf, TArray<AActor*>& OutHitActors)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
		return false;

	// Setup Collision Query Params
	FCollisionQueryParams OutParams;
	ConfigureQueryParams(WorldContextObject, bTraceComplex, ActorsToIgnore, bIgnoreSelf, TEXT("CapsuleOverlapMultiByProfile"), OutParams);

	TArray<FOverlapResult> OutOverlaps;
	const bool bOverlapped =  World->OverlapMultiByProfile(OutOverlaps, Pos, Rot, ProfileName, FCollisionShape::MakeCapsule(Radius, HalfHeight), OutParams);
	if (!bOverlapped)
		return false;

	// Return all overlapped actors
	for (auto const& OutOverlap : OutOverlaps)
	{
		if (OutOverlap.GetActor() != nullptr)
			OutHitActors.Add(OutOverlap.GetActor());
	}

	return true;
}

bool UHitboxFunctionLibrary::BoxOverlapMultiByProfile(const UObject* WorldContextObject, const FVector& Pos,
	const FQuat& Rot, const FVector& BoxHalfExtent, const FName ProfileName, const bool bTraceComplex,
	const TArray<AActor*>& ActorsToIgnore, const bool bIgnoreSelf, TArray<AActor*>& OutHitActors)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World)
		return false;

	// Setup Collision Query Params
	FCollisionQueryParams OutParams;
	ConfigureQueryParams(WorldContextObject, bTraceComplex, ActorsToIgnore, bIgnoreSelf, TEXT("BoxOverlapMultiByProfile"), OutParams);

	TArray<FOverlapResult> OutOverlaps;
	const bool bOverlapped = World->OverlapMultiByProfile(OutOverlaps, Pos, Rot, ProfileName, FCollisionShape::MakeBox(BoxHalfExtent), OutParams);
	if (!bOverlapped)
		return false;

	// Return all overlapped actors
	for (auto const& OutOverlap : OutOverlaps)
	{
		if (OutOverlap.GetActor() != nullptr)
			OutHitActors.Add(OutOverlap.GetActor());
	}

	return true;
}
