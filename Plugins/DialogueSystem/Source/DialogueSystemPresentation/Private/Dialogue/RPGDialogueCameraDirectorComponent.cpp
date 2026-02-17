#include "Dialogue/RPGDialogueCameraDirectorComponent.h"

#include "Camera/CameraActor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"

URPGDialogueCameraDirectorComponent::URPGDialogueCameraDirectorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

APlayerController* URPGDialogueCameraDirectorComponent::ResolveOwningPC() const
{
	return Cast<APlayerController>(GetOwner());
}

void URPGDialogueCameraDirectorComponent::EnsureCameraActor()
{
	if (DialogueCameraActor || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	DialogueCameraActor = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), FTransform::Identity, SpawnParams);
}

FTransform URPGDialogueCameraDirectorComponent::BuildShotTransform(const FDialogueCinematicCue& Cue, AActor* Speaker) const
{
	const APlayerController* PC = ResolveOwningPC();
	const APawn* PlayerPawn = PC ? PC->GetPawn() : nullptr;

	const FVector PlayerLoc = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
	const FVector SpeakerLoc = Speaker ? Speaker->GetActorLocation() : (PlayerLoc + FVector(100.f, 0.f, 0.f));

	FVector ToSpeaker = (SpeakerLoc - PlayerLoc).GetSafeNormal();
	if (ToSpeaker.IsNearlyZero())
	{
		ToSpeaker = FVector::ForwardVector;
	}

	const FVector Right = FVector::CrossProduct(FVector::UpVector, ToSpeaker).GetSafeNormal();
	const float Dist = Cue.Distance > 0.f ? Cue.Distance : DefaultDistance;
	const float Height = Cue.Height;
	const float Side = Cue.SideOffset;

	FVector CamLoc = PlayerLoc - ToSpeaker * Dist + FVector(0.f, 0.f, Height);
	FVector LookTarget = SpeakerLoc + FVector(0.f, 0.f, 60.f);

	switch (Cue.ShotType)
	{
	case EDialogueCameraShotType::TwoShot:
		CamLoc = ((PlayerLoc + SpeakerLoc) * 0.5f) - ToSpeaker * (Dist * 0.55f) + FVector(0.f, 0.f, Height);
		LookTarget = ((PlayerLoc + SpeakerLoc) * 0.5f) + FVector(0.f, 0.f, 55.f);
		break;
	case EDialogueCameraShotType::OverShoulderPlayer:
		CamLoc = PlayerLoc - ToSpeaker * (Dist * 0.45f) + Right * Side + FVector(0.f, 0.f, Height);
		LookTarget = SpeakerLoc + FVector(0.f, 0.f, 60.f);
		break;
	case EDialogueCameraShotType::OverShoulderNPC:
		CamLoc = SpeakerLoc + ToSpeaker * (Dist * 0.45f) - Right * Side + FVector(0.f, 0.f, Height);
		LookTarget = PlayerLoc + FVector(0.f, 0.f, 60.f);
		break;
	case EDialogueCameraShotType::CloseUpPlayer:
		CamLoc = PlayerLoc + ToSpeaker * (Dist * 0.18f) + FVector(0.f, 0.f, Height);
		LookTarget = PlayerLoc + ToSpeaker * 80.f + FVector(0.f, 0.f, 70.f);
		break;
	case EDialogueCameraShotType::CloseUpNPC:
		CamLoc = SpeakerLoc - ToSpeaker * (Dist * 0.22f) + FVector(0.f, 0.f, Height);
		LookTarget = SpeakerLoc + FVector(0.f, 0.f, 70.f);
		break;
	case EDialogueCameraShotType::Wide:
		CamLoc = ((PlayerLoc + SpeakerLoc) * 0.5f) - ToSpeaker * (Dist * 1.4f) + FVector(0.f, 0.f, Height + 45.f);
		LookTarget = ((PlayerLoc + SpeakerLoc) * 0.5f) + FVector(0.f, 0.f, 55.f);
		break;
	case EDialogueCameraShotType::None:
	default:
		break;
	}

	FRotator CamRot = UKismetMathLibrary::FindLookAtRotation(CamLoc, LookTarget);
	CamRot.Yaw += Cue.YawOffset;

	return FTransform(CamRot, CamLoc);
}

void URPGDialogueCameraDirectorComponent::EnterDialogueCamera(AActor* Speaker)
{
	APlayerController* PC = ResolveOwningPC();
	if (!PC)
	{
		return;
	}

	CachedViewTarget = PC->GetViewTarget();
	CachedControlRotation = PC->GetControlRotation();

	FDialogueCinematicCue Cue;
	Cue.ShotType = EDialogueCameraShotType::TwoShot;
	Cue.BlendTime = DefaultBlendTime;
	Cue.Distance = DefaultDistance;
	Cue.Height = DefaultHeight;
	Cue.SideOffset = DefaultSideOffset;

	ApplyCue(Cue, Speaker);
}

void URPGDialogueCameraDirectorComponent::ApplyCue(const FDialogueCinematicCue& Cue, AActor* Speaker)
{
	APlayerController* PC = ResolveOwningPC();
	if (!PC)
	{
		return;
	}

	EnsureCameraActor();
	if (!DialogueCameraActor)
	{
		return;
	}

	DialogueCameraActor->SetActorTransform(BuildShotTransform(Cue, Speaker));
	PC->SetViewTargetWithBlend(DialogueCameraActor, Cue.BlendTime);
}

void URPGDialogueCameraDirectorComponent::ExitDialogueCamera()
{
	APlayerController* PC = ResolveOwningPC();
	if (!PC)
	{
		return;
	}

	AActor* RestoreTarget = CachedViewTarget.IsValid() ? CachedViewTarget.Get() : PC->GetPawn();
	if (RestoreTarget)
	{
		PC->SetViewTargetWithBlend(RestoreTarget, DefaultBlendTime);
	}
	PC->SetControlRotation(CachedControlRotation);
}

void URPGDialogueCameraDirectorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DialogueCameraActor)
	{
		DialogueCameraActor->Destroy();
		DialogueCameraActor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}
