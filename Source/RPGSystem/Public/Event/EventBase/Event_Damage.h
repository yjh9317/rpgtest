// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Event/EventBase/RPGEventBase.h"
#include "Event_Damage.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UEvent_Damage : public URPGEventBase
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Category="Damage")
	float DamageAmount = 0.f;

	UPROPERTY(BlueprintReadWrite, Category="Damage")
	FGameplayTag DamageTypeTag;
};
