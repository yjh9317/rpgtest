// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_UpdateTargetInfo.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTS_UpdateTargetInfo : public UBTService
{
	GENERATED_BODY()
public:
	UBTS_UpdateTargetInfo();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector DistanceKey;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector HasLineOfSightKey;
};
