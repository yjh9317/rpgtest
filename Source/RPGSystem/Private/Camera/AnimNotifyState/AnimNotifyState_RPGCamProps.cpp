// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/AnimNotifyState/AnimNotifyState_RPGCamProps.h"

#include "Camera/RPGPlayerCameraManager.h"
#include "Camera/Modifiers/RPGCameraNotifyModifiers.h"


FLinearColor URPGFOVRequestAnimNotifyState::GetEditorColor()
{
#if WITH_EDITORONLY_DATA
	const uint8 Ratio = 255 - static_cast<uint8>(FMath::GetMappedRangeValueClamped(FVector2D(5.f, 170.f), FVector2D(0.f, 255.f), TargetFOV));
	NotifyColor = FColor(255, Ratio, Ratio, 255);
	return NotifyColor;
#else
	return FLinearColor::Black;
#endif
}

void URPGFOVRequestAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		if (APlayerController* PC = OwnerPawn->GetController<APlayerController>())
		{
			if (ARPGPlayerCameraManager* CameraManager = Cast<ARPGPlayerCameraManager>(PC->PlayerCameraManager))
			{
				if (URPGFOVAnimNotifyCameraModifier* Modifier = CameraManager->FindCameraModifierOfType<URPGFOVAnimNotifyCameraModifier>())
				{
					Modifier->PushFOVAnimNotifyRequest(RequestId, TargetFOV, TotalDuration, BlendInDuration, BlendInCurve, BlendOutDuration, BlendOutCurve);
				}
			}
		}
	}
}

FString URPGFOVRequestAnimNotifyState::GetNotifyName_Implementation() const
{
#if WITH_EDITORONLY_DATA
	if (bShowRequestIdInName)
	{
		return FString::Printf(TEXT("[%s][UGC] FOV Request: %.2f"), *RequestId.ToString(), TargetFOV);
	}
	else
#endif
	{
		return FString::Printf(TEXT("[UGC] FOV Request: %.2f"), TargetFOV);
	}
}

FLinearColor URPGArmOffsetRequestAnimNotifyState::GetEditorColor()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 212, 105, 255);
	return NotifyColor;
#else
	return FLinearColor::Black;
#endif
}

void URPGArmOffsetRequestAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		if (APlayerController* PC = OwnerPawn->GetController<APlayerController>())
		{
			if (ARPGPlayerCameraManager* CameraManager = Cast<ARPGPlayerCameraManager>(PC->PlayerCameraManager))
			{
				if (URPGArmOffsetAnimNotifyCameraModifier* Modifier = CameraManager->FindCameraModifierOfType<URPGArmOffsetAnimNotifyCameraModifier>())
				{
					if (bModifySocketOffset)
					{
						Modifier->PushArmSocketOffsetAnimNotifyRequest(RequestId, TargetSocketOffset, TotalDuration, SocketOffsetBlendInDuration, SocketOffsetBlendInCurve, SocketOffsetBlendOutDuration, SocketOffsetBlendOutCurve);
					}

					if (bModifyTargetOffset)
					{
						Modifier->PushArmTargetOffsetAnimNotifyRequest(RequestId, TargetTargetOffset, TotalDuration, TargetOffsetBlendInDuration, TargetOffsetBlendInCurve, TargetOffsetBlendOutDuration, TargetOffsetBlendOutCurve);
					}
				}
			}
		}
	}
}

FString URPGArmOffsetRequestAnimNotifyState::GetNotifyName_Implementation() const
{
#if WITH_EDITORONLY_DATA
	if (bShowRequestIdInName)
	{
		FString Text = FString::Printf(TEXT("[%s][UGC]"), *RequestId.ToString());
		if (bModifySocketOffset)
		{
			Text.Append(FString::Printf(TEXT(" Socket: %s"), *TargetSocketOffset.ToCompactString()));
		}
		if (bModifyTargetOffset)
		{
			Text.Append(FString::Printf(TEXT(" Target: %s"), *TargetTargetOffset.ToCompactString()));
		}
		return Text;
	}
	else
#endif
	{
		FString Text = TEXT("[UGC]");
		if (bModifySocketOffset)
		{
			Text.Append(FString::Printf(TEXT(" Socket: %s"), *TargetSocketOffset.ToCompactString()));
		}
		if (bModifyTargetOffset)
		{
			Text.Append(FString::Printf(TEXT(" Target: %s"), *TargetTargetOffset.ToCompactString()));
		}
		return Text;
	}
}

FLinearColor URPGArmLengthRequestAnimNotifyState::GetEditorColor()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(55, 219, 33, 255);
	return NotifyColor;
#else
	return FLinearColor::Black;
#endif
}

void URPGArmLengthRequestAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	AActor* Owner = MeshComp->GetOwner();
	if (APawn* OwnerPawn = Cast<APawn>(Owner))
	{
		if (APlayerController* PC = OwnerPawn->GetController<APlayerController>())
		{
			if (ARPGPlayerCameraManager* CameraManager = Cast<ARPGPlayerCameraManager>(PC->PlayerCameraManager))
			{
				if (URPGArmLengthAnimNotifyCameraModifier* Modifier = CameraManager->FindCameraModifierOfType<URPGArmLengthAnimNotifyCameraModifier>())
				{
					Modifier->PushArmLengthAnimNotifyRequest(RequestId, TargetArmLength, TotalDuration, BlendInDuration, BlendInCurve, BlendOutDuration, BlendOutCurve);
				}
			}
		}
	}
}

FString URPGArmLengthRequestAnimNotifyState::GetNotifyName_Implementation() const
{
#if WITH_EDITORONLY_DATA
	if (bShowRequestIdInName)
	{
		return FString::Printf(TEXT("[%s][UGC] Arm Length: %.2f"), *RequestId.ToString(), TargetArmLength);
	}
	else
#endif
	{
		return FString::Printf(TEXT("[UGC] Arm Length: %.2f"), TargetArmLength);
	}
}
