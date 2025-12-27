// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/RPGCameraDitheringModifier.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"


namespace DitherHelpers
{
	bool DoAnyComponentsOverlap(AActor* inActor, ECollisionChannel overlapChannel)
	{
		if (inActor)
		{
			for (UActorComponent* ActorComponent : inActor->GetComponents())
			{
				UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(ActorComponent);
				if (Primitive && Primitive->IsCollisionEnabled())
				{
					if (Primitive->GetCollisionResponseToChannel(overlapChannel) == ECollisionResponse::ECR_Overlap)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	int32 FindInactiveDitherState(TArray<FDitheredActorState>& DitherStates)
	{
		for (int32 Index = 0; Index < DitherStates.Num(); ++Index)
		{
			const FDitheredActorState& DitherState(DitherStates[Index]);
			if (!DitherState.IsValid())
			{
				return Index;
			}
		}

		return DitherStates.Emplace();
	}

	bool DitherStatesContain(TArray<FDitheredActorState>& DitherStates, AActor* InActor)
	{
		if (InActor)
		{
			for (auto const& State : DitherStates)
			{
				if (State.Actor == InActor)
				{
					return true;
				}
			}
		}

		return false;
	}

	FVector GetTraceSafeLocation(AActor* TargetActor, USpringArmComponent* SpringArm, FMinimalViewInfo const& InPOV)
	{
		FVector SafeLocation = TargetActor ? TargetActor->GetActorLocation() : FVector::Zero();
		if (TargetActor)
		{
			// Try to get spring arm component
			if (SpringArm)
			{
				SafeLocation = SpringArm->GetComponentLocation() + SpringArm->TargetOffset;
			}
			else if (UPrimitiveComponent const* PPActorRootComponent = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent()))
			{
				// Attempt at picking SafeLocation automatically, so we reduce camera translation when aiming.
				// Our camera is our reticle, so we want to preserve our aim and keep that as steady and smooth as possible.
				// Pick closest point on capsule to our aim line.
				FVector ClosestPointOnLineToCapsuleCenter;
				FMath::PointDistToLine(SafeLocation, InPOV.Rotation.Vector(), InPOV.Location, ClosestPointOnLineToCapsuleCenter);

				// Adjust Safe distance height to be same as aim line, but within capsule.
				float const PushInDistance = 0.f;
				float const MaxHalfHeight = TargetActor->GetSimpleCollisionHalfHeight() - PushInDistance;
				SafeLocation.Z = FMath::Clamp(ClosestPointOnLineToCapsuleCenter.Z, SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

				float DistanceSqr = 0.f;
				PPActorRootComponent->GetSquaredDistanceToCollision(ClosestPointOnLineToCapsuleCenter, DistanceSqr, SafeLocation);
				// Push back inside capsule to avoid initial penetration when doing line checks.
				SafeLocation += (SafeLocation - ClosestPointOnLineToCapsuleCenter).GetSafeNormal() * PushInDistance;
			}
		}
		return SafeLocation;
	}
}

void FDitheredActorState::StartDithering(AActor* InActor, EDitherType InDitherType)
{
	if (InActor)
	{
		Actor = InActor;
		CollisionTime = 0.f;
		bIsDitheringIn = false;
		bIsDitheringOut = false;
		DitherType = InDitherType;
	}
}

void FDitheredActorState::Invalidate()
{
	Actor = nullptr;
	CurrentOpacity = 1.f;
	CollisionTime = 0.f;
	bIsDitheringIn = false;
	bIsDitheringOut = false;
	DitherType = EDitherType::None;
}

void FDitheredActorState::ComputeOpacity(float DeltaTime, float DitherInSpeed, float DitherOutSpeed, float DitherMin)
{
	if (!IsValid())
	{
		return;
	}

	bool const bCanDither = !bIsDitheringOut && bIsDitheringIn;
	CurrentOpacity = FMath::FInterpTo(CurrentOpacity, bCanDither ? DitherMin : 1.f, DeltaTime, bIsDitheringOut ? DitherOutSpeed : DitherInSpeed);
}

URPGCameraDitheringModifier::URPGCameraDitheringModifier()
{
	Priority = 255; // You want this modifier to be the last one to be executed so that it always knows the final camera position.
}

void URPGCameraDitheringModifier::ResetDitheredActors_Implementation()
{
	for (auto& DitherState : DitheredActorStates)
	{
		DitherState.bIsDitheringIn = false;
		DitherState.bIsDitheringOut = true;
		DitherState.CurrentOpacity = 1.f;
		static float constexpr DeltaTime = 1 / 60.f;
		ApplyDithering(DeltaTime, DitherState);
		DitherState.Invalidate();
	}
}

bool URPGCameraDitheringModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	Super::ModifyCamera(DeltaTime, InOutPOV);
	if (CameraOwner && CameraOwner->PCOwner && CameraOwner->PCOwner->GetPawn())
	{
		FCollisionQueryParams QueryParams(TEXT("CameraDithering"));

		// Get All overlapped actors and LOS-blocking actors
		TArray<AActor*> ActorsToDither;

		if (DitheringSettings.bDitherOverlaps)
		{
			if (!DitheringSettings.bDitherOwner)
			{
				QueryParams.AddIgnoredActor(CameraOwner->PCOwner->GetPawn());
				if (AActor* PendingTarget = CameraOwner->PendingViewTarget.Target)
				{
					QueryParams.AddIgnoredActor(PendingTarget);
				}
			}

			TArray<FOverlapResult> OutOverlaps;
			GetWorld()->OverlapMultiByChannel(OutOverlaps, InOutPOV.Location, FQuat::Identity, DitheringSettings.DitherOverlapChannel, FCollisionShape::MakeSphere(DitheringSettings.SphereCollisionRadius), QueryParams);

#if ENABLE_DRAW_DEBUG
			if (GetWorld()->DebugDrawTraceTag == TEXT("CameraDithering"))
			{
				DrawDebugSphere(GetWorld(), InOutPOV.Location, DitheringSettings.SphereCollisionRadius, 32, FColor::Red);
			}
#endif

			for (auto const& Overlap : OutOverlaps)
			{
				// Ignore null actors
				if (!Overlap.GetActor())
				{
					continue;
				}

				TArray<AActor*> OverlapActors;
				OverlapActors.Add(Overlap.GetActor());

				if (DitheringSettings.bDitherAttachedComponents)
				{
					Overlap.GetActor()->GetAttachedActors(OverlapActors, false, true);
				}

				for (auto& Actor : OverlapActors)
				{
					// Ignore null actors or actors with IgnoreDitheringTag
					if (!Actor || Actor->ActorHasTag(DitheringSettings.IgnoreDitheringTag))
					{
						continue;
					}

					bool const bIsCollisionResponseCorrect = DitherHelpers::DoAnyComponentsOverlap(Actor, DitheringSettings.DitherOverlapChannel);
					if (!bIsCollisionResponseCorrect)
					{
						continue;
					}

					ActorsToDither.Add(Actor);

					// Start dithering actor if not done already
					if (!DitherHelpers::DitherStatesContain(DitheredActorStates, Actor))
					{
						QueryParams.AddIgnoredActor(Actor);
						int32 Index = DitherHelpers::FindInactiveDitherState(DitheredActorStates);
						DitheredActorStates[Index].StartDithering(Actor, EDitherType::OverlappingCamera);
					}
				}
			}
		}

		if (DitheringSettings.bDitherLineOfSight)
		{
			QueryParams.AddIgnoredActor(CameraOwner->PCOwner->GetPawn());
			if (AActor* PendingTarget = CameraOwner->PendingViewTarget.Target)
			{
				QueryParams.AddIgnoredActor(PendingTarget);
			}

			TArray<FOverlapResult> OutOverlaps;
			FVector const EndLocation = DitherHelpers::GetTraceSafeLocation(GetViewTarget(), SpringArm, InOutPOV);
			FVector const ToTarget = (EndLocation - InOutPOV.Location).GetSafeNormal();
			FVector const StartLocation = InOutPOV.Location + ToTarget * DitheringSettings.SphereCollisionRadius;
			FVector const Delta = EndLocation - StartLocation;
			FQuat const Rotation = FRotationMatrix::MakeFromX(Delta.GetSafeNormal()).ToQuat();
			FVector const BoxOrigin = Delta * 0.5f + StartLocation;

			FVector const BoxExtent = FVector(Delta.Size() * 0.495f, DitheringSettings.LOSProbeSize, DitheringSettings.LOSProbeSize);
			FCollisionShape const Box = FCollisionShape::MakeBox(BoxExtent);

			GetWorld()->OverlapMultiByChannel(OutOverlaps, BoxOrigin, Rotation, DitheringSettings.DitherLOSChannel, Box, QueryParams);

#if ENABLE_DRAW_DEBUG
			if (GetWorld()->DebugDrawTraceTag == TEXT("CameraDithering"))
			{
				DrawDebugBox(GetWorld(), BoxOrigin, BoxExtent, Rotation, FColor::Red);
			}
#endif

			for (auto const& Overlap : OutOverlaps)
			{
				// Ignore null actors
				if (!Overlap.GetActor())
				{
					continue;
				}

				TArray<AActor*> OverlapActors;
				OverlapActors.Add(Overlap.GetActor());

				if (DitheringSettings.bDitherAttachedComponents)
				{
					Overlap.GetActor()->GetAttachedActors(OverlapActors, false, true);
				}

				for (auto& Actor : OverlapActors)
				{
					// Ignore null actors or actors with IgnoreDitheringTag
					if (!Actor || Actor->ActorHasTag(DitheringSettings.IgnoreDitheringTag))
					{
						continue;
					}
					ActorsToDither.Add(Actor);

					// Start dithering actor if not done already
					if (!DitherHelpers::DitherStatesContain(DitheredActorStates, Actor))
					{
						int32 Index = DitherHelpers::FindInactiveDitherState(DitheredActorStates);
						DitheredActorStates[Index].StartDithering(Actor, EDitherType::BlockingLOS);
					}
				}
			}
		}

		// Update dithered state of actors
		for (auto& DitherState : DitheredActorStates)
		{
			if (!DitherState.IsValid())
			{
				continue;
			}

			// Set the opacity
			DitherState.ComputeOpacity(DeltaTime, DitheringSettings.DitherInSpeed, DitheringSettings.DitherOutSpeed, DitheringSettings.MaterialDitherMinimum);

			ApplyDithering(DeltaTime, DitherState);

#if ENABLE_DRAW_DEBUG
			RPGDebugDithering(DitherState, DeltaTime, DitheringSettings.MaterialDitherMinimum);
#endif

			// If the dithered actor isn't overlapped
			if (!ActorsToDither.Contains(DitherState.Actor))
			{
				// Stop and do not dither in
				DitherState.bIsDitheringIn = false;
				DitherState.CollisionTime = 0.f;
				DitherState.bIsDitheringOut = true;

				// If finished dithering out
				if (DitherState.CurrentOpacity >= 1.f)
				{
					DitherState.Invalidate();
				}
			}
			// Otherwise if the dithered actor is still overlapped
			else
			{
				// Do not dither out
				DitherState.bIsDitheringOut = false;
				if (DitherState.DitherType == EDitherType::OverlappingCamera || (DitherState.DitherType == EDitherType::BlockingLOS && DitherState.CollisionTime >= DitheringSettings.CollisionTimeThreshold))
				{
					DitherState.bIsDitheringIn = true;
				}
				DitherState.CollisionTime += DeltaTime;
			}
		}
	}

	return false;
}

void URPGCameraDitheringModifier::ApplyDithering(float DeltaTime, FDitheredActorState& DitherState)
{
	if (!DitherState.IsValid())
	{
		return;
	}

	if (DitheringSettings.bUpdateMaterialPlayerPosition && CameraOwner && CameraOwner->PCOwner && CameraOwner->PCOwner->GetPawn() && DitheringMPC && GetWorld())
	{
		if (!DitheringMPCInstance)
		{
			DitheringMPCInstance = GetWorld()->GetParameterCollectionInstance(DitheringMPC.Get());
		}
		if (DitheringMPCInstance)
		{
			DitheringMPCInstance->SetVectorParameterValue(DitheringSettings.MaterialPlayerPositionParameterName, CameraOwner->PCOwner->GetPawn()->GetActorLocation());
		}
	}

	// Get all mesh components
	TArray<UMeshComponent*> Meshes;
	DitherState.Actor->GetComponents(Meshes, DitheringSettings.bDitherChildActors);

	for (auto& Mesh : Meshes)
	{
		if (!Mesh)
		{
			continue;
		}

		// Get all materials
		const TArray<UMaterialInterface*> Materials = Mesh->GetMaterials();
		for (auto& Material : Materials)
		{
			if (!Material)
			{
				continue;
			}

			// Get all float parameters
			TArray<FMaterialParameterInfo> ScalarParams;
			TArray<FGuid> ScalarParamIds;
			Material->GetAllScalarParameterInfo(ScalarParams, ScalarParamIds);

			// Set the params that have the same name as MaterialOpacityParameterName
			for (auto& ScalarParam : ScalarParams)
			{
				if (ScalarParam.Name != DitheringSettings.MaterialOpacityParameterName)
				{
					continue;
				}

				Mesh->SetScalarParameterValueOnMaterials(DitheringSettings.MaterialOpacityParameterName, DitherState.CurrentOpacity);
			}
		}
	}
}

#if ENABLE_DRAW_DEBUG
void URPGCameraDitheringModifier::RPGDebugDithering(FDitheredActorState& DitherState, float DeltaTime, float DitherMin)
{
	if (bDebug && DitherState.IsValid() && GEngine)
	{
		const FString EaseInString = DitherState.bIsDitheringIn ? FString::Printf(TEXT(" - Dithering In")) : TEXT("");
		const FString EaseOutString = DitherState.bIsDitheringOut ? FString::Printf(TEXT(" - Dithering Out")) : TEXT("");
		const FString DebugText = FString::Printf(TEXT("RPGCameraDitheringModifier: %s - Type: %s - Opacity: %f - Elapsed: %f%s%s")
			, *GetNameSafe(DitherState.Actor), *UEnum::GetDisplayValueAsText(DitherState.DitherType).ToString(), DitherState.CurrentOpacity, DitherState.CollisionTime, *EaseInString, *EaseOutString);

		GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Silver, DebugText);
	}
}
#endif