// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/Input/RPGCameraTurnRateModifier.h"

#include "EnhancedPlayerInput.h"
#include "Camera/RPGPlayerCameraManager.h"


FInputActionValue URPGCameraTurnRateModifier::ModifyRaw_Implementation(const UEnhancedPlayerInput* PlayerInput, FInputActionValue CurrentValue, float DeltaTime)
{
	EInputActionValueType ValueType = CurrentValue.GetValueType();
	if (ValueType == EInputActionValueType::Boolean)
	{
		return CurrentValue;
	}

	if (!PlayerCameraManager)
	{
		if (!PlayerInput->GetOuterAPlayerController() || !PlayerInput->GetOuterAPlayerController()->PlayerCameraManager)
		{
#if ENABLE_DRAW_DEBUG
			// Debugging
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("UGC_CameraSlowDownInputModifier: Could not find Player Camera Manager to use Camera Slow Down constraint.")));
			}
#endif
			return CurrentValue;
		}

		ARPGPlayerCameraManager* PCManager = Cast<ARPGPlayerCameraManager>(PlayerInput->GetOuterAPlayerController()->PlayerCameraManager);
		if (!PCManager)
		{
#if ENABLE_DRAW_DEBUG
			// Debugging
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(-1, DeltaTime, FColor::Red, FString::Printf(TEXT("UGC_CameraSlowDownInputModifier: Player's Camera Manager does not inherit from UGC_PlayerCameraManager. Angle Constraint cannot be used.")));
			}
#endif
			return CurrentValue;
		}
		// Here we know that it's not null
		PlayerCameraManager = PCManager;
	}

	const FVector TurnRate = PlayerCameraManager->GetCameraTurnRate();
	switch (ValueType)
	{
	case EInputActionValueType::Axis1D:
		return CurrentValue.Get<float>() * TurnRate.X;
	case EInputActionValueType::Axis2D:
	{
		const FVector2D Value = CurrentValue.Get<FVector2D>();
		return FVector2D(Value.X * TurnRate.X, Value.Y * TurnRate.Y);
	}
	case EInputActionValueType::Axis3D:
	{
		const FVector Value = CurrentValue.Get<FVector>();
		return FVector(Value.X * TurnRate.X, Value.Y * TurnRate.Y, Value.Z * TurnRate.Z);
	}
	default:
		return CurrentValue;
	}
}
