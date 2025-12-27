// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Decorator/BTD_HasAnyActiveActions.h"
#include "AIController.h"
#include "Combat/Action/ActionBTUtils.h"
#include "Combat/Action/Components/ActionComponent.h"

UBTD_HasAnyActiveActions::UBTD_HasAnyActiveActions(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Has Any Active Actions");
}

bool UBTD_HasAnyActiveActions::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
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

	const bool bHasAny = ActionComp->HasAnyActiveActions();
	return bInvert ? !bHasAny : bHasAny;
}