// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Decorator/BTD_CheckState.h"
#include "Enemy/EnemyController.h"

UBTD_CheckState::UBTD_CheckState()
{
	NodeName = "Is Current State?";
}

bool UBTD_CheckState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	AEnemyController* AIC = Cast<AEnemyController>(OwnerComp.GetAIOwner());
	if (AIC)
	{
		return AIC->GetCurrentState() == TargetState;
	}
	return false;
}