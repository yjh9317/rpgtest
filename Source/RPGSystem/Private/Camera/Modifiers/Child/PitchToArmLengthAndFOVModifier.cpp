// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Modifiers/Child/PitchToArmLengthAndFOVModifier.h"

#include "Camera/RPGPlayerCameraManager.h"
#include "Kismet/KismetMathLibrary.h"

void UPitchToArmLengthAndFOVModifier::OnModifierDisabled_Implementation(FMinimalViewInfo const& LastPOV,
                                                                        bool bWasImmediate)
{
	PreDisabledArmLength = CurrentArmLength;
}

void UPitchToArmLengthAndFOVModifier::OnModifierEnabled_Implementation(FMinimalViewInfo const& LastPOV)
{
	PreEnabledFOV = LastPOV.FOV;
}

void UPitchToArmLengthAndFOVModifier::OnAnyLevelSequenceStarted_Implementation()
{
	EnableModifier();
	bCanProcessViewTargetChanges = false;
}

void UPitchToArmLengthAndFOVModifier::OnAnyLevelSequenceEnded_Implementation()
{
	DisableModifier(true);
	bCanProcessViewTargetChanges = true;
}

void UPitchToArmLengthAndFOVModifier::OnSetViewTarget_Implementation(bool bImmediate, bool bNewTargetIsOwner)
{
	if(bNewTargetIsOwner)
	{
		if(bCanProcessViewTargetChanges)
		{
			EnableModifier();
		}
	}
	else
	{
		DisableModifier(true);
	}
}

void UPitchToArmLengthAndFOVModifier::ProcessBoomLengthAndFOV_Implementation(float DeltaTime, float InFOV, float InArmLength,
                                                              FVector ViewLocation, FRotator ViewRotation, float& OutFOV, float& OutArmLength)
{
	if (!RPGCameraManager)
	{
		return;
	}
	float FOV = 0.f;
	float Length = 0.f;
	Super::ProcessBoomLengthAndFOV_Implementation(DeltaTime,InFOV,InArmLength,ViewLocation,ViewRotation,FOV,Length);

	constexpr float Epsilon = 1.e-6f;
	if(RPGCameraManager)
	{
		if(!RPGCameraManager->IsPlayingAnyLevelSequence() && CurveSettings.Enabled && CurveSettings.PitchToArmLengthCurve && CurveSettings.PitchToFOVCurve)
		{
			// Validate curves: the curves' time range should go from -1.0 to 1.0 so that we can map the Min Pitch and Max Pitch to the X axis
			double ArmLengthMinTime = 0.f;
			double ArmLengthMaxTime = 0.f;
			CurveSettings.PitchToArmLengthCurve->GetTimeRange(ArmLengthMinTime, ArmLengthMaxTime);
			const bool bIsArmLengthCurveTimeRangeValid =
				UKismetMathLibrary::NearlyEqual_FloatFloat(ArmLengthMinTime, -1.f, Epsilon) &&
				UKismetMathLibrary::NearlyEqual_FloatFloat(ArmLengthMaxTime, 1.f, Epsilon);

			double FOVMinTime = 0.f;
			double FOVMaxTime = 0.f;
			CurveSettings.PitchToFOVCurve->GetTimeRange(FOVMinTime, FOVMaxTime);
			const bool bIsFOVCurveTimeRangeValid =
				UKismetMathLibrary::NearlyEqual_FloatFloat(FOVMinTime, -1.f, Epsilon) &&
				UKismetMathLibrary::NearlyEqual_FloatFloat(FOVMaxTime, 1.f, Epsilon);

			double ArmLengthMinValue = 0.f;
			double ArmLengthMaxValue = 0.f;
			CurveSettings.PitchToArmLengthCurve->GetValueRange(ArmLengthMinValue, ArmLengthMaxValue);
			const bool bIsArmLengthCurveValueRangeValid =
				UKismetMathLibrary::NearlyEqual_FloatFloat(ArmLengthMinValue, -1.f, Epsilon) &&
				UKismetMathLibrary::NearlyEqual_FloatFloat(ArmLengthMaxValue, 1.f, Epsilon);

			double FOVMinValue = 0.f;
			double FOVMaxValue = 0.f;
			CurveSettings.PitchToFOVCurve->GetValueRange(FOVMinValue, FOVMaxValue);
			const bool bIsFOVCurveValueRangeValid =
				UKismetMathLibrary::NearlyEqual_FloatFloat(FOVMinValue, -1.f, Epsilon) &&
				UKismetMathLibrary::NearlyEqual_FloatFloat(FOVMaxValue, 1.f, Epsilon);

			if(bIsArmLengthCurveTimeRangeValid && bIsFOVCurveTimeRangeValid && bIsArmLengthCurveValueRangeValid && bIsFOVCurveValueRangeValid)
			{
				// TO DO: Use pitch max min abs
				float PitchRatio = UKismetMathLibrary::MapRangeClamped(ViewRotation.Pitch,RPGCameraManager->ViewPitchMin,RPGCameraManager->ViewPitchMax,-1.f,1.f);
				/*
				if(ARPG_ConfigurableCameraManager* ConfigurableCameraManager = Cast<ARPG_ConfigurableCameraManager>(RPGCameraManager))
				{
					// Get value from curve (between -1 and 1) and map it to the actual range
					float FOVCurveValue = CurveSettings.PitchToFOVCurve->GetFloatValue(PitchRatio);
					double MinFOV = 0; double MaxFOV = 0;
					ConfigurableCameraManager->GetCurrentFOVRange(MinFOV,MaxFOV);
					double ClampedFOVValue = UKismetMathLibrary::MapRangeClamped(FOVCurveValue,-1.f,1.f,MinFOV,MaxFOV);
					
					float ArmLengthCurveValue = CurveSettings.PitchToArmLengthCurve->GetFloatValue(PitchRatio);
					double MinArmLength = 0; double MaxArmLength = 0;
					ConfigurableCameraManager->GetCurrentArmLengthRange(MinArmLength,MaxArmLength);
					double ClampedArmLengthValue = UKismetMathLibrary::MapRangeClamped(ArmLengthCurveValue,-1.f,1.f,MinArmLength,MaxArmLength);
					// Toggle interpolation. This is useful when blending out of cutscenes.
					FOV = UKismetMathLibrary::Lerp(PreEnabledFOV,ClampedFOVValue,Alpha);
					Length = UKismetMathLibrary::Lerp(PreDisabledArmLength,ClampedArmLengthValue,Alpha);
				}
				*/
			}
		}
	}
	OutFOV = FOV;
	OutArmLength = Length;
}
