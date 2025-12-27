// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "InputMappingData.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UInputMappingData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Mapping")
	TMap<FGameplayTag, FGameplayTag> InputToActionMap;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Mapping")
	FString MappingName = "Default";
};
