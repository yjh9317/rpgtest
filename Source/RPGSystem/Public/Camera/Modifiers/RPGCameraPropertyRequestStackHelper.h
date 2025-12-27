// Copyright(c) Aurora Devs 2022-2025. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/Build.h"
#include "Misc/Guid.h"
#include "Camera/CameraTypes.h"

class URPGCameraModifier;

template<typename ValueType>
struct RPGCameraPropertyValueRequest
{
public:
	bool IsValid() const { return RequestId.IsValid(); }
	void Invalidate();

public:
	FGuid RequestId = FGuid();

	ValueType TargetValue = {};
	float TotalDuration = 0.f;
	float BlendInDuration = 0.f;
	float BlendOutDuration = 0.f;
	float CurrentTime = 0.f;
	float BlendInCurrentTime = 0.f;
	float BlendOutCurrentTime = 0.f;

	class UCurveFloat* BlendInCurve = nullptr;
	class UCurveFloat* BlendOutCurve = nullptr;

	bool bBlendingIn = false;
	bool bBlendingOut = false;
};

/**
 * Camera Modifier Helper in charge of handling value change requests from Anim Notifies.
 */
template<typename ValueType>
class RPGSYSTEM_API RPGCameraPropertyRequestStackHelper final
{
public:
	void Init(const FName& PropertyName, const TFunction<bool()>& DebugGetterFunction);

public:
	void PushValueRequest(FGuid RequestId, ValueType TargetValue, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve);
	void PopValueRequest(FGuid RequestId);
	void ProcessValue(float DeltaTime, ValueType InValue, FVector ViewLocation, FRotator ViewRotation, ValueType& OutValue);
	void OnModifierDisabled(FMinimalViewInfo const& LastPOV, bool bWasImmediate);

protected:
	void InvalidateRequest(int32 RequestIndex);
	int32 FindInactiveRequest();

protected:
	friend class UUGC_FOVRequestAnimNotifyState;
	TArray<RPGCameraPropertyValueRequest<ValueType>> Requests;
	TFunction<bool()> DebugGetterFunction;

	FName PropertyName = TEXT("");

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
public:
	FLinearColor PropertyColor = FColor::Red;

protected:
	uint32 NumberActive = 0;
#endif
};

#include "RPGCameraPropertyRequestStackHelper.inl"