#include "Enemy/Behavior/Service/BTS_SelectSoulslikeCombatIntent.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTS_SelectSoulslikeCombatIntent::UBTS_SelectSoulslikeCombatIntent()
{
    NodeName = "Select Soulslike Combat Intent";
    Interval = 0.2f;
}

void UBTS_SelectSoulslikeCombatIntent::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!Blackboard || !AIController)
    {
        return;
    }

    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    const bool bHasLOS = Blackboard->GetValueAsBool(HasLineOfSightKey.SelectedKeyName);
    const float Distance = Blackboard->GetValueAsFloat(DistanceKey.SelectedKeyName);

    ESoulslikeCombatPhase NextPhase = ESoulslikeCombatPhase::Idle;
    FName NextIntent = NAME_None;

    if (!Target || !bHasLOS)
    {
        NextPhase = ESoulslikeCombatPhase::Approach;
        NextIntent = GapCloseIntent;
    }
    else if (Distance <= AttackRange)
    {
        NextPhase = ESoulslikeCombatPhase::Attack;
        NextIntent = CloseAttackIntent;
    }
    else if (Distance <= StrafeRange)
    {
        NextPhase = ESoulslikeCombatPhase::Strafe;
        NextIntent = StrafeIntent;
    }
    else
    {
        NextPhase = ESoulslikeCombatPhase::Approach;
        NextIntent = GapCloseIntent;
    }

    Blackboard->SetValueAsEnum(CombatPhaseKey.SelectedKeyName, static_cast<uint8>(NextPhase));
    Blackboard->SetValueAsName(ActionIntentNameKey.SelectedKeyName, NextIntent);
}
