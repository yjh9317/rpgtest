// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Service/BTS_UpdateTargetInfo.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTS_UpdateTargetInfo::UBTS_UpdateTargetInfo()
{
	NodeName = "Update Target Info";
	Interval = 0.5f; // 0.5초마다 갱신
}

void UBTS_UpdateTargetInfo::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
	if (!AIC || !Blackboard) return;

	AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (Target)
	{
		// 1. 거리 계산
		float Dist = AIC->GetPawn()->GetDistanceTo(Target);
		Blackboard->SetValueAsFloat(DistanceKey.SelectedKeyName, Dist);

		// 2. 시야(Line Of Sight) 체크
		bool bCanSee = AIC->LineOfSightTo(Target);
		Blackboard->SetValueAsBool(HasLineOfSightKey.SelectedKeyName, bCanSee);
	}
}
