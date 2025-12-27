// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Decorator/BTD_CanExecuteActionByTag.h"
#include "AIController.h"
#include "Combat/Action/ActionBTUtils.h"
#include "Combat/Action/BaseAction.h"
#include "Combat/Action/Components/ActionComponent.h"

UBTD_CanExecuteActionByTag::UBTD_CanExecuteActionByTag(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NodeName = TEXT("Can Execute Action By Tag");
}

bool UBTD_CanExecuteActionByTag::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
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

	UBaseAction* Action = ActionComp->GetAction(ActionTag);
	if (!Action)
	{
		return false;
	}

	// 이미 활성 중이면 실행 불가
	if (Action->IsActive())
	{
		return false;
	}

	// 쿨다운/리소스/조건은 CanExecute에 위임 (BaseAction 쪽 구현 사용)
	return Action->CanExecute();
}