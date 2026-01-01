// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "DataAsset_ActionConfig.generated.h"

class UBaseAction;

USTRUCT(BlueprintType)
struct FActionDefinition
{
	GENERATED_BODY()

	// 이 액션을 실행할 때 사용할 태그 (예: Action.Attack)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Action"))
	FGameplayTag ActionTag;

	// 실행될 액션 클래스 (예: BP_MeleeAttack)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UBaseAction> ActionClass;
};

UCLASS()
class RPGSYSTEM_API UDataAsset_ActionConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Meta = (TitleProperty = "ActionTag"))
	TArray<FActionDefinition> DefaultActions;
};
