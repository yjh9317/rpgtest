// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Task/BTT_ExecuteActionByTag.h"

#include "Combat/Action/Components/ActionComponent.h"

UBTT_ExecuteActionByTag::UBTT_ExecuteActionByTag(const FObjectInitializer& ObjectInitializer)
	: UBTT_BaseActionTask(ObjectInitializer)
{
	NodeName = TEXT("Execute Action By Tag");
}

EBTNodeResult::Type UBTT_ExecuteActionByTag::ExecuteActionTask(
	UBehaviorTreeComponent& OwnerComp,
	UActionComponent* ActionComp,
	APawn* ControlledPawn)
{
	if (!ActionTag.IsValid() || !ActionComp)
	{
		return EBTNodeResult::Failed;
	}

	const bool bResult = ActionComp->ExecuteAction(ActionTag);
	return bResult ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}