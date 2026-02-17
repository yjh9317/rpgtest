// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/RPGPlayerCameraManager.h"

#include "EngineUtils.h"
#include "LevelSequenceActor.h"
#include "LevelSequencePlayer.h"
#include "Camera/CameraComponent.h"
#include "Camera/RPGPawnInterface.h"
#include "Camera/Data/RPGCameraDataAsset.h"
#include "Camera/Methods/RPGFocusTargetMethod.h"
#include "Camera/Modifiers/RPGCameraModifier.h"
#include "Camera/Component/RPGSpringArmComponentBase.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/Modifiers/RPGPlayCameraAnimCallbackProxy.h"
#include "Engine/Canvas.h"
#include "Kismet/KismetMathLibrary.h"

#include "Camera/Modifiers/RPGCameraAnimationModifier.h"
#include "Camera/Modifiers/RPGCameraCollisionModifier.h"
#include "Camera/Modifiers/RPGCameraDitheringModifier.h"
#include "Camera/Modifiers/Child/FocusCameraModifier.h"
#include "Camera/Modifiers/Child/YawFollowCameraModifier.h"
#include "Camera/Modifiers/Child/PitchFollowModifier.h"
#include "Camera/Modifiers/Child/ArmOffsetModifier.h"
#include "Camera/Modifiers/Child/PitchToArmLengthAndFOVModifier.h"
#include "Camera/Modifiers/Child/AnglesConstraintModifier.h"
#include "Components/TimelineComponent.h"
#include "MovieSceneSequence.h"
#include "Camera/Modifiers/Child/ArmLagModifier.h"


TAutoConsoleVariable<bool> GShowCameraManagerModifiersCVar(
	TEXT("ShowCameraModifiersDebug"),
	false,
	TEXT("Show information about the currently active camera modifiers and their priorities."));

namespace PlayerCameraHelpers
{
	UCameraComponent* GetSpringArmChildCamera(USpringArmComponent* SpringArm)
	{
		UCameraComponent* Camera = nullptr;
		if (SpringArm)
		{
			for (int32 i = 0; i < SpringArm->GetNumChildrenComponents(); ++i)
			{
				USceneComponent* Child = SpringArm->GetChildComponent(i);

				if (UCameraComponent* PotentialCamera = Cast<UCameraComponent>(Child))
				{
					Camera = PotentialCamera;
					break; // found it
				}
			}
		}
		return Camera;
	}
}

ARPGPlayerCameraManager::ARPGPlayerCameraManager()
{
	PrimaryActorTick.bCanEverTick = true;
	
	ResetControlRotationToTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ResetControlRotationToTimeline"));
	ArmLengthRangeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ArmLengthRangeTimeline"));
	FOVRangeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("FOVRangeTimeline"));
	ArmSocketOffsetTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ArmSocketOffsetTimeline"));
	ArmTargetOffsetTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("ArmTargetOffsetTimeline"));
	PitchRangeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("PitchRangeTimeline"));
	YawRangeTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("YawRangeTimeline"));
}

void ARPGPlayerCameraManager::InitializeFor(APlayerController* PC)
{
	Super::InitializeFor(PC);
	SetupModifierReferences();
	RefreshLevelSequences(); // TO DO, this might not work for open worlds if level sequences are loaded in runtime.
}

void ARPGPlayerCameraManager::PrePossess(APawn* NewPawn, URPGCameraDataAssetBase* NewCameraDA, bool bBlendSpringArmProperties, bool bMatchCameraRotation)
{
	if (!NewPawn)
	{
		return;
	}

	USpringArmComponent* NewSpringArm = NewPawn->FindComponentByClass<USpringArmComponent>();
	if (!NewSpringArm || !CameraArm)
	{
		return;
	}

	if (bBlendSpringArmProperties)
	{
		const bool bSkipChecks = !NewCameraDA || !GetCurrentCameraDataAsset();
		if (bSkipChecks || ((NewCameraDA->ArmLengthSettings.MinArmLength != GetCurrentCameraDataAsset()->ArmLengthSettings.MinArmLength
			|| NewCameraDA->ArmLengthSettings.MaxArmLength != GetCurrentCameraDataAsset()->ArmLengthSettings.MaxArmLength)
			&& NewCameraDA->ArmLengthSettings.ArmRangeBlendTime > 0.f))
		{
			NewSpringArm->TargetArmLength = CameraArm->TargetArmLength;
		}

		if (bSkipChecks || (NewCameraDA->ArmOffsetSettings.ArmSocketOffset != GetCurrentCameraDataAsset()->ArmOffsetSettings.ArmSocketOffset
			&& NewCameraDA->ArmOffsetSettings.ArmSocketOffsetBlendTime > 0.f))
		{
			NewSpringArm->SocketOffset = CameraArm->SocketOffset;
		}

		if (bSkipChecks || (NewCameraDA->ArmOffsetSettings.ArmTargetOffset != GetCurrentCameraDataAsset()->ArmOffsetSettings.ArmTargetOffset
			&& NewCameraDA->ArmOffsetSettings.ArmTargetOffsetBlendTime > 0.f))
		{
			NewSpringArm->TargetOffset = CameraArm->TargetOffset;
		}

		if (bSkipChecks || ((NewCameraDA->FOVSettings.MinFOV != GetCurrentCameraDataAsset()->FOVSettings.MinFOV
			|| NewCameraDA->FOVSettings.MaxFOV != GetCurrentCameraDataAsset()->FOVSettings.MaxFOV)
			&& NewCameraDA->FOVSettings.FOVRangeBlendTime > 0.f))
		{
			if (UCameraComponent* NewCamera = PlayerCameraHelpers::GetSpringArmChildCamera(NewSpringArm))
			{
				NewCamera->SetFieldOfView(ViewTarget.POV.FOV);
			}
		}
	}
	else if (NewCameraDA)
	{
		const float PitchRatio = FMath::GetMappedRangeValueClamped(
			FVector2D(ViewPitchMin, ViewPitchMax), FVector2D(-1.f, 1.f),
			bMatchCameraRotation ? static_cast<float>(ViewTarget.POV.Rotation.Pitch) : 0.f);
		NewSpringArm->TargetArmLength = NewCameraDA->PitchToArmAndFOVCurveSettings.PitchToArmLengthCurve ?
			FMath::GetMappedRangeValueClamped(FVector2D(-1.f, 1.f), FVector2D(NewCameraDA->ArmLengthSettings.MinArmLength, NewCameraDA->ArmLengthSettings.MaxArmLength), NewCameraDA->PitchToArmAndFOVCurveSettings.PitchToArmLengthCurve->GetFloatValue(PitchRatio))
			: NewCameraDA->ArmLengthSettings.MinArmLength;
		NewSpringArm->SocketOffset = NewCameraDA->ArmOffsetSettings.ArmSocketOffset;
		NewSpringArm->TargetOffset = NewCameraDA->ArmOffsetSettings.ArmTargetOffset;
		if (UCameraComponent* NewCamera = PlayerCameraHelpers::GetSpringArmChildCamera(NewSpringArm))
		{
			NewCamera->SetFieldOfView(NewCameraDA->PitchToArmAndFOVCurveSettings.PitchToFOVCurve ?
				FMath::GetMappedRangeValueClamped(FVector2D(-1.f, 1.f), FVector2D(NewCameraDA->FOVSettings.MinFOV, NewCameraDA->FOVSettings.MaxFOV), NewCameraDA->PitchToArmAndFOVCurveSettings.PitchToFOVCurve->GetFloatValue(PitchRatio))
				: NewCameraDA->FOVSettings.MinFOV);
		}
	}

	if (bMatchCameraRotation)
	{
		PendingPossessPayload.bMatchCameraRotation = true;
		PendingPossessPayload.PendingControlRotation = GetOwningPlayerController()->GetControlRotation();
	}
	PendingPossessPayload.PendingCameraDA = NewCameraDA;
	PendingPossessPayload.bBlendCameraProperties = bBlendSpringArmProperties;
}

void ARPGPlayerCameraManager::PostPossess(bool bReplaceCurrentCameraDA)
{
	if (PendingPossessPayload.PendingCameraDA)
	{
		URPGCameraDataAssetBase* CurrentHead = GetCurrentCameraDataAsset();
		PushCameraData_Internal(PendingPossessPayload.PendingCameraDA, PendingPossessPayload.bBlendCameraProperties);
		if (bReplaceCurrentCameraDA && CurrentHead)
		{
			PopCameraData(CurrentHead);
		}
	}
	if (PendingPossessPayload.bMatchCameraRotation)
	{
		GetOwningPlayerController()->SetControlRotation(PendingPossessPayload.PendingControlRotation);
	}
	PendingPossessPayload = ARPGPlayerCameraManager::PossessPayload();
}

void ARPGPlayerCameraManager::RefreshLevelSequences()
{
	// This resets the array and gets all actors of class.
	QUICK_SCOPE_CYCLE_COUNTER(ARPGPlayerCameraManager_RefreshLevelSequences);
	LevelSequences.Reset();

	for (TActorIterator<ALevelSequenceActor> It(GetWorld()); It; ++It)
	{
		ALevelSequenceActor* LevelSequence = *It;
		LevelSequences.Add(LevelSequence);

		LevelSequence->GetSequencePlayer()->OnPlay.AddDynamic(this, &ARPGPlayerCameraManager::OnLevelSequenceStarted);
		LevelSequence->GetSequencePlayer()->OnPlayReverse.AddDynamic(this, &ARPGPlayerCameraManager::OnLevelSequenceStarted);
		LevelSequence->GetSequencePlayer()->OnStop.AddDynamic(this, &ARPGPlayerCameraManager::OnLevelSequenceEnded);
		LevelSequence->GetSequencePlayer()->OnPause.AddDynamic(this, &ARPGPlayerCameraManager::OnLevelSequencePaused);
	}
}

void ARPGPlayerCameraManager::OnLevelSequenceStarted()
{
	if (NbrActivePausedLevelSequences > 0) --NbrActivePausedLevelSequences;
	++NbrActiveLevelSequences;
	DoForEachRPGModifier(&URPGCameraModifier::OnAnyLevelSequenceStarted);
}

void ARPGPlayerCameraManager::OnLevelSequencePaused()
{
	++NbrActivePausedLevelSequences;
	--NbrActiveLevelSequences;
	ensure(NbrActiveLevelSequences >= 0);
}

void ARPGPlayerCameraManager::OnLevelSequenceEnded()
{
	--NbrActiveLevelSequences;
	ensure(NbrActiveLevelSequences >= 0);
	DoForEachRPGModifier(&URPGCameraModifier::OnAnyLevelSequenceEnded);
}



void ARPGPlayerCameraManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateInternalVariables(DeltaTime);
}

bool ARPGPlayerCameraManager::IsPlayingAnyCameraAnimation() const
{
	if (URPGCameraAnimationModifier const* CameraAnimModifier = FindCameraModifierOfType<URPGCameraAnimationModifier>())
	{
		return CameraAnimModifier && CameraAnimModifier->IsAnyCameraAnimationSequence();
	}
	return false;
}

void ARPGPlayerCameraManager::PlayCameraAnimation(UCameraAnimationSequence* CameraSequence, FRPGCameraAnimationParams const& Params, bool bInterruptOthers, bool bDoCollisionChecks)
{
	if (URPGCameraAnimationModifier* CameraAnimModifier = FindCameraModifierOfType<URPGCameraAnimationModifier>())
	{
		CameraAnimModifier->PlaySingleCameraAnimation(CameraSequence, static_cast<FCameraAnimationParams>(Params), Params.ResetType, bInterruptOthers, bDoCollisionChecks);
	}
}

void ARPGPlayerCameraManager::SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
	auto OldPendingTarget = PendingViewTarget.Target;
	auto OldTarget = ViewTarget.Target;

	Super::SetViewTarget(NewViewTarget, TransitionParams);

	if (!OwnerPawn)
	{
		return;
	}

	bool const bAssignedNewTarget = ViewTarget.Target != OldTarget;
	bool const bBlendingToNewTarget = PendingViewTarget.Target != OldPendingTarget;
	if (bAssignedNewTarget || bBlendingToNewTarget)
	{
		bool const bWasImmediate = bAssignedNewTarget && !bBlendingToNewTarget;
		bool bNewTargetIsOwner = false;

		// If old character has been unpossessed, then our new target is the owner!
		// Or, if the view target is the controller, we are doing seamless travel which does not unpossess the pawn.
		if (bWasImmediate && (OwnerPawn->GetController() == nullptr || OwnerPawn->GetController() == ViewTarget.Target))
		{
			bNewTargetIsOwner = true;
		}
		else
		{
			bNewTargetIsOwner = bWasImmediate ? ViewTarget.Target == OwnerPawn : PendingViewTarget.Target == OwnerPawn;
		}

		DoForEachRPGModifier([bNewTargetIsOwner, bWasImmediate](URPGCameraModifier* RPGModifier)
		{
			RPGModifier->OnSetViewTarget(bWasImmediate, bNewTargetIsOwner);
		});
	}
}

UCameraModifier* ARPGPlayerCameraManager::FindCameraModifierOfClass(TSubclassOf<UCameraModifier> ModifierClass, bool bIncludeInherited)
{
	for (UCameraModifier* Mod : ModifierList)
	{
		if (bIncludeInherited)
		{
			if (Mod->GetClass()->IsChildOf(ModifierClass))
			{
				return Mod;
			}
		}
		else
		{
			if (Mod->GetClass() == ModifierClass)
			{
				return Mod;
			}
		}
	}

	return nullptr;
}

UCameraModifier const* ARPGPlayerCameraManager::FindCameraModifierOfClass(TSubclassOf<UCameraModifier> ModifierClass, bool bIncludeInherited) const
{
	for (UCameraModifier* Mod : ModifierList)
	{
		if (bIncludeInherited)
		{
			if (Mod->GetClass()->IsChildOf(ModifierClass))
			{
				return Mod;
			}
		}
		else
		{
			if (Mod->GetClass() == ModifierClass)
			{
				return Mod;
			}
		}
	}

	return nullptr;
}

void ARPGPlayerCameraManager::ToggleRPGCameraModifiers(bool const bEnabled, bool const bImmediate)
{
	DoForEachRPGModifier([bEnabled, bImmediate](URPGCameraModifier* RPGModifier)
	{
		if (bEnabled)
		{
			RPGModifier->EnableModifier();
		}
		else
		{
			RPGModifier->DisableModifier(bImmediate);
		}
	});
}

void ARPGPlayerCameraManager::ToggleCameraModifiers(bool const bEnabled, bool const bImmediate)
{
	for (int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ModifierIdx++)
	{
		if (ModifierList[ModifierIdx] != nullptr)
		{
			if (bEnabled)
			{
				ModifierList[ModifierIdx]->EnableModifier();
			}
			else
			{
				ModifierList[ModifierIdx]->DisableModifier(bImmediate);
			}
		}
	}
}

void ARPGPlayerCameraManager::ToggleAllRPGModifiersDebug(bool const bEnabled)
{
	DoForEachRPGModifier([bEnabled](URPGCameraModifier* RPGModifier)
	{
		if (!RPGModifier->IsDisabled())
		{
			RPGModifier->bDebug = bEnabled;
		}
	});
}

void ARPGPlayerCameraManager::ToggleAllModifiersDebug(bool const bEnabled)
{
	for (int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ModifierIdx++)
	{
		if (ModifierList[ModifierIdx] != nullptr && !ModifierList[ModifierIdx]->IsDisabled())
		{
			ModifierList[ModifierIdx]->bDebug = bEnabled;
		}
	}
}

void ARPGPlayerCameraManager::PushCameraData_Internal(URPGCameraDataAssetBase* CameraDA, bool bBlendCameraProperties)
{
	CameraDataStack.Push(CameraDA);
	OnCameraDataStackChanged(CameraDA, bBlendCameraProperties);
}

void ARPGPlayerCameraManager::PushCameraData(URPGCameraDataAssetBase* CameraDA)
{
	static constexpr bool bBlendCameraProperties = true;
	PushCameraData_Internal(CameraDA, bBlendCameraProperties);
}

void ARPGPlayerCameraManager::PopCameraDataHead()
{
	CameraDataStack.Pop();
	OnCameraDataStackChanged(CameraDataStack.IsEmpty() ? nullptr : CameraDataStack[CameraDataStack.Num() - 1]);
}

void ARPGPlayerCameraManager::PopCameraData(URPGCameraDataAssetBase* CameraDA)
{
	if (CameraDataStack.IsEmpty())
	{
		return;
	}

	if (GetCurrentCameraDataAsset() == CameraDA)
	{
		PopCameraDataHead();
	}

	CameraDataStack.Remove(CameraDA);
}

void ARPGPlayerCameraManager::OnCameraDataStackChanged_Implementation(URPGCameraDataAssetBase* CameraDA, bool bBlendSpringArmProperties)
{
	if (!CameraDA)
	{
		CameraDA = DefaultCameraData;
		if (!CameraDA)
		{
			return;
		}
	}

	SetupModifierReferences();

	MinArmLength = CameraDA->ArmLengthSettings.MinArmLength;
	MaxArmLength = CameraDA->ArmLengthSettings.MaxArmLength;
	MinFOV = CameraDA->FOVSettings.MinFOV;
	MaxFOV = CameraDA->FOVSettings.MaxFOV;
	ArmSocketOffset = CameraDA->ArmOffsetSettings.ArmSocketOffset;
	ArmTargetOffset = CameraDA->ArmOffsetSettings.ArmTargetOffset;

	const bool bShouldBlendProperties = bBlendSpringArmProperties && !CameraDataStack.IsEmpty();
	const float ArmLengthBlendTime = bShouldBlendProperties ? CameraDA->ArmLengthSettings.ArmRangeBlendTime : 0.f;
	const float FOVBlendTime = bShouldBlendProperties ? CameraDA->FOVSettings.FOVRangeBlendTime : 0.f;
	const float ArmSocketBlendTime = bShouldBlendProperties ? CameraDA->ArmOffsetSettings.ArmSocketOffsetBlendTime : 0.f;
	const float ArmTargetBlendTime = bShouldBlendProperties ? CameraDA->ArmOffsetSettings.ArmTargetOffsetBlendTime : 0.f;

	SetArmLengthRange(
		CameraDA->ArmLengthSettings.MinArmLength,
		CameraDA->ArmLengthSettings.MaxArmLength,
		ArmLengthBlendTime,
		CameraDA->ArmLengthSettings.ArmRangeBlendCurve);
	SetFOVRange(
		CameraDA->FOVSettings.MinFOV,
		CameraDA->FOVSettings.MaxFOV,
		FOVBlendTime,
		CameraDA->FOVSettings.FOVRangeBlendCurve);
	SetArmSocketOffset(
		CameraDA->ArmOffsetSettings.ArmSocketOffset,
		ArmSocketBlendTime,
		CameraDA->ArmOffsetSettings.ArmSocketOffsetBlendCurve);
	SetArmTargetOffset(
		CameraDA->ArmOffsetSettings.ArmTargetOffset,
		ArmTargetBlendTime,
		CameraDA->ArmOffsetSettings.ArmTargetOffsetBlendCurve);

	FCameraPitchConstraintSettings PitchSettings = CameraDA->PitchConstraints;
	FCameraYawConstraintSettings YawSettings = CameraDA->YawConstraints;
	if (!bShouldBlendProperties)
	{
		PitchSettings.PitchConstraintsBlendTime = 0.f;
		YawSettings.YawConstraintsBlendTime = 0.f;
	}
	SetPitchConstraints(PitchSettings);
	SetYawConstraints(YawSettings);

	SetPitchMovementFollowSettings(CameraDA->PitchFollowSettings);
	SetYawMovementFollowSettings(CameraDA->YawFollowSettings);
	SetCameraFocusSettings(CameraDA->FocusSettings);
	SetPitchToArmLengthAndFOVCurves(CameraDA->PitchToArmAndFOVCurveSettings);
	SetCameraDitheringSettings(CameraDA->DitheringSettings);
	SetCameraLagSettings(CameraDA->ArmLagSettings);

	if (CachedCollisionModifier)
	{
		CachedCollisionModifier->CollisionSettings = CameraDA->CollisionSettings;
	}

	if (URPGSpringArmComponentBase* RPGSpringArm = Cast<URPGSpringArmComponentBase>(CameraArm))
	{
		RPGSpringArm->SetCameraCollisionSettings(CameraDA->CollisionSettings);
	}

	if (!bShouldBlendProperties)
	{
		SetFOV(MinFOV);
		if (CameraArm)
		{
			CameraArm->TargetArmLength = MinArmLength;
			CameraArm->SocketOffset = ArmSocketOffset;
			CameraArm->TargetOffset = ArmTargetOffset;
		}
	}
}

void ARPGPlayerCameraManager::ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
	Super::ProcessViewRotation(DeltaTime, OutViewRotation, OutDeltaRot);
	if (PCOwner && ViewTarget.Target)
	{
		FRotator const ControlRotation = PCOwner->GetControlRotation();
		FRotator const OwnerRotation = ViewTarget.Target->GetActorRotation();
		FRotator InLocalControlRotation = ControlRotation - OwnerRotation;
		InLocalControlRotation.Normalize();

		float OutPitchTurnRate = PitchTurnRate;
		float OutYawTurnRate = YawTurnRate;

		// TO DO #GravityCompatibility
		ProcessTurnRate(DeltaTime, InLocalControlRotation, OutPitchTurnRate, OutYawTurnRate);

		PitchTurnRate = FMath::Clamp(OutPitchTurnRate, 0.f, 1.f);
		YawTurnRate = FMath::Clamp(OutYawTurnRate, 0.f, 1.f);
	}
}

void ARPGPlayerCameraManager::ProcessTurnRate(float DeltaTime, FRotator InLocalControlRotation, float& OutPitchTurnRate, float& OutYawTurnRate)
{
	DoForEachRPGModifierWithBreak([&](URPGCameraModifier* RPGModifier) -> bool
	{
		if (!RPGModifier->IsDisabled())
		{
			if (!RPGModifier->CanPlayDuringCameraAnimation())
			{
				if (IsPlayingAnyCameraAnimation())
				{
					return false;
				}
			}

			// TO DO #GravityCompatibility
			return RPGModifier->ProcessTurnRate(DeltaTime, InLocalControlRotation, PitchTurnRate, YawTurnRate, OutPitchTurnRate, OutYawTurnRate);
		}
		return false; // Don't break
	});
}

void ARPGPlayerCameraManager::UpdateInternalVariables_Implementation(float DeltaTime)
{
	AspectRatio = GetCameraCacheView().AspectRatio;
	HorizontalFOV = GetFOVAngle();
	ensureAlways(!FMath::IsNearlyZero(AspectRatio));
	VerticalFOV = FMath::RadiansToDegrees(2.f * FMath::Atan(FMath::Tan(FMath::DegreesToRadians(HorizontalFOV) * 0.5f) / AspectRatio));

	if (PCOwner && PCOwner->GetPawn())
	{
		APawn* NewOwnerPawn = PCOwner->GetPawn();
		if (!OwnerPawn || NewOwnerPawn != OwnerPawn)
		{
			// Either initialising reference, or we have possessed a new character.
			OwnerPawn = NewOwnerPawn;
			OwnerCharacter = PCOwner->GetPawn<ACharacter>();
			if (OwnerCharacter)
			{
				MovementComponent = OwnerCharacter->GetCharacterMovement();
			}
			CameraArm = OwnerPawn->FindComponentByClass<USpringArmComponent>();
			OriginalArmLength = CameraArm ? CameraArm->TargetArmLength : 0.f;

		}

		if (OwnerPawn)
		{
			MovementInput = GetMovementControlInput();
			bHasMovementInput = !MovementInput.IsZero();
			TimeSinceMovementInput = bHasMovementInput ? 0.f : TimeSinceMovementInput + DeltaTime;

			RotationInput = GetRotationInput();
			bHasRotationInput = !RotationInput.IsZero();
			TimeSinceRotationInput = bHasRotationInput ? 0.f : TimeSinceRotationInput + DeltaTime;
		}
	}
}

FRotator ARPGPlayerCameraManager::GetRotationInput_Implementation() const
{
	FRotator RotInput = FRotator::ZeroRotator;

	if (OwnerPawn && OwnerPawn->GetClass()->ImplementsInterface(URPGPawnInterface::StaticClass()))
	{
		RotInput = IRPGPawnInterface::Execute_GetRotationInput(OwnerPawn);
	}
	return RotInput;
}

FVector ARPGPlayerCameraManager::GetMovementControlInput_Implementation() const
{
	FVector MovInput = FVector::ZeroVector;

	if (OwnerPawn && OwnerPawn->GetClass()->ImplementsInterface(URPGPawnInterface::StaticClass()))
	{
		MovInput = IRPGPawnInterface::Execute_GetMovementInput(OwnerPawn);
	}
	return MovInput;
}

// Limit the view yaw in local space instead of world space.
void ARPGPlayerCameraManager::LimitViewYaw(FRotator& ViewRotation, float InViewYawMin, float InViewYawMax)
{
	// TO DO #GravityCompatibility
	if (PCOwner && PCOwner->GetPawn())
	{
		FRotator ActorRotation = PCOwner->GetPawn()->GetActorRotation();
		ViewRotation.Yaw = FMath::ClampAngle(ViewRotation.Yaw, ActorRotation.Yaw + InViewYawMin, ActorRotation.Yaw + InViewYawMax);
		ViewRotation.Yaw = FRotator::ClampAxis(ViewRotation.Yaw);
	}
}

void ARPGPlayerCameraManager::DrawRealDebugCamera(float Duration, FLinearColor CameraColor, float Thickness) const
{
#if ENABLE_DRAW_DEBUG
	::DrawDebugCamera(GetWorld(), ViewTarget.POV.Location, ViewTarget.POV.Rotation, ViewTarget.POV.FOV, 1.0f, CameraColor.ToFColor(true), false, Duration);
#endif
}

/** Draw a debug camera shape. */
void ARPGPlayerCameraManager::DrawGameDebugCamera(float Duration, bool bDrawCamera, FLinearColor CameraColor, bool bDrawSpringArm, FLinearColor SpringArmColor, float Thickness) const
{
#if ENABLE_DRAW_DEBUG

	if (bDrawCamera && CameraArm)
	{
		int32 const NbrComponents = CameraArm->GetNumChildrenComponents();
		for (int32 i = 0; i < NbrComponents; ++i)
		{
			if (USceneComponent* ChildComp = CameraArm->GetChildComponent(i))
			{
				if (UCameraComponent* CameraComp = Cast<UCameraComponent>(ChildComp))
				{
					::DrawDebugCamera(GetWorld(), CameraComp->GetComponentLocation(), CameraComp->GetComponentRotation(), ViewTarget.POV.FOV, 1.0f, CameraColor.ToFColor(true), false, Duration);

					if (bDrawSpringArm)
					{
						DrawDebugSpringArm(CameraComp->GetComponentLocation(), Duration, SpringArmColor, Thickness);
					}
					break;

				}

			}
		}
	}
#endif
}

void ARPGPlayerCameraManager::DrawDebugSpringArm(FVector const& CameraLocation, float Duration, FLinearColor SpringArmColor, float Thickness) const
{
#if ENABLE_DRAW_DEBUG
	if (CameraArm)
	{
		FVector const SafeLocation = CameraArm->GetComponentLocation() + CameraArm->TargetOffset;
		::DrawDebugLine(GetWorld(), CameraLocation, SafeLocation, SpringArmColor.ToFColor(true), false, Duration, 0, Thickness);
	}
#endif
}

void ARPGPlayerCameraManager::DoForEachRPGModifier(TFunction<void(URPGCameraModifier*)> const& Function)
{
	if (Function)
	{
		for (int32 ModifierIdx = 0; ModifierIdx < RPGModifiersList.Num(); ++ModifierIdx)
		{
			ensure(RPGModifiersList[ModifierIdx]);

			if (RPGModifiersList[ModifierIdx])
			{
				Function(RPGModifiersList[ModifierIdx]);
			}
		}
	}
}

void ARPGPlayerCameraManager::DoForEachRPGModifierWithBreak(TFunction<bool(URPGCameraModifier*)> const& Function)
{
	if (Function)
	{
		for (int32 ModifierIdx = 0; ModifierIdx < RPGModifiersList.Num(); ++ModifierIdx)
		{
			ensure(RPGModifiersList[ModifierIdx]);

			if (RPGModifiersList[ModifierIdx])
			{
				if (Function(RPGModifiersList[ModifierIdx]))
				{
					break;
				}
			}
		}
	}
}

void ARPGPlayerCameraManager::DisplayDebug(class UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
	const bool bShowModifierList = GShowCameraManagerModifiersCVar.GetValueOnGameThread();
	if (bShowModifierList)
	{
		for (int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ++ModifierIdx)
		{
			if (ModifierList[ModifierIdx] != nullptr)
			{
				Canvas->SetDrawColor(FColor::White);
				FString DebugString = FString::Printf(TEXT("RPG Modifier %d: %s - Priority %d"), ModifierIdx, *ModifierList[ModifierIdx]->GetName(), ModifierList[ModifierIdx]->Priority);
				Canvas->DrawText(GEngine->GetSmallFont(), DebugString, YL, YPos);
				YPos += YL;
			}
		}
	}
}

UCameraModifier* ARPGPlayerCameraManager::AddNewCameraModifier(TSubclassOf<UCameraModifier> ModifierClass)
{
	UCameraModifier* AddedModifier = Super::AddNewCameraModifier(ModifierClass);
	if (AddedModifier)
	{
		if (URPGCameraModifier* RPGModifier = Cast<URPGCameraModifier>(AddedModifier))
		{
			RPGModifiersList.Add(RPGModifier);
			if (URPGCameraAddOnModifier* RPGAddOnModifier = Cast<URPGCameraAddOnModifier>(AddedModifier))
			{
				RPGAddOnModifiersList.Add(RPGAddOnModifier);
			}
			SetupModifierReferences();
		}
	}
	return AddedModifier;
}

bool ARPGPlayerCameraManager::RemoveCameraModifier(UCameraModifier* ModifierToRemove)
{
	if (ModifierToRemove)
	{
		if (URPGCameraModifier* RPGModifierToRemove = Cast<URPGCameraModifier>(ModifierToRemove))
		{
			// Loop through each modifier in camera
			for (int32 ModifierIdx = 0; ModifierIdx < RPGModifiersList.Num(); ++ModifierIdx)
			{
				// If we found ourselves, remove ourselves from the list and return
				if (RPGModifiersList[ModifierIdx] == RPGModifierToRemove)
				{
					RPGModifiersList.RemoveAt(ModifierIdx, 1);
					break;
				}
			}

			if (URPGCameraAddOnModifier* RPGAddOnModifier = Cast<URPGCameraAddOnModifier>(ModifierToRemove))
			{
				// Loop through each modifier in camera
				for (int32 ModifierIdx = 0; ModifierIdx < RPGAddOnModifiersList.Num(); ++ModifierIdx)
				{
					// If we found ourselves, remove ourselves from the list and return
					if (RPGAddOnModifiersList[ModifierIdx] == RPGModifierToRemove)
					{
						RPGAddOnModifiersList.RemoveAt(ModifierIdx, 1);
						break;
					}
				}
			}
		}
	}
	const bool bResult = Super::RemoveCameraModifier(ModifierToRemove);
	SetupModifierReferences();
	return bResult;
}

FVector ARPGPlayerCameraManager::GetOwnerVelocity() const
{
	FVector Velocity = FVector::ZeroVector;
	if (MovementComponent)
	{
		Velocity = MovementComponent->Velocity;
	}
	else if (OwnerPawn && OwnerPawn->GetClass()->ImplementsInterface(URPGPawnMovementInterface::StaticClass()))
	{
		Velocity = IRPGPawnMovementInterface::Execute_GetOwnerVelocity(OwnerPawn);
	}
	return Velocity;
}

bool ARPGPlayerCameraManager::IsOwnerFalling() const
{
	bool bIsFalling = false;
	if (MovementComponent)
	{
		bIsFalling = MovementComponent->IsFalling();
	}
	else if (OwnerPawn && OwnerPawn->GetClass()->ImplementsInterface(URPGPawnMovementInterface::StaticClass()))
	{
		bIsFalling = IRPGPawnMovementInterface::Execute_IsOwnerFalling(OwnerPawn);
	}
	return bIsFalling;
}

bool ARPGPlayerCameraManager::IsOwnerStrafing() const
{
	bool bIsStrafing = false;
	if (MovementComponent && OwnerPawn)
	{
		bIsStrafing = OwnerPawn->bUseControllerRotationYaw || (MovementComponent->bUseControllerDesiredRotation && !MovementComponent->bOrientRotationToMovement);
	}
	else if (OwnerPawn)
	{
		if (OwnerPawn->GetClass()->ImplementsInterface(URPGPawnMovementInterface::StaticClass()))
		{
			bIsStrafing = IRPGPawnMovementInterface::Execute_IsOwnerStrafing(OwnerPawn);
		}
		else
		{
			bIsStrafing = OwnerPawn->bUseControllerRotationYaw;
		}
	}
	return bIsStrafing;
}

bool ARPGPlayerCameraManager::IsOwnerMovingOnGround() const
{
	bool bIsMovingOnGround = false;
	if (MovementComponent)
	{
		bIsMovingOnGround = MovementComponent->IsMovingOnGround();
	}
	else if (OwnerPawn && OwnerPawn->GetClass()->ImplementsInterface(URPGPawnMovementInterface::StaticClass()))
	{
		bIsMovingOnGround = IRPGPawnMovementInterface::Execute_IsOwnerMovingOnGround(OwnerPawn);
	}
	return bIsMovingOnGround;
}

void ARPGPlayerCameraManager::ComputeOwnerFloorDist(float SweepDistance, float CapsuleRadius, bool& bOutFloorExists, float& OutFloorDistance) const
{
	if (MovementComponent && OwnerCharacter)
	{
		CapsuleRadius = FMath::Max(CapsuleRadius, OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius());
		FFindFloorResult OutFloorResult;
		MovementComponent->ComputeFloorDist(OwnerPawn->GetActorLocation(), SweepDistance, SweepDistance, OutFloorResult, CapsuleRadius);
		bOutFloorExists = OutFloorResult.bBlockingHit;
		OutFloorDistance = bOutFloorExists ? OutFloorResult.FloorDist : 0.f;
	}
	else
	{
		FHitResult OutHit;
		bOutFloorExists = GetWorld()->SweepSingleByChannel(OutHit
			, OwnerPawn->GetActorLocation()
			, OwnerPawn->GetActorLocation() - SweepDistance * FVector::UpVector
			, FQuat::Identity
			, ECollisionChannel::ECC_Visibility
			, FCollisionShape::MakeSphere(CapsuleRadius));

		OutFloorDistance = bOutFloorExists ? OutHit.Distance : 0.f;
	}
}

void ARPGPlayerCameraManager::ComputeOwnerFloorNormal(float SweepDistance, float CapsuleRadius, bool& bOutFloorExists, FVector& OutFloorNormal) const
{
	if (MovementComponent && OwnerCharacter)
	{
		bOutFloorExists = MovementComponent->CurrentFloor.IsWalkableFloor();
		OutFloorNormal = MovementComponent->CurrentFloor.HitResult.ImpactNormal;
	}
	else
	{
		FHitResult OutHit;
		bOutFloorExists = GetWorld()->SweepSingleByChannel(OutHit
			, OwnerPawn->GetActorLocation()
			, OwnerPawn->GetActorLocation() - SweepDistance * FVector::UpVector
			, FQuat::Identity
			, ECollisionChannel::ECC_Visibility
			, FCollisionShape::MakeSphere(CapsuleRadius));

		bOutFloorExists = OutHit.bBlockingHit;
		OutFloorNormal = bOutFloorExists ? OutHit.ImpactNormal : FVector::ZeroVector;
	}
}

void ARPGPlayerCameraManager::ComputeOwnerSlopeAngle(float& OutSlopePitchDegrees, float& OutSlopeRollDegrees)
{
	bool bOutFloorExists = false;
	FVector OutFloorNormal = FVector::ZeroVector;
	ComputeOwnerFloorNormal(96.f, 64.f, bOutFloorExists, OutFloorNormal);
	UKismetMathLibrary::GetSlopeDegreeAngles(OwnerPawn->GetActorRightVector(), OutFloorNormal, OwnerPawn->GetActorUpVector(), OutSlopePitchDegrees, OutSlopeRollDegrees);
}

float ARPGPlayerCameraManager::ComputeOwnerLookAndMovementDot()
{
	if (IsOwnerStrafing())
	{
		return 1.f;
	}

	FVector const Velocity = GetOwnerVelocity();
	if (Velocity.IsNearlyZero())
	{
		return 0.f;
	}

	float const Dot = Velocity | OwnerPawn->GetControlRotation().Vector();
	return Dot;
}


void ARPGPlayerCameraManager::SetupModifierReferences()
{
	// =================================================================
	// Base Modifiers
	// =================================================================

	if (AnimationModifierClass)
	{
		CachedAnimationModifier = Cast<URPGCameraAnimationModifier>(FindCameraModifierOfClass(AnimationModifierClass, true));
	}

	if (CollisionModifierClass)
	{
		CachedCollisionModifier = Cast<URPGCameraCollisionModifier>(FindCameraModifierOfClass(CollisionModifierClass, true));
	}

	if (DitheringModifierClass)
	{
		CachedDitheringModifier = Cast<URPGCameraDitheringModifier>(FindCameraModifierOfClass(DitheringModifierClass, true));
	}

	// =================================================================
	// Child Modifiers
	// =================================================================

	if (FocusModifierClass)
	{
		CachedFocusModifier = Cast<UFocusCameraModifier>(FindCameraModifierOfClass(FocusModifierClass, true));
	}

	if (YawFollowModifierClass)
	{
		CachedYawFollowModifier = Cast<UYawFollowCameraModifier>(FindCameraModifierOfClass(YawFollowModifierClass, true));
	}

	if (PitchFollowModifierClass)
	{
		CachedPitchFollowModifier = Cast<UPitchFollowModifier>(FindCameraModifierOfClass(PitchFollowModifierClass, true));
	}

	if (ArmOffsetModifierClass)
	{
		CachedArmOffsetModifier = Cast<UArmOffsetModifier>(FindCameraModifierOfClass(ArmOffsetModifierClass, true));
	}

	if (ArmLagModifierClass)
	{
		CachedArmLagModifier = Cast<UArmLagModifier>(FindCameraModifierOfClass(ArmLagModifierClass, true));
	}

	if (PitchToArmFOVModifierClass)
	{
		CachedPitchToArmFOVModifier = Cast<UPitchToArmLengthAndFOVModifier>(FindCameraModifierOfClass(PitchToArmFOVModifierClass, true));
	}

	if (AnglesConstraintModifierClass)
	{
		CachedAnglesConstraintModifier = Cast<UAnglesConstraintModifier>(FindCameraModifierOfClass(AnglesConstraintModifierClass, true));
	}
}

void ARPGPlayerCameraManager::SetCameraData(class URPGCameraDataAssetBase* NewCameraData)
{
	if (NewCameraData)
	{
		PushCameraData(NewCameraData);
	}
}

void ARPGPlayerCameraManager::GetCurrentArmOffsets(FVector& OutSocketOffset, FVector& OutTargetOffset) const
{
	OutSocketOffset = ArmSocketOffset;
	OutTargetOffset = ArmTargetOffset;
}

void ARPGPlayerCameraManager::GetCurrentArmLengthRange(float& MinLength, float& MaxLength) const
{
	MinLength = MinArmLength;
	MaxLength = MaxArmLength;
}

FVector ARPGPlayerCameraManager::GetCameraPivotLocation() const
{
	if (USpringArmComponent* SpringArmComp= GetOwnerSpringArmComponent())
	{
		return SpringArmComp->GetComponentLocation() + SpringArmComp->TargetOffset;
	}
	return FVector::ZeroVector;
}

FRotator ARPGPlayerCameraManager::GetControlRotation() const
{
	APlayerController* PC = GetOwningPlayerController();
	if (PC)
	{
		return PC->GetControlRotation();
	}
	return FRotator::ZeroRotator;
}

void ARPGPlayerCameraManager::GetCurrentFOVRange(float& OutMinFOV, float& OutMaxFOV) const
{
	OutMinFOV = MinFOV;
	OutMaxFOV = MaxFOV;
}

void ARPGPlayerCameraManager::SetPitchToArmLengthAndFOVCurves(const FCameraPitchToArmAndFOVCurveSettings& InSettings)
{
	if (CachedPitchToArmFOVModifier)
	{
		CachedPitchToArmFOVModifier->CurveSettings = InSettings;
	}
}

float ARPGPlayerCameraManager::GetCameraAnimationDuration(UMovieSceneSequence* Sequence,const FRPGCameraAnimationParams& AnimationParams) const
{
	if (Sequence)
	{
		// 시퀀스 데이터(MovieScene)를 가져옵니다.
		if (UMovieScene* MovieScene = Sequence->GetMovieScene())
		{
			// 1. 시퀀스의 틱 해상도(초당 프레임 정밀도)를 가져옵니다.
			FFrameRate TickResolution = MovieScene->GetTickResolution();
            
			// 2. 재생 범위(Playback Range)의 끝 프레임 번호를 가져옵니다.
			FFrameNumber EndFrame = MovieScene->GetPlaybackRange().GetUpperBoundValue();
            
			// 3. 프레임을 초(Seconds) 단위로 변환합니다.
			double EndSeconds = TickResolution.AsSeconds(EndFrame);
            
			return (float)EndSeconds * AnimationParams.PlayRate;
		}
	} 
	return 0.f;
}

void ARPGPlayerCameraManager::StopCameraAnimation(UCameraAnimationSequence* Sequence, bool bImmediate)
{
	if (CachedAnimationModifier)
	{
		CachedAnimationModifier->StopCameraAnimationSequence(Sequence,bImmediate);
	}
}

bool ARPGPlayerCameraManager::IsPlayingCameraAnimation(UCameraAnimationSequence* Sequence) const
{
	if (CachedAnimationModifier)
	{
		return CachedAnimationModifier->IsCameraAnimationSequenceActive(Sequence);
	}
	return false;
}

void ARPGPlayerCameraManager::GetCurrentCameraAnimations(TArray<UCameraAnimationSequence*>& OutAnimations) const
{
	if (CachedAnimationModifier)
	{
		CachedAnimationModifier->GetCurrentCameraAnimations(OutAnimations);
	}
}


bool ARPGPlayerCameraManager::IsYawMovementFollowEnabled() const
{
	if (CachedYawFollowModifier)
	{
		return CachedYawFollowModifier->FollowSettings.bEnableYawMovementFollow;
	}
	return false;
}

void ARPGPlayerCameraManager::SetYawMovementFollowSettings(const FCameraYawFollowSettings& InSettings)
{
	if (CachedYawFollowModifier)
	{
		CachedYawFollowModifier->FollowSettings = InSettings;
	}
}

bool ARPGPlayerCameraManager::IsPitchMovementFollowEnabled() const
{
	if (CachedPitchFollowModifier)
	{
		return CachedPitchFollowModifier->FollowSettings.bEnablePitchMovementFollow ||
			CachedPitchFollowModifier->FollowSettings.bTriggerWhenFalling ||
			CachedPitchFollowModifier->FollowSettings.bTriggerOnSlopes;
	}
	return false;
}

void ARPGPlayerCameraManager::SetPitchMovementFollowSettings(const FCameraPitchFollowSettings& InSettings)
{
	if (CachedPitchFollowModifier)
	{
		CachedPitchFollowModifier->FollowSettings = InSettings;
	}
}

void ARPGPlayerCameraManager::ToggleYawAndPitchFollow(bool bEnableYaw, bool bEnablePitch, bool bTriggerPitchFollowWhenFalling,bool bTriggerPitchFollowOnSlopes)
{
	if (CachedPitchFollowModifier)
	{
		CachedPitchFollowModifier->FollowSettings.bEnablePitchMovementFollow = bEnablePitch;
		CachedPitchFollowModifier->FollowSettings.bTriggerWhenFalling = bTriggerPitchFollowWhenFalling;
		CachedPitchFollowModifier->FollowSettings.bTriggerOnSlopes = bTriggerPitchFollowOnSlopes;
	}
	
	if (CachedYawFollowModifier)
	{
		CachedYawFollowModifier->FollowSettings.bEnableYawMovementFollow = bEnableYaw;
	}
}

void ARPGPlayerCameraManager::SetPitchConstraints(const FCameraPitchConstraintSettings& InSettings)
{
	if (CachedAnglesConstraintModifier)
	{
		CachedAnglesConstraintModifier->PitchConstraintTolerance = InSettings.PitchConstraintTolerance;
		SetPitchViewConstraints(
			InSettings.bConstrainPitch ? InSettings.LocalMinPitch : -89.9f,
			InSettings.bConstrainPitch ? InSettings.LocalMaxPitch : 89.9f,
			InSettings.PitchConstraintsBlendTime,
			InSettings.PitchConstraintsBlendCurve
			);
	}
}

void ARPGPlayerCameraManager::SetYawConstraints(const FCameraYawConstraintSettings& InSettings)
{
	if (CachedAnglesConstraintModifier)
	{
		CachedAnglesConstraintModifier->YawConstraintTolerance = InSettings.YawConstraintTolerance;
		SetYawViewConstraints(
			InSettings.bConstrainYaw ? InSettings.LocalMinYaw : -89.9f,
			InSettings.bConstrainYaw ? InSettings.LocalMaxYaw : 89.9f,
			InSettings.YawConstraintsBlendTime,
			InSettings.YawConstraintsBlendCurve
			);
	}
}

void ARPGPlayerCameraManager::SetCameraFocusSettings(const FRPGCameraFocusSettings& InSettings)
{
	if (CachedFocusModifier)
	{
		CachedFocusModifier->FocusSettings = InSettings;
	}
}

void ARPGPlayerCameraManager::ToggleCameraFocus(bool bEnable)
{
	if (CachedFocusModifier)
	{
		CachedFocusModifier->FocusSettings.bEnabled = bEnable;
	}
}

URPGFocusTargetMethod* ARPGPlayerCameraManager::SetCameraFocusTargetingMethod(TSubclassOf<URPGFocusTargetMethod> MethodClass)
{
	if (CachedFocusModifier)
	{
		URPGFocusTargetMethod* NewObj = NewObject<URPGFocusTargetMethod>(this, MethodClass);
		CachedFocusModifier->FocusSettings.FocusTargetMethod = NewObj;
		return NewObj;	
	}
	return nullptr;
}

bool ARPGPlayerCameraManager::IsFocusEnabledInCurrentData() const
{
	if (URPGCameraDataAssetBase* CameraData = GetCurrentCameraDataAsset())
	{
		return CameraData->FocusSettings.bEnabled;
	}
	return false;
}

void ARPGPlayerCameraManager::SetCameraDitheringSettings(const FCameraDitheringSettings& InSettings)
{
	if (CachedDitheringModifier)
	{
		CachedDitheringModifier->DitheringSettings = InSettings;
	}
}

void ARPGPlayerCameraManager::SetCameraLagSettings(const FCameraArmLagSettings& InSettings)
{
	if (CachedArmLagModifier)
	{
		CachedArmLagModifier->ArmLagSettings = InSettings;
		CachedArmLagModifier->UpdateLagSettings();
	}
}

void ARPGPlayerCameraManager::UpdateAddOnSettings(TSubclassOf<class URPGCameraAddOnModifierSettings> SettingsClass,
	class URPGCameraAddOnModifierSettings* NewSettings)
{
}

void ARPGPlayerCameraManager::ResetControlRotationTo(FRotator TargetRotation, float Duration, UCurveFloat* Curve,
	bool bInterruptWithLookInput)
{
	// 1. Validate Timeline Component
	if (!ResetControlRotationToTimeline)
	{
		UE_LOG(LogTemp, Warning, TEXT("ResetControlRotationTo: Timeline Component is missing!"));
		return;
	}

	// 2. Stop existing animation
	ResetControlRotationToTimeline->Stop();

	// 3. Validate Input Curve
	if (!Curve)
	{
		UE_LOG(LogTemp, Warning, TEXT("ResetControlRotationTo: Curve is invalid!"));
		return;
	}

	// 4. Setup State Variables
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	StartControlRotation = PC->GetControlRotation();
	TargetResetRotation = TargetRotation;
	bInterruptResetWithLookInput = bInterruptWithLookInput;

	// 5. Instant Reset Check (Duration is nearly zero)
	// "If blend time is 0, do it instantly" Logic
	if (Duration <= KINDA_SMALL_NUMBER)
	{
		PC->SetControlRotation(TargetRotation);
		return;
	}

	// 6. Configure Timeline
	// 블루프린트의 "Set Float Curve" 노드와 동일
	// 타임라인에 사용할 커브를 동적으로 할당합니다. 트랙 이름은 "Alpha"로 가정합니다.
	// 만약 생성자에서 FOnTimelineFloat를 바인딩하지 않았다면 여기서 바인딩해야 합니다.
    
	// (중요) 이전에 바인딩된 트랙이 있다면 먼저 정리하는 것이 좋으나, 
	// 여기서는 AddInterpFloat가 기존 트랙을 덮어쓰거나 추가한다고 가정합니다.
	// 가장 깔끔한 방법은 BeginPlay에서 델리게이트를 미리 묶어두고, 여기서는 Curve만 갈아끼우는 것입니다.
    
	FOnTimelineFloat ProgressFunction;
	ProgressFunction.BindUFunction(this, FName("OnResetControlRotationTimelineUpdate"));
    
	// 기존 트랙이 있다면 덮어씌우기 위해 AddInterpFloat 사용
	ResetControlRotationToTimeline->AddInterpFloat(Curve, ProgressFunction, FName("Alpha"));

	// 7. Set Play Rate ( Duration이 1초가 되도록 Rate 조정 )
	// "Safe Divide" Logic
	float NewRate = (Duration > 0.0f) ? (1.0f / Duration) : 1.0f;
	ResetControlRotationToTimeline->SetPlayRate(NewRate);

	// 8. Play
	ResetControlRotationToTimeline->SetNewTime(0.0f);
	ResetControlRotationToTimeline->PlayFromStart();
}

void ARPGPlayerCameraManager::ResetControlRotation(float Duration, UCurveFloat* Curve, bool bInterruptWithLookInput)
{
	// 1. Player Controller 확인
	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	// 2. Controlled Pawn 확인
	APawn* ControlledPawn = PC->GetPawn();
	if (!ControlledPawn)
	{
		// 폰이 없으면 리셋할 기준이 없으므로 리턴
		return; 
	}

	// 3. 캐릭터의 현재 회전값 가져오기 (Get Actor Rotation)
	// 보통 캐릭터의 뒤를 본다는 것은 캐릭터의 ControlRotation이나 ActorRotation을 따라간다는 의미입니다.
	FRotator CharacterRotation = ControlledPawn->GetActorRotation();

	// 4. ResetControlRotationTo 호출 (재사용)
	// 구한 캐릭터의 회전값을 TargetRotation으로 넘겨줍니다.
	ResetControlRotationTo(CharacterRotation, Duration, Curve, bInterruptWithLookInput);
}


void ARPGPlayerCameraManager::OnResetControlRotationTimelineUpdate(float Alpha)
{
	// 1. Check Interruption
	// "HasRotationInput" AND "InterruptWithLookInput" -> Branch
	if (bInterruptResetWithLookInput && bHasRotationInput)
	{
		if (ResetControlRotationToTimeline)
		{
			ResetControlRotationToTimeline->Stop();
		}
		return;
	}

	APlayerController* PC = GetOwningPlayerController();
	if (!PC) return;

	// 2. Lerp Rotation (Shortest Path)
	// 블루프린트의 "Lerp (Rotator)" 노드 (Shortest Path 체크됨)
	FRotator NewRotation = UKismetMathLibrary::RLerp(StartControlRotation, TargetResetRotation, Alpha, true);
	
	// 3. Set Control Rotation
	PC->SetControlRotation(NewRotation);
}

void ARPGPlayerCameraManager::SetArmLengthRange(float InMinArmLength, float InMaxArmLength, float InBlendTime,
	UCurveFloat* InBlendCurve)
{
	// 0. Safety Check
	if (!ArmLengthRangeTimeline) return;

	// 1. Save Target Values
	TargetMinArmLength = InMinArmLength;
	TargetMaxArmLength = InMaxArmLength;

	// 2. Save Previous Values (현재 적용 중인 데이터 에셋이나 상태에서 가져옴)
	// GetCurrentCameraDataAsset() 함수가 있다고 가정 (혹은 멤버변수 직접 접근)
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		PreviousMinArmLength = CurrentData->ArmLengthSettings.MinArmLength;
		PreviousMaxArmLength = CurrentData->ArmLengthSettings.MaxArmLength;
	}
	else
	{
		// 데이터가 없으면 타겟값으로 초기화 (즉시 적용 효과)
		PreviousMinArmLength = InMinArmLength;
		PreviousMaxArmLength = InMaxArmLength;
	}

	// 3. Instant Set Check (BlendTime <= 0)
	if (InBlendTime <= KINDA_SMALL_NUMBER)
	{
		// 즉시 적용: 데이터 에셋의 값을 직접 수정하거나, 상태 변수를 업데이트
		if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
		{
			CurrentData->ArmLengthSettings.MinArmLength = TargetMinArmLength;
			CurrentData->ArmLengthSettings.MaxArmLength = TargetMaxArmLength;
		}
		return;
	}

	// 4. Configure Timeline
	ArmLengthRangeTimeline->Stop();

	UCurveFloat* CurveToUse = InBlendCurve ? InBlendCurve : nullptr;
	if (!CurveToUse)
	{
		// 커브가 없으면 선형 보간을 위해 PlayRate만 설정하고 Alpha는 시간값 자체를 쓸 수도 있으나,
		// 여기서는 커브 필수 로직을 따릅니다.
		UE_LOG(LogTemp, Warning, TEXT("SetArmLengthRange: No Valid Curve!"));
		return;
	}

	FOnTimelineFloat ProgressFunction;
	ProgressFunction.BindUFunction(this, FName("OnArmLengthRangeTimelineUpdate"));
	ArmLengthRangeTimeline->AddInterpFloat(CurveToUse, ProgressFunction, FName("Alpha"));

	float PlayRate = (InBlendTime > 0.0f) ? (1.0f / InBlendTime) : 1.0f;
	ArmLengthRangeTimeline->SetPlayRate(PlayRate);
	ArmLengthRangeTimeline->SetNewTime(0.0f);
	ArmLengthRangeTimeline->PlayFromStart();
}

void ARPGPlayerCameraManager::OnArmLengthRangeTimelineUpdate(float Alpha)
{
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		// Lerp Values
		float NewMin = FMath::Lerp(PreviousMinArmLength, TargetMinArmLength, Alpha);
		float NewMax = FMath::Lerp(PreviousMaxArmLength, TargetMaxArmLength, Alpha);

		// Apply to Data Asset (런타임 수정)
		// 주의: 이 방식은 에셋 자체를 수정하므로, 게임 종료 후에도 값이 유지될 수 있습니다 (에디터에서).
		// 런타임 복사본(Instance)을 사용하는 것이 좋습니다.
		CurrentData->ArmLengthSettings.MinArmLength = NewMin;
		CurrentData->ArmLengthSettings.MaxArmLength = NewMax;
	}
}

void ARPGPlayerCameraManager::SetFOVRange(float InMinFOV, float InMaxFOV, float InBlendTime, UCurveFloat* InBlendCurve)
{
	// 0. Safety Check
	if (!FOVRangeTimeline) return;

	// 1. Save Target Values
	TargetMinFOV = InMinFOV;
	TargetMaxFOV = InMaxFOV;

	// 2. Save Previous Values
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		PreviousMinFOV = CurrentData->FOVSettings.MinFOV;
		PreviousMaxFOV = CurrentData->FOVSettings.MaxFOV;
	}
	else
	{
		PreviousMinFOV = InMinFOV;
		PreviousMaxFOV = InMaxFOV;
	}

	// 3. Instant Set Check
	if (InBlendTime <= KINDA_SMALL_NUMBER)
	{
		if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
		{
			CurrentData->FOVSettings.MinFOV = TargetMinFOV;
			CurrentData->FOVSettings.MaxFOV = TargetMaxFOV;
		}
		return;
	}

	// 4. Configure Timeline
	FOVRangeTimeline->Stop();

	UCurveFloat* CurveToUse = InBlendCurve ? InBlendCurve : nullptr;
	if (!CurveToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetFOVRange: No Valid Curve!"));
		return;
	}

	FOnTimelineFloat ProgressFunction;
	ProgressFunction.BindUFunction(this, FName("OnFOVRangeTimelineUpdate"));
	FOVRangeTimeline->AddInterpFloat(CurveToUse, ProgressFunction, FName("Alpha"));

	float PlayRate = (InBlendTime > 0.0f) ? (1.0f / InBlendTime) : 1.0f;
	FOVRangeTimeline->SetPlayRate(PlayRate);
	FOVRangeTimeline->SetNewTime(0.0f);
	FOVRangeTimeline->PlayFromStart();
}

void ARPGPlayerCameraManager::OnFOVRangeTimelineUpdate(float Alpha)
{
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
    {
        // Lerp Values
        float NewMin = FMath::Lerp(PreviousMinFOV, TargetMinFOV, Alpha);
        float NewMax = FMath::Lerp(PreviousMaxFOV, TargetMaxFOV, Alpha);

        // Apply to Data Asset
        CurrentData->FOVSettings.MinFOV = NewMin;
        CurrentData->FOVSettings.MaxFOV = NewMax;
    }
}

void ARPGPlayerCameraManager::SetArmSocketOffset(FVector ArmOffset, float BlendTime, UCurveFloat* BlendCurve)
{
	// 0. Safety Check
	if (!ArmSocketOffsetTimeline) return;

	// 1. Save Target Value
	TargetArmSocketOffset = ArmOffset;

	// 2. Save Previous Value
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		PreviousArmSocketOffset = CurrentData->ArmOffsetSettings.ArmSocketOffset;
	}
	else
	{
		// Fallback: 현재 매니저의 상태 변수나 타겟값으로 초기화
		PreviousArmSocketOffset = ArmSocketOffset; // ArmSocketOffset 변수는 헤더에 선언되어 있다고 가정
	}

	// 3. Instant Set Check
	if (BlendTime <= KINDA_SMALL_NUMBER)
	{
		if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
		{
			CurrentData->ArmOffsetSettings.ArmSocketOffset = TargetArmSocketOffset;
		}
		// 상태 변수도 즉시 업데이트 (선택 사항)
		ArmSocketOffset = TargetArmSocketOffset; 
		return;
	}

	// 4. Configure Timeline
	ArmSocketOffsetTimeline->Stop();

	UCurveFloat* CurveToUse = BlendCurve ? BlendCurve : nullptr;
	if (!CurveToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetArmSocketOffset: No Valid Curve!"));
		return;
	}

	FOnTimelineFloat ProgressFunction;
	ProgressFunction.BindUFunction(this, FName("OnArmSocketOffsetTimelineUpdate"));
	ArmSocketOffsetTimeline->AddInterpFloat(CurveToUse, ProgressFunction, FName("Alpha"));

	float PlayRate = (BlendTime > 0.0f) ? (1.0f / BlendTime) : 1.0f;
	ArmSocketOffsetTimeline->SetPlayRate(PlayRate);
	ArmSocketOffsetTimeline->SetNewTime(0.0f);
	ArmSocketOffsetTimeline->PlayFromStart();
}

void ARPGPlayerCameraManager::OnArmSocketOffsetTimelineUpdate(float Alpha)
{
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		// Lerp Vector
		FVector NewOffset = FMath::Lerp(PreviousArmSocketOffset, TargetArmSocketOffset, Alpha);

		// Apply to Data Asset
		CurrentData->ArmOffsetSettings.ArmSocketOffset = NewOffset;
        
		// (옵션) 상태 변수 업데이트 - 다른 로직에서 참조할 경우 필요
		ArmSocketOffset = NewOffset; 
	}
}

void ARPGPlayerCameraManager::SetArmTargetOffset(FVector InArmTargetOffset, float BlendTime, UCurveFloat* BlendCurve)
{
	// 0. Safety Check
	if (!ArmTargetOffsetTimeline) return;

	// 1. Save Target Value
	TargetArmTargetOffset = InArmTargetOffset;

	// 2. Save Previous Value
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		PreviousArmTargetOffset = CurrentData->ArmOffsetSettings.ArmTargetOffset;
	}
	else
	{
		PreviousArmTargetOffset = ArmTargetOffset; 
	}

	// 3. Instant Set Check
	if (BlendTime <= KINDA_SMALL_NUMBER)
	{
		if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
		{
			CurrentData->ArmOffsetSettings.ArmTargetOffset = TargetArmTargetOffset;
		}
		ArmTargetOffset = TargetArmTargetOffset;
		return;
	}

	// 4. Configure Timeline
	ArmTargetOffsetTimeline->Stop();

	UCurveFloat* CurveToUse = BlendCurve ? BlendCurve : nullptr;
	if (!CurveToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetArmTargetOffset: No Valid Curve!"));
		return;
	}

	FOnTimelineFloat ProgressFunction;
	ProgressFunction.BindUFunction(this, FName("OnArmTargetOffsetTimelineUpdate"));
	ArmTargetOffsetTimeline->AddInterpFloat(CurveToUse, ProgressFunction, FName("Alpha"));

	float PlayRate = (BlendTime > 0.0f) ? (1.0f / BlendTime) : 1.0f;
	ArmTargetOffsetTimeline->SetPlayRate(PlayRate);
	ArmTargetOffsetTimeline->SetNewTime(0.0f);
	ArmTargetOffsetTimeline->PlayFromStart();
}

void ARPGPlayerCameraManager::OnArmTargetOffsetTimelineUpdate(float Alpha)
{
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		// Lerp Vector
		FVector NewOffset = FMath::Lerp(PreviousArmTargetOffset, TargetArmTargetOffset, Alpha);

		// Apply
		CurrentData->ArmOffsetSettings.ArmTargetOffset = NewOffset;
		ArmTargetOffset = NewOffset;
	}
}

void ARPGPlayerCameraManager::SetPitchViewConstraints(float InViewPitchMin, float InViewPitchMax, float InBlendTime,
	UCurveFloat* InBlendCurve)
{
	// 0. Safety Check
	if (!PitchRangeTimeline) return;

	// 1. Save Target Values
	TargetMinPitch = InViewPitchMin;
	TargetMaxPitch = InViewPitchMax;

	// 2. Save Previous Values
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		PreviousMinPitch = CurrentData->PitchConstraints.LocalMinPitch;
		PreviousMaxPitch = CurrentData->PitchConstraints.LocalMaxPitch;
	}
	else
	{
		PreviousMinPitch = InViewPitchMin;
		PreviousMaxPitch = InViewPitchMax;
	}

	// 3. Instant Set Check
	if (InBlendTime <= KINDA_SMALL_NUMBER)
	{
		if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
		{
			CurrentData->PitchConstraints.LocalMinPitch = TargetMinPitch;
			CurrentData->PitchConstraints.LocalMaxPitch = TargetMaxPitch;
		}
		return;
	}

	// 4. Configure Timeline
	PitchRangeTimeline->Stop();

	UCurveFloat* CurveToUse = InBlendCurve ? InBlendCurve : nullptr;
	if (!CurveToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetPitchViewConstraints: No Valid Curve!"));
		return;
	}

	FOnTimelineFloat ProgressFunction;
	ProgressFunction.BindUFunction(this, FName("OnPitchRangeTimelineUpdate"));
	PitchRangeTimeline->AddInterpFloat(CurveToUse, ProgressFunction, FName("Alpha"));

	float PlayRate = (InBlendTime > 0.0f) ? (1.0f / InBlendTime) : 1.0f;
	PitchRangeTimeline->SetPlayRate(PlayRate);
	PitchRangeTimeline->SetNewTime(0.0f);
	PitchRangeTimeline->PlayFromStart();
}


void ARPGPlayerCameraManager::OnPitchRangeTimelineUpdate(float Alpha)
{
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		// Lerp Values
		float NewMin = FMath::Lerp(PreviousMinPitch, TargetMinPitch, Alpha);
		float NewMax = FMath::Lerp(PreviousMaxPitch, TargetMaxPitch, Alpha);

		// Apply to Data Asset
		CurrentData->PitchConstraints.LocalMinPitch = NewMin;
		CurrentData->PitchConstraints.LocalMaxPitch = NewMax;
	}
}

void ARPGPlayerCameraManager::SetYawViewConstraints(float InViewYawMin, float InViewYawMax, float InBlendTime,
	UCurveFloat* InBlendCurve)
{
	// 0. Safety Check
	if (!YawRangeTimeline) return;

	// 1. Save Target Values
	TargetMinYaw = InViewYawMin;
	TargetMaxYaw = InViewYawMax;

	// 2. Save Previous Values
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		PreviousMinYaw = CurrentData->YawConstraints.LocalMinYaw;
		PreviousMaxYaw = CurrentData->YawConstraints.LocalMaxYaw;
	}
	else
	{
		PreviousMinYaw = InViewYawMin;
		PreviousMaxYaw = InViewYawMax;
	}

	// 3. Instant Set Check
	if (InBlendTime <= KINDA_SMALL_NUMBER)
	{
		if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
		{
			CurrentData->YawConstraints.LocalMinYaw = TargetMinYaw;
			CurrentData->YawConstraints.LocalMaxYaw = TargetMaxYaw;
		}
		return;
	}

	// 4. Configure Timeline
	YawRangeTimeline->Stop();

	UCurveFloat* CurveToUse = InBlendCurve ? InBlendCurve : nullptr;
	if (!CurveToUse)
	{
		UE_LOG(LogTemp, Warning, TEXT("SetYawViewConstraints: No Valid Curve!"));
		return;
	}

	FOnTimelineFloat ProgressFunction;
	ProgressFunction.BindUFunction(this, FName("OnYawRangeTimelineUpdate"));
	YawRangeTimeline->AddInterpFloat(CurveToUse, ProgressFunction, FName("Alpha"));

	float PlayRate = (InBlendTime > 0.0f) ? (1.0f / InBlendTime) : 1.0f;
	YawRangeTimeline->SetPlayRate(PlayRate);
	YawRangeTimeline->SetNewTime(0.0f);
	YawRangeTimeline->PlayFromStart();
}

void ARPGPlayerCameraManager::OnYawRangeTimelineUpdate(float Alpha)
{
	if (URPGCameraDataAssetBase* CurrentData = GetCurrentCameraDataAsset())
	{
		// Lerp Values
		float NewMin = FMath::Lerp(PreviousMinYaw, TargetMinYaw, Alpha);
		float NewMax = FMath::Lerp(PreviousMaxYaw, TargetMaxYaw, Alpha);

		// Apply to Data Asset
		CurrentData->YawConstraints.LocalMinYaw = NewMin;
		CurrentData->YawConstraints.LocalMaxYaw = NewMax;
	}
}
