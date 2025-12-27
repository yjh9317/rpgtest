// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/RPGCameraNotifyModifiers.h"


URPGFOVAnimNotifyCameraModifier::URPGFOVAnimNotifyCameraModifier()
	: Super()
{
	bPlayDuringCameraAnimations = true;
	RequestHelper.Init(TEXT("FOV"), [this]() -> bool { return bDebug; });
}

void URPGFOVAnimNotifyCameraModifier::ProcessBoomLengthAndFOV_Implementation(float DeltaTime, float InFOV, float InArmLength, FVector ViewLocation, FRotator ViewRotation, float& OutFOV, float& OutArmLength)
{
	Super::ProcessBoomLengthAndFOV_Implementation(DeltaTime, InFOV, InArmLength, ViewLocation, ViewRotation, OutFOV, OutArmLength);
	RequestHelper.ProcessValue(DeltaTime, InFOV, ViewLocation, ViewRotation, OutFOV);
}

void URPGFOVAnimNotifyCameraModifier::OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV, bool bWasImmediate)
{
	Super::OnModifierDisabled_Implementation(LastPOV, bWasImmediate);
	RequestHelper.OnModifierDisabled(LastPOV, bWasImmediate);
}

void URPGFOVAnimNotifyCameraModifier::PushFOVAnimNotifyRequest(FGuid RequestId, float TargetFOV, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve)
{
	RequestHelper.PushValueRequest(RequestId, TargetFOV, TotalDuration, BlendInDuration, BlendInCurve, BlendOutDuration, BlendOutCurve);
}

void URPGFOVAnimNotifyCameraModifier::PopFOVAnimNotifyRequest(FGuid RequestId)
{
	RequestHelper.PopValueRequest(RequestId);
}

URPGArmOffsetAnimNotifyCameraModifier::URPGArmOffsetAnimNotifyCameraModifier()
	: Super()
{
	bPlayDuringCameraAnimations = false;
	SocketOffsetRequestHelper.Init(TEXT("SocketOffset"), [this]() -> bool { return bDebug; });
	TargetOffsetRequestHelper.Init(TEXT("TargetOffset"), [this]() -> bool { return bDebug; });

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	SocketOffsetRequestHelper.PropertyColor = FColor(252, 195, 53, 255);
	TargetOffsetRequestHelper.PropertyColor = FColor(255, 212, 105, 255);
#endif
}

void URPGArmOffsetAnimNotifyCameraModifier::ProcessBoomOffsets_Implementation(float DeltaTime, FVector InSocketOffset, FVector InTargetOffset, FVector ViewLocation, FRotator ViewRotation, FVector& OutSocketOffset, FVector& OutTargetOffset)
{
	Super::ProcessBoomOffsets_Implementation(DeltaTime, InSocketOffset, InTargetOffset, ViewLocation, ViewRotation, OutSocketOffset, OutTargetOffset);
	SocketOffsetRequestHelper.ProcessValue(DeltaTime, InSocketOffset, ViewLocation, ViewRotation, OutSocketOffset);
	TargetOffsetRequestHelper.ProcessValue(DeltaTime, InTargetOffset, ViewLocation, ViewRotation, OutTargetOffset);
}

void URPGArmOffsetAnimNotifyCameraModifier::OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV, bool bWasImmediate)
{
	Super::OnModifierDisabled_Implementation(LastPOV, bWasImmediate);
	SocketOffsetRequestHelper.OnModifierDisabled(LastPOV, bWasImmediate);
	TargetOffsetRequestHelper.OnModifierDisabled(LastPOV, bWasImmediate);
}

void URPGArmOffsetAnimNotifyCameraModifier::PushArmSocketOffsetAnimNotifyRequest(FGuid RequestId, FVector TargetOffset, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve)
{
	SocketOffsetRequestHelper.PushValueRequest(RequestId, TargetOffset, TotalDuration, BlendInDuration, BlendInCurve, BlendOutDuration, BlendOutCurve);
}

void URPGArmOffsetAnimNotifyCameraModifier::PopArmSocketOffsetAnimNotifyRequest(FGuid RequestId)
{
	SocketOffsetRequestHelper.PopValueRequest(RequestId);
}

void URPGArmOffsetAnimNotifyCameraModifier::PushArmTargetOffsetAnimNotifyRequest(FGuid RequestId, FVector TargetOffset, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve)
{
	TargetOffsetRequestHelper.PushValueRequest(RequestId, TargetOffset, TotalDuration, BlendInDuration, BlendInCurve, BlendOutDuration, BlendOutCurve);
}

void URPGArmOffsetAnimNotifyCameraModifier::PopArmTargetOffsetAnimNotifyRequest(FGuid RequestId)
{
	TargetOffsetRequestHelper.PopValueRequest(RequestId);
}

URPGArmLengthAnimNotifyCameraModifier::URPGArmLengthAnimNotifyCameraModifier()
	: Super()
{
	bPlayDuringCameraAnimations = false;
	RequestHelper.Init(TEXT("ArmLength"), [this]() -> bool { return bDebug; });
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	RequestHelper.PropertyColor = FColor(55, 219, 33, 255);
#endif
}

void URPGArmLengthAnimNotifyCameraModifier::ProcessBoomLengthAndFOV_Implementation(float DeltaTime, float InFOV, float InArmLength, FVector ViewLocation, FRotator ViewRotation, float& OutFOV, float& OutArmLength)
{
	Super::ProcessBoomLengthAndFOV_Implementation(DeltaTime, InFOV, InArmLength, ViewLocation, ViewRotation, OutFOV, OutArmLength);
	RequestHelper.ProcessValue(DeltaTime, InArmLength, ViewLocation, ViewRotation, OutArmLength);
}

void URPGArmLengthAnimNotifyCameraModifier::OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV, bool bWasImmediate)
{
	Super::OnModifierDisabled_Implementation(LastPOV, bWasImmediate);
	RequestHelper.OnModifierDisabled(LastPOV, bWasImmediate);
}

void URPGArmLengthAnimNotifyCameraModifier::PushArmLengthAnimNotifyRequest(FGuid RequestId, float TargetLength, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve)
{
	RequestHelper.PushValueRequest(RequestId, TargetLength, TotalDuration, BlendInDuration, BlendInCurve, BlendOutDuration, BlendOutCurve);
}

void URPGArmLengthAnimNotifyCameraModifier::PopArmLengthAnimNotifyRequest(FGuid RequestId)
{
	RequestHelper.PopValueRequest(RequestId);
}