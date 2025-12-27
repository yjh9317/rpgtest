// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Task/BTT_BaseActionTask.h"

#include "AIController.h"
#include "Combat/Action/ActionBTUtils.h"


UBTT_BaseActionTask::UBTT_BaseActionTask(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Base Action Task");
}

EBTNodeResult::Type UBTT_BaseActionTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		return EBTNodeResult::Failed;
	}

	UActionComponent* ActionComp = ActionBTUtils::GetActionComponentFromActor(ControlledPawn);
	if (!ActionComp)
	{
		return EBTNodeResult::Failed;
	}

	return ExecuteActionTask(OwnerComp, ActionComp, ControlledPawn);
}

EBTNodeResult::Type UBTT_BaseActionTask::ExecuteActionTask(
	UBehaviorTreeComponent& OwnerComp,
	UActionComponent* ActionComp,
	APawn* ControlledPawn)
{
	// 기본 구현은 실패, 자식에서 오버라이드
	return EBTNodeResult::Failed;
}