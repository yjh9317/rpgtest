#pragma once
#include "Curves/CurveFloat.h"
#include "Engine/Engine.h"
#include "Containers/UnrealString.h"
#include "Templates/IsArithmetic.h"
#include "Templates/Decay.h"

template<typename ValueType>
void RPGCameraPropertyValueRequest<ValueType>::Invalidate()
{
	*this = RPGCameraPropertyValueRequest<ValueType>();
}

template<typename ValueType>
void RPGCameraPropertyRequestStackHelper<ValueType>::Init(const FName& InPropertyName, const TFunction<bool()>& BoolDebugGetterFunction)
{
	PropertyName = InPropertyName;
	DebugGetterFunction = BoolDebugGetterFunction;
}

template<typename ValueType>
void RPGCameraPropertyRequestStackHelper<ValueType>::PushValueRequest(FGuid RequestId, ValueType TargetValue, float TotalDuration, float BlendInDuration, UCurveFloat* BlendInCurve, float BlendOutDuration, UCurveFloat* BlendOutCurve)
{
	if (!RequestId.IsValid() || TotalDuration <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT(__FUNCTION__ ": Wrong request."));
		return;
	}

	RPGCameraPropertyValueRequest<ValueType> Request;
	Request.RequestId = RequestId;
	Request.CurrentTime = 0.f;
	Request.TargetValue = TargetValue;
	Request.TotalDuration = TotalDuration;

	Request.bBlendingIn = BlendInDuration > 0.f;
	Request.BlendInDuration = BlendInDuration;
	Request.BlendInCurve = BlendInCurve;
	Request.BlendInCurrentTime = 0.f;

	Request.bBlendingOut = false;
	Request.BlendOutDuration = BlendOutDuration;
	Request.BlendOutCurve = BlendOutCurve;
	Request.BlendOutCurrentTime = 0.f;

	const int32 NewIndex = FindInactiveRequest();
	check(NewIndex < MAX_uint16);
	Requests[NewIndex] = MoveTemp(Request);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	NumberActive++;
#endif
}

template<typename ValueType>
void RPGCameraPropertyRequestStackHelper<ValueType>::PopValueRequest(FGuid RequestId)
{
	int32 FoundIndex = -1;
	for (int32 Index = 0; Index < Requests.Num(); ++Index)
	{
		if (Requests[Index].RequestId == RequestId)
		{
			FoundIndex = Index;
			break;
		}
	}

	if (FoundIndex >= 0)
	{
		InvalidateRequest(FoundIndex);
	}
}

template<typename ValueType>
int32 RPGCameraPropertyRequestStackHelper<ValueType>::FindInactiveRequest()
{
	for (int32 Index = 0; Index < Requests.Num(); ++Index)
	{
		const RPGCameraPropertyValueRequest<ValueType>& Request(Requests[Index]);
		if (!Request.IsValid())
		{
			return Index;
		}
	}

	return Requests.Emplace();
}

template<typename ValueType>
void RPGCameraPropertyRequestStackHelper<ValueType>::InvalidateRequest(int32 RequestIndex)
{
	if (RequestIndex >= 0 && RequestIndex < Requests.Num())
	{
		Requests[RequestIndex].Invalidate();
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		--NumberActive;
#endif
	}
}

template<typename ValueType>
void RPGCameraPropertyRequestStackHelper<ValueType>::ProcessValue(float DeltaTime, ValueType InValue, FVector ViewLocation, FRotator ViewRotation, ValueType& OutValue)
{
	ValueType FinalValue = InValue;
	for (int32 Index = 0; Index < Requests.Num(); ++Index)
	{
		RPGCameraPropertyValueRequest<ValueType>& Request = Requests[Index];
		if (!Request.IsValid())
		{
			continue;
		}

		Request.CurrentTime += DeltaTime;

		// Advance any easing times.
		if (Request.bBlendingIn)
		{
			Request.BlendInCurrentTime += DeltaTime;
		}
		if (Request.bBlendingOut)
		{
			Request.BlendOutCurrentTime += DeltaTime;
		}

		// Start easing out if we're nearing the end.
		if (!Request.bBlendingOut)
		{
			const float BlendOutStartTime = Request.TotalDuration - Request.BlendOutDuration;
			if (Request.CurrentTime > BlendOutStartTime)
			{
				Request.bBlendingOut = true;
				Request.BlendOutCurrentTime = Request.CurrentTime - BlendOutStartTime;
			}
		}

		// Check if we're done easing in or out.
		bool bIsDoneEasingOut = false;
		if (Request.bBlendingIn)
		{
			if (Request.BlendInCurrentTime > Request.BlendInDuration || Request.BlendInDuration == 0.f)
			{
				Request.bBlendingIn = false;
			}
		}
		if (Request.bBlendingOut)
		{
			if (Request.BlendOutCurrentTime > Request.BlendOutDuration)
			{
				bIsDoneEasingOut = true;
			}
		}

		// Figure out the final easing weight.
		const float EasingInT = FMath::Clamp((Request.BlendInCurrentTime / Request.BlendInDuration), 0.f, 1.f);
		const float EasingInWeight = Request.bBlendingIn ?
			(Request.BlendInCurve ? Request.BlendInCurve->GetFloatValue(EasingInT) : EasingInT) : 1.f;

		const float EasingOutT = FMath::Clamp((1.f - Request.BlendOutCurrentTime / Request.BlendOutDuration), 0.f, 1.f);
		const float EasingOutWeight = Request.bBlendingOut ?
			(Request.BlendOutCurve ? Request.BlendOutCurve->GetFloatValue(EasingOutT) : EasingOutT) : 1.f;

		const float TotalEasingWeight = FMath::Min(EasingInWeight, EasingOutWeight);

		// We might be done playing or there was a DisableModifier() call with bImmediate=false to let a value request blend out.
		if (bIsDoneEasingOut || TotalEasingWeight <= 0.f)
		{
			InvalidateRequest(Index);
			continue;
		}

		// Using FinalValue so that having multiple value requests at the same time doesn't cause jumps in values.
		FinalValue = FMath::Lerp(FinalValue, Request.TargetValue, TotalEasingWeight);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
		if (DebugGetterFunction && DebugGetterFunction() && GEngine)
		{
			const float TextAlpha = (TotalEasingWeight * static_cast<float>(Index + 1) / static_cast<float>(NumberActive));
			const FLinearColor TextColor = FMath::Lerp(FLinearColor::White, PropertyColor, TextAlpha);

			FString ValueStr = "";
			if constexpr (TIsArithmetic<ValueType>::Value) ValueStr = LexToSanitizedString<FString, ValueType>(FinalValue);
			else ValueStr = FinalValue.ToCompactString();

			GEngine->AddOnScreenDebugMessage(-1, DeltaTime, TextColor.ToFColor(true),
				FString::Printf(TEXT("URPG%sAnimNotifyRequest: Notify %d - %s"), *PropertyName.ToString(), Index, *ValueStr));
		}
#endif
	}
	OutValue = FinalValue;
}

template<typename ValueType>
void RPGCameraPropertyRequestStackHelper<ValueType>::OnModifierDisabled(FMinimalViewInfo const& LastPOV, bool bWasImmediate)
{
	for (int32 Index = 0; Index < Requests.Num(); ++Index)
	{
		if (!Requests[Index].IsValid())
		{
			continue;
		}

		if (bWasImmediate)
		{
			InvalidateRequest(Index);
		}
		else if (!Requests[Index].bBlendingOut)
		{
			Requests[Index].bBlendingOut = true;
		}
	}
}
