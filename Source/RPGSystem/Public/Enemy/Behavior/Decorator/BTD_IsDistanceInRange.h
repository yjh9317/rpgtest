// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IsDistanceInRange.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTD_IsDistanceInRange : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTD_IsDistanceInRange();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	float MinDistance = 0.0f;

	UPROPERTY(EditAnywhere, Category = "AI")
	float MaxDistance = 200.0f;
};
