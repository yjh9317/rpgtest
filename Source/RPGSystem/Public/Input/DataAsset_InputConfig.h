// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "InputAction.h"
#include "DataAsset_InputConfig.generated.h"

/**
 * 입력 액션과 태그를 매핑하는 구조체
 */
USTRUCT(BlueprintType)
struct FInputActionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Input"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;
};

USTRUCT(BlueprintType)
struct FAbilityInputConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag ActionTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UInputAction> InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETriggerEvent TriggerEvent = ETriggerEvent::Started;
};


UCLASS()
class RPGSYSTEM_API UDataAsset_InputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	// 이동, 시점 변환 등 고정된 네이티브 입력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputTag"))
	TArray<FInputActionConfig> NativeInputActions;
	
	// 액션 컴포넌트 실행용 입력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "InputTag"))
	TArray<FAbilityInputConfig> AbilityInputActions;

	const UInputAction* FindNativeInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = true) const;
};