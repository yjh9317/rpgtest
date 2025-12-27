// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Behavior/Service/BTS_FocusTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTS_FocusTarget::UBTS_FocusTarget()
{
	NodeName = "Focus Target";
}

void UBTS_FocusTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	
	if (AIC && BB)
	{
		AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
		if (Target)
		{
			AIC->SetFocus(Target); // 항상 타겟을 바라보게 함
		}
	}
}

void UBTS_FocusTarget::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
    
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC)
	{
		AIC->ClearFocus(EAIFocusPriority::Gameplay); // 주시 해제
	}
}
