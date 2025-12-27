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
	return 0.f;
	// return CurrentValue.Get<FVector>() * PlayerCameraManager->GetCameraTurnRate();
}
