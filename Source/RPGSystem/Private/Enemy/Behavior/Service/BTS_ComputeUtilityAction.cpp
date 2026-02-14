#include "Enemy/Behavior/Service/BTS_ComputeUtilityAction.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Combat/Action/ActionBTUtils.h"
#include "Combat/Action/BaseAction.h"

namespace
{
    struct FScoredUtilityAction
    {
        FGameplayTag ActionTag;
        float Score = 0.0f;
    };
}

UBTS_ComputeUtilityAction::UBTS_ComputeUtilityAction()
{
    NodeName = TEXT("Compute Utility Action");
    Interval = 0.2f;
}

void UBTS_ComputeUtilityAction::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!Blackboard || !AIController)
    {
        return;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        Blackboard->SetValueAsName(SelectedActionNameKey.SelectedKeyName, NAME_None);
        return;
    }

    UActionComponent* ActionComp = ActionBTUtils::GetActionComponentFromActor(ControlledPawn);
    if (!ActionComp)
    {
        Blackboard->SetValueAsName(SelectedActionNameKey.SelectedKeyName, NAME_None);
        return;
    }

    const bool bHasLOS = Blackboard->GetValueAsBool(HasLineOfSightKey.SelectedKeyName);

    float Distance = Blackboard->GetValueAsFloat(DistanceKey.SelectedKeyName);
    AActor* Target = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (Distance <= 0.0f && Target)
    {
        Distance = ControlledPawn->GetDistanceTo(Target);
    }

    TArray<FScoredUtilityAction> Candidates;
    Candidates.Reserve(ActionOptions.Num());

    for (const FUtilityActionOption& Option : ActionOptions)
    {
        if (!Option.ActionTag.IsValid())
        {
            continue;
        }

        UBaseAction* Action = ActionComp->GetAction(Option.ActionTag);
        if (!Action)
        {
            continue;
        }

        if (!ActionComp->CheckTagRequirements(Action) || !Action->CanExecute())
        {
            continue;
        }

        if (Option.bRequireLineOfSight && !bHasLOS)
        {
            continue;
        }

        if (Distance < Option.MinDistance || Distance > Option.MaxDistance)
        {
            continue;
        }

        float Score = Option.BaseScore;

        const float DistanceWindow = FMath::Max(Option.MaxDistance - Option.MinDistance, KINDA_SMALL_NUMBER);
        const float DistanceDelta = FMath::Abs(Distance - Option.PreferredDistance);
        const float NormalizedDistanceFit = 1.0f - FMath::Clamp(DistanceDelta / DistanceWindow, 0.0f, 1.0f);
        Score += NormalizedDistanceFit * Option.PreferredDistanceWeight;

        if (Option.RandomJitter > 0.0f)
        {
            Score += FMath::FRandRange(-Option.RandomJitter, Option.RandomJitter);
        }

        if (Score <= 0.0f)
        {
            continue;
        }

        FScoredUtilityAction& NewCandidate = Candidates.AddDefaulted_GetRef();
        NewCandidate.ActionTag = Option.ActionTag;
        NewCandidate.Score = Score;
    }

    if (Candidates.IsEmpty())
    {
        Blackboard->SetValueAsName(SelectedActionNameKey.SelectedKeyName, NAME_None);
        return;
    }

    Candidates.Sort([](const FScoredUtilityAction& A, const FScoredUtilityAction& B)
    {
        return A.Score > B.Score;
    });

    FGameplayTag SelectedActionTag = Candidates[0].ActionTag;

    if (bUseWeightedRandomFromTopN)
    {
        const int32 CandidateCount = FMath::Clamp(TopNForRandom, 1, Candidates.Num());
        float TotalScore = 0.0f;
        for (int32 i = 0; i < CandidateCount; ++i)
        {
            TotalScore += Candidates[i].Score;
        }

        if (TotalScore > 0.0f)
        {
            float Roll = FMath::FRandRange(0.0f, TotalScore);
            for (int32 i = 0; i < CandidateCount; ++i)
            {
                Roll -= Candidates[i].Score;
                if (Roll <= 0.0f)
                {
                    SelectedActionTag = Candidates[i].ActionTag;
                    break;
                }
            }
        }
    }

    Blackboard->SetValueAsName(SelectedActionNameKey.SelectedKeyName, SelectedActionTag.GetTagName());
}
