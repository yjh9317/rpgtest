// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DataAsset_StatConfig.generated.h"

USTRUCT(BlueprintType)
struct FStatConfig
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, meta = (Categories = "Character.Stats"))
	FGameplayTag StatTag;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float BaseValue = 0.0f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float MaxValue = 0.0f;

	FStatConfig()
	: BaseValue(0.0f)
	, MaxValue(0.0f)
	{}

	FStatConfig(const FGameplayTag& InTag, float InBase, float InMax)
		: StatTag(InTag)
		, BaseValue(InBase)
		, MaxValue(InMax > 0.0f ? InMax : InBase)
	{}

	bool IsValid() const
	{
		return StatTag.IsValid() && BaseValue >= 0.0f;
	}
};

UCLASS()
class RPGSYSTEM_API UDataAsset_StatConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "StatTag"))
	TArray<FStatConfig> StatConfigs;
};
