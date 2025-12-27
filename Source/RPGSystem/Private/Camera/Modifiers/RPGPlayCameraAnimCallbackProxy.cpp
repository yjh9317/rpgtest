// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/RPGPlayCameraAnimCallbackProxy.h"


namespace PlayCameraAnimCallbackProxyHelper
{
	FCameraAnimationHandle const RPGInvalid(MAX_int16, 0);
}

URPGPlayCameraAnimCallbackProxy::URPGPlayCameraAnimCallbackProxy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bInterruptedCalledBeforeBlendingOut(false)
{
}

FRPGCameraAnimationParams::operator FCameraAnimationParams() const
{
	FCameraAnimationParams Params;
	Params.PlayRate = PlayRate;
	Params.EaseInDuration = EaseInDuration;
	Params.EaseOutDuration = EaseOutDuration;
	Params.EaseInType = EaseInType;
	Params.EaseOutType = EaseOutType;
	Params.bLoop = false;

	return Params;
}

URPGPlayCameraAnimCallbackProxy* URPGPlayCameraAnimCallbackProxy::CreateProxyObjectForPlayCameraAnim(APlayerCameraManager* InPlayerCameraManager, TSubclassOf<URPGCameraAnimationModifier> ModifierClass, UCameraAnimationSequence* CameraSequence, FRPGCameraAnimationParams Params, FCameraAnimationHandle& Handle, bool bInterruptOthers, bool bDoCollisionChecks)
{
	URPGPlayCameraAnimCallbackProxy* Proxy = NewObject<URPGPlayCameraAnimCallbackProxy>();
	Proxy->SetFlags(RF_StrongRefOnFrame);
	Proxy->PlayCameraAnimation(InPlayerCameraManager, ModifierClass, CameraSequence, Params, Handle, bInterruptOthers, bDoCollisionChecks);
	return Proxy;
}

URPGPlayCameraAnimCallbackProxy* URPGPlayCameraAnimCallbackProxy::CreateProxyObjectForPlayCameraAnimForModifier(URPGCameraAnimationModifier* CameraAnimationModifier, UCameraAnimationSequence* CameraSequence, FRPGCameraAnimationParams Params, FCameraAnimationHandle& Handle, bool bInterruptOthers, bool bDoCollisionChecks)
{
	URPGPlayCameraAnimCallbackProxy* Proxy = NewObject<URPGPlayCameraAnimCallbackProxy>();
	Proxy->SetFlags(RF_StrongRefOnFrame);
	Proxy->PlayCameraAnimation(CameraAnimationModifier, CameraSequence, Params, Handle, bInterruptOthers, bDoCollisionChecks);
	return Proxy;
}

void URPGPlayCameraAnimCallbackProxy::PlayCameraAnimation(URPGCameraAnimationModifier* CameraAnimModifier, UCameraAnimationSequence* CameraSequence, FRPGCameraAnimationParams Params, FCameraAnimationHandle& Handle, bool bInterruptOthers, bool bDoCollisionChecks)
{
	Handle = PlayCameraAnimCallbackProxyHelper::RPGInvalid;
	bool bPlayedSuccessfully = false;

	if (CameraAnimModifier)
	{
		Handle = CameraAnimModifier->PlaySingleCameraAnimation(CameraSequence, static_cast<FCameraAnimationParams>(Params), Params.ResetType, bInterruptOthers, bDoCollisionChecks);
		bPlayedSuccessfully = Handle.IsValid();

		if (bPlayedSuccessfully)
		{
			CameraAnimationModifierPtr = CameraAnimModifier;

			CameraAnimationEasingOutDelegate.BindUObject(this, &URPGPlayCameraAnimCallbackProxy::OnCameraAnimationEasingOut);
			CameraAnimationModifierPtr->CameraAnimation_SetEasingOutDelegate(CameraAnimationEasingOutDelegate, Handle);

			CameraAnimationEndedDelegate.BindUObject(this, &URPGPlayCameraAnimCallbackProxy::OnCameraAnimationEnded);
			CameraAnimationModifierPtr->CameraAnimation_SetEndedDelegate(CameraAnimationEndedDelegate, Handle);
		}
	}

	if (!bPlayedSuccessfully)
	{
		OnInterrupted.Broadcast();
	}
}

void URPGPlayCameraAnimCallbackProxy::PlayCameraAnimation(APlayerCameraManager* InPlayerCameraManager, TSubclassOf<URPGCameraAnimationModifier> ModifierClass, UCameraAnimationSequence* CameraSequence, FRPGCameraAnimationParams Params, FCameraAnimationHandle& Handle, bool bInterruptOthers, bool bDoCollisionChecks)
{
	if (InPlayerCameraManager)
	{
		if (URPGCameraAnimationModifier* CameraAnimModifier = Cast<URPGCameraAnimationModifier>(InPlayerCameraManager->FindCameraModifierByClass(ModifierClass)))
		{
			PlayCameraAnimation(CameraAnimModifier, CameraSequence, Params, Handle, bInterruptOthers, bDoCollisionChecks);
		}
	}
}

void URPGPlayCameraAnimCallbackProxy::OnCameraAnimationEasingOut(UCameraAnimationSequence* CameraAnimation)
{
	OnEaseOut.Broadcast();
}

void URPGPlayCameraAnimCallbackProxy::OnCameraAnimationEnded(UCameraAnimationSequence* CameraAnimation, bool bInterrupted)
{
	if (!bInterrupted)
	{
		OnCompleted.Broadcast();
	}
	else if (!bInterruptedCalledBeforeBlendingOut)
	{
		OnInterrupted.Broadcast();
	}
}