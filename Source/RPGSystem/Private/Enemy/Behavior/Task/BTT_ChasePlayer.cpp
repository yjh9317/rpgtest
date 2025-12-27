// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Task/BTT_ChasePlayer.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UBTT_ChasePlayer::UBTT_ChasePlayer()
{
	NodeName = "Chase Player";
}

EBTNodeResult::Type UBTT_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    
	if (AIC && BB)
	{
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (Target)
		{
			// MoveToActor는 내부적으로 비동기 처리되지만, Simple MoveTo 형태로 호출시 즉시 결과를 반환하기도 합니다.
			// EPathFollowingRequestResult::RequestSuccessful 등을 체크 가능
			EPathFollowingRequestResult::Type Result = AIC->MoveToActor(Target, AcceptanceRadius);
			
			if (Result == EPathFollowingRequestResult::Type::RequestSuccessful || Result == EPathFollowingRequestResult::Type::AlreadyAtGoal)
			{
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
}
