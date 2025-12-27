// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Decorator/BTD_IsDistanceInRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTD_IsDistanceInRange::UBTD_IsDistanceInRange()
{
	NodeName = "Distance Check";
}

bool UBTD_IsDistanceInRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	AAIController* AIC = OwnerComp.GetAIOwner();
    
	if (BB && AIC && AIC->GetPawn())
	{
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (Target)
		{
			float Dist = AIC->GetPawn()->GetDistanceTo(Target);
			return (Dist >= MinDistance && Dist <= MaxDistance);
		}
	}
	return false;
}
