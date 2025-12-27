
#pragma once

#include "CoreMinimal.h"
#include "RPGCameraModifier.h"
#include "RPGCameraPropertyRequestStackHelper.h"
#include "RPGCameraNotifyModifiers.generated.h"

/**
 * Camera Modifier in charge of handling FOV change requests from Anim Notifies.
 */
UCLASS(abstract, ClassGroup = "UGC Camera Modifiers")
class RPGSYSTEM_API URPGFOVAnimNotifyCameraModifier : public URPGCameraModifier
{
	GENERATED_BODY()

public:
	URPGFOVAnimNotifyCameraModifier();

protected:
	void PushFOVAnimNotifyRequest(FGuid RequestId, float TargetFOV, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve);
	void PopFOVAnimNotifyRequest(FGuid RequestId);

protected:
	void ProcessBoomLengthAndFOV_Implementation(float DeltaTime, float InFOV, float InArmLength, FVector ViewLocation, FRotator ViewRotation, float& OutFOV, float& OutArmLength) override;
	void OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV, bool bWasImmediate);

protected:
	friend class URPGFOVRequestAnimNotifyState;
	RPGCameraPropertyRequestStackHelper<float> RequestHelper;
};

/**
 * Camera Modifier in charge of handling Arm Offset changes requests from Anim Notifies.
 */
UCLASS(abstract, ClassGroup = "UGC Camera Modifiers")
class RPGSYSTEM_API URPGArmOffsetAnimNotifyCameraModifier : public URPGCameraModifier
{
	GENERATED_BODY()

public:
	URPGArmOffsetAnimNotifyCameraModifier();

protected:
	void PushArmSocketOffsetAnimNotifyRequest(FGuid RequestId, FVector TargetOffset, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve);
	void PopArmSocketOffsetAnimNotifyRequest(FGuid RequestId);

	void PushArmTargetOffsetAnimNotifyRequest(FGuid RequestId, FVector TargetOffset, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve);
	void PopArmTargetOffsetAnimNotifyRequest(FGuid RequestId);

protected:
	void ProcessBoomOffsets_Implementation(float DeltaTime, FVector InSocketOffset, FVector InTargetOffset, FVector ViewLocation, FRotator ViewRotation, FVector& OutSocketOffset, FVector& OutTargetOffset) override;
	void OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV, bool bWasImmediate);

protected:
	friend class URPGArmOffsetRequestAnimNotifyState;
	RPGCameraPropertyRequestStackHelper<FVector> SocketOffsetRequestHelper;
	RPGCameraPropertyRequestStackHelper<FVector> TargetOffsetRequestHelper;
};

/**
 * Camera Modifier in charge of handling Arm Length changes requests from Anim Notifies.
 */
UCLASS(abstract, ClassGroup = "UGC Camera Modifiers")
class RPGSYSTEM_API URPGArmLengthAnimNotifyCameraModifier : public URPGCameraModifier
{
	GENERATED_BODY()

public:
	URPGArmLengthAnimNotifyCameraModifier();

protected:
	void PushArmLengthAnimNotifyRequest(FGuid RequestId, float TargetLength, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve);
	void PopArmLengthAnimNotifyRequest(FGuid RequestId);

protected:
	void ProcessBoomLengthAndFOV_Implementation(float DeltaTime, float InFOV, float InArmLength, FVector ViewLocation, FRotator ViewRotation, float& OutFOV, float& OutArmLength) override;
	void OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV, bool bWasImmediate);

protected:
	friend class URPGArmLengthRequestAnimNotifyState;
	RPGCameraPropertyRequestStackHelper<float> RequestHelper;
};