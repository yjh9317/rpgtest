// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Task/BTT_WaitForAction.h"

#include "AIController.h"
#include "Combat/Action/Components/ActionComponent.h"


UBTT_WaitForAction::UBTT_WaitForAction(const FObjectInitializer& ObjectInitializer)
	: UBTT_BaseActionTask(ObjectInitializer)
{
	NodeName = TEXT("Wait For Action");

	bNotifyTick = true;
	bCreateNodeInstance = true; // 인스턴스별 상태(ElapsedTime)를 가지려면 필요
}

EBTNodeResult::Type UBTT_WaitForAction::ExecuteActionTask(
	UBehaviorTreeComponent& OwnerComp,
	UActionComponent* ActionComp,
	APawn* ControlledPawn)
{
	ElapsedTime = 0.0f;

	if (!ActionTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	// 이미 액션이 비활성이면 바로 성공 처리
	if (!ActionComp->IsActionActive(ActionTag))
	{
		return EBTNodeResult::Succeeded;
	}

	// 아직 돌고 있으면 Running으로 유지 → TickTask에서 감시
	return EBTNodeResult::InProgress;
}

void UBTT_WaitForAction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	ElapsedTime += DeltaSeconds;

	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	UActionComponent* ActionComp = ControlledPawn->FindComponentByClass<UActionComponent>();
	if (!ActionComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 액션이 끝났으면 성공
	if (!ActionComp->IsActionActive(ActionTag))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}

	// 타임아웃 보호
	if (Timeout > 0.f && ElapsedTime >= Timeout)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
}