// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_FocusTarget.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTS_FocusTarget : public UBTService
{
	GENERATED_BODY()
public:
	UBTS_FocusTarget();
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    
	// 서비스가 끝날 때(노드를 벗어날 때) 주시를 풀기 위해 필요
	virtual void OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override; 

	UPROPERTY(EditAnywhere, Category = "AI")
	FBlackboardKeySelector TargetActorKey;
};
