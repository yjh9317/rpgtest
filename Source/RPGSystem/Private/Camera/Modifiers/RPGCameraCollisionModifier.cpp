// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/RPGCameraCollisionModifier.h"

#include "Camera/RPGPlayerCameraManager.h"
#include "GameFramework/CameraBlockingVolume.h"
#include "GameFramework/SpringArmComponent.h"


namespace CollisionHelpers
{
#if ENABLE_DRAW_DEBUG
	FColor const DebugColor = FColor(150, 150, 200);
#endif
}

URPGCameraCollisionModifier::URPGCameraCollisionModifier()
{
	Priority = 134;
	bExclusive = true;
}

bool URPGCameraCollisionModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	Super::ModifyCamera(DeltaTime, InOutPOV);

	if (!bPlayDuringCameraAnimations)
	{
		if (RPGCameraManager && RPGCameraManager->IsPlayingAnyCameraAnimation())
		{
			return false;
		}
	}

	if (SpringArm)
	{
		// Don't do collision tests is spring arm is taking care of them.
		if (SpringArm->bDoCollisionTest && CollisionSettings.bPreventPenetration)
		{
#if ENABLE_DRAW_DEBUG
			if (GEngine != nullptr)
			{
				FString const CameraManagerName = GetNameSafe(CameraOwner);
				FString const ActorName = GetNameSafe(GetViewTarget());
				FString const SpringName = GetNameSafe(SpringArm);
				FString const DebugText = FString::Printf(TEXT("Actor %s has Collision Checks enabled on SpringArm %s but Camera Manager %s has Camera Modifier RPGCameraCollisionModifier. This is not allowed so the modifier will be aborted.")
					, *ActorName, *SpringName, *CameraManagerName);
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, DebugText);
			}
#endif
			return false;
		}
	}

	// Do collision checks
	UpdatePreventPenetration(DeltaTime, InOutPOV);

	return false;
}

void URPGCameraCollisionModifier::UpdatePreventPenetration(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
	if (!CollisionSettings.bPreventPenetration)
	{
		return;
	}

	FVector const SafeLocation = GetTraceSafeLocation(InOutPOV);

	// Then aim line to desired camera position
	bool const bSingleRayPenetrationCheck = !CollisionSettings.bDoPredictiveAvoidance || !SingleRayOverriders.IsEmpty();

	if (bSingleRayPenetrationCheck)
	{
#if ENABLE_DRAW_DEBUG
		if (bDebug && GEngine != nullptr)
		{
			FString const& DebugText = FString::Printf(TEXT("RPGCameraCollisionModifier: Single Ray mode enabled."));
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, CollisionHelpers::DebugColor, DebugText);
		}
#endif
	}

	if (AActor* TargetActor = GetViewTarget())
	{
		PreventCameraPenetration(*TargetActor, SafeLocation, InOutPOV.Location, DeltaTime, AimLineToDesiredPosBlockedPct, bSingleRayPenetrationCheck);
	}
}

FVector URPGCameraCollisionModifier::GetTraceSafeLocation(FMinimalViewInfo const& InPOV)
{
	AActor* TargetActor = GetViewTarget();
	FVector SafeLocation = TargetActor ? TargetActor->GetActorLocation() : FVector::Zero();
	if (TargetActor)
	{
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
			float const PushInDistance = CollisionSettings.PenetrationAvoidanceFeelers[0].ProbeRadius;
			float const MaxHalfHeight = TargetActor->GetSimpleCollisionHalfHeight() - PushInDistance;
			SafeLocation.Z = FMath::Clamp(ClosestPointOnLineToCapsuleCenter.Z, SafeLocation.Z - MaxHalfHeight, SafeLocation.Z + MaxHalfHeight);

			float DistanceSqr = 0.f;
			PPActorRootComponent->GetSquaredDistanceToCollision(ClosestPointOnLineToCapsuleCenter, DistanceSqr, SafeLocation);
			// Push back inside capsule to avoid initial penetration when doing line checks.
			if (CollisionSettings.PenetrationAvoidanceFeelers.Num() > 0)
			{
				SafeLocation += (SafeLocation - ClosestPointOnLineToCapsuleCenter).GetSafeNormal() * PushInDistance;
			}
		}
	}
	return SafeLocation;
}

void URPGCameraCollisionModifier::PreventCameraPenetration(AActor const& ViewTarget, FVector const& SafeLoc, FVector& OutCameraLoc, float const& DeltaTime, float& OutDistBlockedPct, bool bSingleRayOnly)
{
#if ENABLE_DRAW_DEBUG
	DebugActorsHitDuringCameraPenetration.Reset();
#endif

	FVector const& CameraLoc = OutCameraLoc;
	float HardBlockedPct = OutDistBlockedPct;
	float SoftBlockedPct = OutDistBlockedPct;

	FVector BaseRay = CameraLoc - SafeLoc;
	FRotationMatrix BaseRayMatrix(BaseRay.Rotation());
	FVector BaseRayLocalUp, BaseRayLocalFwd, BaseRayLocalRight;

	BaseRayMatrix.GetScaledAxes(BaseRayLocalFwd, BaseRayLocalRight, BaseRayLocalUp);

	float DistBlockedPctThisFrame = 1.f;

	int32 const NumRaysToShoot = bSingleRayOnly ? FMath::Min(1, CollisionSettings.PenetrationAvoidanceFeelers.Num()) : CollisionSettings.PenetrationAvoidanceFeelers.Num();

	CollidingFeelers = TStaticBitArray<128U>();

	FCollisionQueryParams SphereParams(SCENE_QUERY_STAT(CameraPen), false, nullptr/*PlayerCamera*/);

	SphereParams.AddIgnoredActor(&ViewTarget);

	FCollisionShape SphereShape = FCollisionShape::MakeSphere(0.f);
	UWorld* World = GetWorld();

	int32 NbrHits = 0;

	for (int32 RayIdx = 0; RayIdx < NumRaysToShoot; ++RayIdx)
	{
		FPenetrationAvoidanceFeeler& Feeler = CollisionSettings.PenetrationAvoidanceFeelers[RayIdx];
		FRotator AdjustmentRot = Feeler.AdjustmentRot;

		if (RayIdx == 0 && Feeler.AdjustmentRot != FRotator::ZeroRotator)
		{
#if ENABLE_DRAW_DEBUG
			if (GEngine != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, TEXT("RPGCameraCollisionModifier: First Penetration Avoidance Feeler should always have an adjustment roation equal to 0,0,0!."));
			}
#endif
			AdjustmentRot = FRotator::ZeroRotator;
		}

		// calc ray target
		FVector RayTarget;
		{
			// TO DO #GravityCompatibility
			FVector RotatedRay = BaseRay.RotateAngleAxis(AdjustmentRot.Yaw, BaseRayLocalUp);
			RotatedRay = RotatedRay.RotateAngleAxis(AdjustmentRot.Pitch, BaseRayLocalRight);
			RayTarget = SafeLoc + RotatedRay;
		}

		// Cast for world and pawn hits separately.  this is so we can safely ignore the camera's target pawn.
		bool const bForceSmallShape = bSingleRayOnly && !SingleRayOverriders.IsEmpty();
		SphereShape.Sphere.Radius = bForceSmallShape ? 2.f : Feeler.ProbeRadius;
		ECollisionChannel TraceChannel = ECC_Camera;

		// Do multi-line check to make sure the hits we throw out aren't masking real hits behind (these are important rays).
		// Passing camera as actor so that camerablockingvolumes know when it's the camera doing traces.
		FHitResult Hit;
		const bool bHit = World->SweepSingleByChannel(Hit, SafeLoc, RayTarget, FQuat::Identity, TraceChannel, SphereShape, SphereParams);
#if ENABLE_DRAW_DEBUG
		if (World->TimeSince(LastDrawDebugTime) < 1.f)
		{
			DrawDebugSphere(World, SafeLoc, SphereShape.Sphere.Radius, 8, FColor::Red);
			DrawDebugSphere(World, bHit ? Hit.Location : RayTarget, SphereShape.Sphere.Radius, 8, FColor::Red);
			DrawDebugLine(World, SafeLoc, bHit ? Hit.Location : RayTarget, FColor::Red);
		}
#endif // ENABLE_DRAW_DEBUG

		const AActor* HitActor = Hit.GetActor();

		if (bHit && HitActor)
		{
			bool bIgnoreHit = false;

			if (HitActor->ActorHasTag(CollisionSettings.IgnoreCameraCollisionTag))
			{
				bIgnoreHit = true;
				SphereParams.AddIgnoredActor(HitActor);
			}

			// Ignore CameraBlockingVolume hits that occur in front of the ViewTarget.
			if (!bIgnoreHit && HitActor->IsA<ACameraBlockingVolume>())
			{
				const FVector ViewTargetForwardXY = ViewTarget.GetActorForwardVector().GetSafeNormal2D();
				const FVector ViewTargetLocation = ViewTarget.GetActorLocation();
				const FVector HitOffset = Hit.Location - ViewTargetLocation;
				const FVector HitDirectionXY = HitOffset.GetSafeNormal2D();
				const float DotHitDirection = FVector::DotProduct(ViewTargetForwardXY, HitDirectionXY);
				if (DotHitDirection > 0.0f)
				{
					bIgnoreHit = true;
					// Ignore this CameraBlockingVolume on the remaining sweeps.
					SphereParams.AddIgnoredActor(HitActor);
				}
				else
				{
#if ENABLE_DRAW_DEBUG
					DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif
				}
			}

			if (!bIgnoreHit)
			{
				CollidingFeelers[RayIdx] = true;
				++NbrHits;

				float const Weight = Cast<APawn>(Hit.GetActor()) ? Feeler.PawnWeight : Feeler.WorldWeight;
				float NewBlockPct = Hit.Time;
				NewBlockPct += (1.f - NewBlockPct) * (1.f - Weight);

				// Recompute blocked pct taking into account pushout distance.
				NewBlockPct = (Hit.Location - SafeLoc).Size() / (RayTarget - SafeLoc).Size();
				DistBlockedPctThisFrame = FMath::Min(NewBlockPct, DistBlockedPctThisFrame);

#if ENABLE_DRAW_DEBUG
				DebugActorsHitDuringCameraPenetration.AddUnique(TObjectPtr<const AActor>(HitActor));
#endif
			}
		}

		if (RayIdx == 0)
		{
			// Don't interpolate toward this one, snap to it. THIS ASSUMES RAY 0 IS THE CENTER/MAIN RAY.
			HardBlockedPct = DistBlockedPctThisFrame;
		}
		else
		{
			SoftBlockedPct = DistBlockedPctThisFrame;
		}
	}

#if ENABLE_DRAW_DEBUG
	if (NbrHits > 0)
	{
		if (bDebug && GEngine != nullptr)
		{

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
			FString const& DebugText = FString::Printf(TEXT("RPGCameraCollisionModifier: %d feeler%hs colliding."), NbrHits, NbrHits > 1 ? "s" : "");
#else
			FString const& DebugText = FString::Printf(TEXT("RPGCameraCollisionModifier: %d feeler%s colliding."), NbrHits, NbrHits > 1 ? "s" : "");
#endif
			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, CollisionHelpers::DebugColor, DebugText);
		}
	}
#endif

	if (OutDistBlockedPct < DistBlockedPctThisFrame)
	{
		// interpolate smoothly out
		if (CollisionSettings.PenetrationBlendOutTime > DeltaTime)
		{
			OutDistBlockedPct = OutDistBlockedPct + DeltaTime / CollisionSettings.PenetrationBlendOutTime * (DistBlockedPctThisFrame - OutDistBlockedPct);
		}
		else
		{
			OutDistBlockedPct = DistBlockedPctThisFrame;
		}
	}
	else
	{
		if (OutDistBlockedPct > HardBlockedPct)
		{
			OutDistBlockedPct = HardBlockedPct;
		}
		else if (OutDistBlockedPct > SoftBlockedPct)
		{
			// interpolate smoothly in
			if (CollisionSettings.PenetrationBlendInTime > DeltaTime)
			{
				OutDistBlockedPct = OutDistBlockedPct - DeltaTime / CollisionSettings.PenetrationBlendInTime * (OutDistBlockedPct - SoftBlockedPct);
			}
			else
			{
				OutDistBlockedPct = SoftBlockedPct;
			}
		}
	}

	OutDistBlockedPct = FMath::Clamp<float>(OutDistBlockedPct, 0.f, 1.f);
	if (OutDistBlockedPct < (1.f - ZERO_ANIMWEIGHT_THRESH))
	{
		OutCameraLoc = SafeLoc + (CameraLoc - SafeLoc) * OutDistBlockedPct;
	}
}