// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Component/RPGSpringArmComponentBase.h"
#include "PhysicsEngine/PhysicsSettings.h"


void URPGSpringArmComponentBase::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void URPGSpringArmComponentBase::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime)
{
	FRotator DesiredRot = GetTargetRotation();

	// If our viewtarget is simulating using physics, we may need to clamp deltatime
	if (bClampToMaxPhysicsDeltaTime)
	{
		// Use the same max timestep cap as the physics system to avoid camera jitter when the viewtarget simulates less time than the camera
		DeltaTime = FMath::Min(DeltaTime, UPhysicsSettings::Get()->MaxPhysicsDeltaTime);
	}

	// Apply 'lag' to rotation if desired
	if (bDoRotationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.f)
		{
			const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
			FRotator LerpTarget = PreviousDesiredRot;
			float RemainingTime = DeltaTime;
			while (RemainingTime > UE_KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmRotStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(LerpTarget), LerpAmount, CameraRotationLagSpeed));
				PreviousDesiredRot = DesiredRot;
			}
		}
		else
		{
			DesiredRot = FRotator(FMath::QInterpTo(FQuat(PreviousDesiredRot), FQuat(DesiredRot), DeltaTime, CameraRotationLagSpeed));
		}
	}
	PreviousDesiredRot = DesiredRot;

	// Get the spring arm 'origin', the target we want to look at
	FVector const OriginalArmOrigin = GetComponentLocation() + TargetOffset;
	FVector const ArmOriginWithSocketOffset = OriginalArmOrigin + FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	// Get the spring arm 'origin', the target we want to look at
	FVector ArmOrigin = OriginalArmOrigin;
	// We lag the target, not the actual camera position, so rotating the camera around does not have lag
	FVector DesiredLoc = ArmOrigin;
	if (bDoLocationLag)
	{
		if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
		{
			const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
			FVector LerpTarget = PreviousDesiredLoc;

			float RemainingTime = DeltaTime;
			while (RemainingTime > UE_KINDA_SMALL_NUMBER)
			{
				const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
				LerpTarget += ArmMovementStep * LerpAmount;
				RemainingTime -= LerpAmount;

				DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
				PreviousDesiredLoc = DesiredLoc;
			}
		}
		else
		{
			DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
		}

		// Clamp distance if requested
		bool bClampedDist = false;
		if (CameraLagMaxDistance > 0.f)
		{
			const FVector FromOrigin = DesiredLoc - ArmOrigin;
			if (FromOrigin.SizeSquared() > FMath::Square(CameraLagMaxDistance))
			{
				DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
				bClampedDist = true;
			}
		}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (bDrawDebugLagMarkers)
		{
			DrawDebugSphere(GetWorld(), ArmOrigin, 5.f, 8, FColor::Green);
			DrawDebugSphere(GetWorld(), DesiredLoc, 5.f, 8, FColor::Yellow);

			const FVector ToOrigin = ArmOrigin - DesiredLoc;
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc, DesiredLoc + ToOrigin * 0.5f, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
			DrawDebugDirectionalArrow(GetWorld(), DesiredLoc + ToOrigin * 0.5f, ArmOrigin, 7.5f, bClampedDist ? FColor::Red : FColor::Green);
		}
#endif
	}

	PreviousArmOrigin = ArmOrigin;
	PreviousDesiredLoc = DesiredLoc;

	// Now offset camera position back along our rotation
	DesiredLoc -= DesiredRot.Vector() * TargetArmLength;
	// Add socket offset in local space
	DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

	// Do a sweep to ensure we are not penetrating the world
	FVector ResultLoc;
	if (bDoTrace && (TargetArmLength != 0.0f))
	{
		bIsCameraFixed = true;
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());

		FHitResult Result;
		// It's important to use the OriginalArmOrigin for the traces otherwise jumping over obstacles will trigger collisions (because ArmOrigin stays low).
		GetWorld()->SweepSingleByChannel(Result, OriginalArmOrigin, DesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);

		UnfixedCameraPosition = DesiredLoc;

		ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);

		if (ResultLoc == DesiredLoc)
		{
			bIsCameraFixed = false;
		}
	}
	else
	{
		ResultLoc = DesiredLoc;
		bIsCameraFixed = false;
		UnfixedCameraPosition = ResultLoc;
	}

	// Form a transform for new world transform for camera
	FTransform WorldCamTM(DesiredRot, ResultLoc);
	// Convert to relative to component
	FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentTransform());

	// Update socket location/rotation
	RelativeSocketLocation = RelCamTM.GetLocation();
	RelativeSocketRotation = RelCamTM.GetRotation();

	UpdateChildTransforms();
}

FVector URPGSpringArmComponentBase::BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime)
{
	if (!bDoCollisionTest || !CameraCollisionSettings.bPreventPenetration)
	{
		return DesiredArmLocation;
	}

	const bool bShouldComplexTrace = CameraCollisionSettings.bDoPredictiveAvoidance && CameraCollisionSettings.PenetrationAvoidanceFeelers.Num() > 0 && IsPlayerControlled();
	if (!bShouldComplexTrace)
	{
		return Super::BlendLocations(DesiredArmLocation, TraceHitLocation, bHitSomething, DeltaTime);
	}

	FVector SafeLoc = FVector::ZeroVector;
	if (bMaintainFramingDuringCollisions)
	{
		FRotator DesiredRot = GetTargetRotation();
		const FVector OriginalArmOrigin = GetComponentLocation() + TargetOffset;
		SafeLoc = OriginalArmOrigin + FRotationMatrix(DesiredRot).TransformVector(SocketOffset);
	}
	else
	{
		SafeLoc = PreviousArmOrigin;
	}

	const FVector CameraLoc = DesiredArmLocation;
	const FVector BaseRay = CameraLoc - SafeLoc;

	if (BaseRay.IsNearlyZero())
	{
		return DesiredArmLocation;
	}

	const FRotationMatrix BaseMatrix(BaseRay.Rotation());
	const FVector Right = BaseMatrix.GetUnitAxis(EAxis::Y);
	const FVector Up = BaseMatrix.GetUnitAxis(EAxis::Z);

	float HardBlockedPct = DistBlockedPct;
	float SoftBlockedPct = DistBlockedPct;
	float BlockedThisFrame = 1.f;

	UWorld* World = GetWorld();
	int32 NbrHits = 0;
	const AActor* OwningActor = GetOwner();

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CameraPen), false, OwningActor);
	QueryParams.AddIgnoredActor(OwningActor);

#if WITH_EDITORONLY_DATA
	const int32 NewCapacity = CameraCollisionSettings.PenetrationAvoidanceFeelers.Num();
	const int32 OldCapacity = HitActors.Max();
	HitActors.Reset(FMath::Max(NewCapacity, OldCapacity)); // Reset array elements but keep allocation to max capacity.
#endif

	for (int32 i = 0; i < CameraCollisionSettings.PenetrationAvoidanceFeelers.Num(); ++i)
	{
		const FPenetrationAvoidanceFeeler& Feeler = CameraCollisionSettings.PenetrationAvoidanceFeelers[i];
		FRotator OffsetRot = Feeler.AdjustmentRot;

		if (i == 0 && Feeler.AdjustmentRot != FRotator::ZeroRotator)
		{
#if ENABLE_DRAW_DEBUG
			if (GEngine != nullptr)
			{
				GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, TEXT("DSA_SpringArmComponent: First Penetration Avoidance Feeler should always have an adjustment roation equal to 0,0,0!."));
			}
#endif
			OffsetRot = FRotator::ZeroRotator;
		}

		FVector RayTarget = BaseRay.RotateAngleAxis(OffsetRot.Yaw, Up).RotateAngleAxis(OffsetRot.Pitch, Right) + SafeLoc;

		FHitResult Hit;
		FCollisionShape Shape = FCollisionShape::MakeSphere(Feeler.ProbeRadius);
		bool bHit = World->SweepSingleByChannel(Hit, SafeLoc, RayTarget, FQuat::Identity, ProbeChannel, Shape, QueryParams);

		if (bHit && Hit.GetActor())
		{
			if (Hit.GetActor()->ActorHasTag(CameraCollisionSettings.IgnoreCameraCollisionTag))
			{
				QueryParams.AddIgnoredActor(Hit.GetActor());
				continue;
			}

			++NbrHits;
			const float Weight = Hit.GetActor()->IsA<APawn>() ? Feeler.PawnWeight : Feeler.WorldWeight;

#if WITH_EDITORONLY_DATA
			HitActors.AddUnique(Hit.GetActor());
#endif

			float NewBlockPct = Hit.Time + (1.f - Hit.Time) * (1.f - Weight);
			NewBlockPct = (Hit.Location - SafeLoc).Size() / (RayTarget - SafeLoc).Size();

			BlockedThisFrame = FMath::Min(NewBlockPct, BlockedThisFrame);
			if (i == 0)
				HardBlockedPct = BlockedThisFrame;
			else
				SoftBlockedPct = BlockedThisFrame;
		}
	}

#if ENABLE_DRAW_DEBUG
	if (NbrHits > 0)
	{
		if (bPrintCollisionDebug && GEngine != nullptr)
		{
			if (bPrintHitActors)
			{
#if WITH_EDITORONLY_DATA
				for (int i = HitActors.Num() - 1; i >= 0; --i)
				{
					if (AActor* HitActor = HitActors[i])
					{
						FString const DebugText = FString::Printf(TEXT("DSA_SpringArmComponent: Colliding with %s."), *GetNameSafe(HitActor));
						GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor(150, 150, 200), DebugText);
					}
				}
#endif
			}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 6
			FString const DebugText = FString::Printf(TEXT("DSA_SpringArmComponent: %d feeler%hs colliding."), NbrHits, NbrHits > 1 ? "s" : "");
#else
			FString const DebugText = FString::Printf(TEXT("DSA_SpringArmComponent: %d feeler%s colliding."), NbrHits, NbrHits > 1 ? "s" : "");
#endif
			GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor(150, 150, 200), DebugText);
		}
	}
#endif

	if (DistBlockedPct < BlockedThisFrame)
	{
		float BlendOutTime = FMath::Max(CameraCollisionSettings.PenetrationBlendOutTime, UE_KINDA_SMALL_NUMBER);
		DistBlockedPct += DeltaTime / BlendOutTime * (BlockedThisFrame - DistBlockedPct);
	}
	else if (DistBlockedPct > HardBlockedPct)
	{
		DistBlockedPct = HardBlockedPct;
	}
	else if (DistBlockedPct > SoftBlockedPct)
	{
		float BlendInTime = FMath::Max(CameraCollisionSettings.PenetrationBlendInTime, UE_KINDA_SMALL_NUMBER);
		DistBlockedPct -= DeltaTime / BlendInTime * (DistBlockedPct - SoftBlockedPct);
	}

	DistBlockedPct = FMath::Clamp(DistBlockedPct, 0.f, 1.f);
	return SafeLoc + (CameraLoc - SafeLoc) * DistBlockedPct;
}

bool URPGSpringArmComponentBase::IsPlayerControlled() const
{
	APawn* PawnOwner = Cast<APawn>(GetOwner());
	AController* Controller = PawnOwner ? PawnOwner->GetController() : nullptr;

	bool bPlayerControlled = (PawnOwner && Controller && PawnOwner->IsPlayerControlled());
	if (!bPlayerControlled)
	{
		if (GEngine)
		{
			if (APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld()))
			{
				if (APlayerCameraManager* PCM = PC->PlayerCameraManager)
				{
					bPlayerControlled = PCM->PendingViewTarget.Target == GetOwner() || (Controller != nullptr && PCM->PendingViewTarget.Target == Controller);
				}
			}
		}
	}
	return bPlayerControlled;
}