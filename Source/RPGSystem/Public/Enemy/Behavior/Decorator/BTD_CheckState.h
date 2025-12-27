// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "Enemy/EnemyController.h"
#include "BTD_CheckState.generated.h"

/**
 * 
 */
UCLASS()
class RPGSYSTEM_API UBTD_CheckState : public UBTDecorator
{
	GENERATED_BODY()
public:
	UBTD_CheckState();
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

	UPROPERTY(EditAnywhere, Category = "AI")
	EEnemyAIState TargetState;
};
