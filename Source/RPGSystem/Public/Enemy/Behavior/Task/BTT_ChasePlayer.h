// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_ChasePlayer.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTT_ChasePlayer : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTT_ChasePlayer();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;
    
	UPROPERTY(EditAnywhere, Category = "AI")
	float AcceptanceRadius = 150.f; // 이 거리 안으로 들어가면 멈춤
};
