// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Decorator/BTD_IsActionActive.h"

#include "AIController.h"
#include "Combat/Action/ActionBTUtils.h"

UBTD_IsActionActive::UBTD_IsActionActive(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Is Action Active");
}

bool UBTD_IsActionActive::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	if (!ActionTag.IsValid())
	{
		return false;
	}

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return false;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return false;
	}

	UActionComponent* ActionComp = ActionBTUtils::GetActionComponentFromActor(ControlledPawn);
	if (!ActionComp)
	{
		return false;
	}

	const bool bActive = ActionComp->IsActionActive(ActionTag);
	return bInvert ? !bActive : bActive;
}