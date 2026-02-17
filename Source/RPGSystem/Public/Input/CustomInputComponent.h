// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset_InputConfig.h"
#include "EnhancedInputComponent.h"
#include "CustomInputComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class RPGSYSTEM_API UCustomInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	// 태그를 기반으로 네이티브 함수 바인딩 (템플릿 함수)
	template<class UserClass, typename FuncType>
	void BindNativeAction(const UDataAsset_InputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound = true);
	
	template<class UserClass, typename FuncType>
	void BindAbilityActions(const UDataAsset_InputConfig* InputConfig, UserClass* Object, FuncType Func);
};

template<class UserClass, typename FuncType>
void UCustomInputComponent::BindNativeAction(const UDataAsset_InputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, bool bLogIfNotFound)
{
	check(InputConfig);
	if (const UInputAction* IA = InputConfig->FindNativeInputActionForTag(InputTag, bLogIfNotFound))
	{
		BindAction(IA, TriggerEvent, Object, Func);
	}
}

template<class UserClass, typename FuncType>
void UCustomInputComponent::BindAbilityActions(const UDataAsset_InputConfig* InputConfig, UserClass* Object, FuncType Func)
{
	check(InputConfig);

	for (const FAbilityInputConfig& AbilityConfig : InputConfig->AbilityInputActions)
	{
		if (AbilityConfig.InputAction && AbilityConfig.ActionTag.IsValid())
		{
			TArray<ETriggerEvent> TriggerEvents;
			TriggerEvents.AddUnique(AbilityConfig.TriggerEvent);
			TriggerEvents.AddUnique(ETriggerEvent::Started);
			TriggerEvents.AddUnique(ETriggerEvent::Triggered);
			TriggerEvents.AddUnique(ETriggerEvent::Completed);
			TriggerEvents.AddUnique(ETriggerEvent::Canceled);

			for (const ETriggerEvent TriggerEvent : TriggerEvents)
			{
				BindAction(AbilityConfig.InputAction, TriggerEvent, Object, Func, AbilityConfig.ActionTag);
			}
		}
	}
}
