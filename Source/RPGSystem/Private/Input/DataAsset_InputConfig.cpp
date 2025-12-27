// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/DataAsset_InputConfig.h"

const UInputAction* UDataAsset_InputConfig::FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const FInputActionConfig& ActionConfig : NativeInputActions)
	{
		if (ActionConfig.InputTag == InputTag && ActionConfig.InputAction)
		{
			return ActionConfig.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't find NativeInputAction for InputTag [%s] on InputConfig [%s]"), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}