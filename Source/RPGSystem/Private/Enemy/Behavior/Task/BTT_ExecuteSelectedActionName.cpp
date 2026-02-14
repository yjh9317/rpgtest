#include "Enemy/Behavior/Task/BTT_ExecuteSelectedActionName.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Combat/Action/Components/ActionComponent.h"
#include "GameplayTagsManager.h"

UBTT_ExecuteSelectedActionName::UBTT_ExecuteSelectedActionName(const FObjectInitializer& ObjectInitializer)
    : UBTT_BaseActionTask(ObjectInitializer)
{
    NodeName = TEXT("Execute Selected Action Name");
}

EBTNodeResult::Type UBTT_ExecuteSelectedActionName::ExecuteActionTask(
    UBehaviorTreeComponent& OwnerComp,
    UActionComponent* ActionComp,
    APawn* ControlledPawn)
{
    if (!ActionComp)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        return EBTNodeResult::Failed;
    }

    const FName SelectedActionName = Blackboard->GetValueAsName(SelectedActionNameKey.SelectedKeyName);
    if (SelectedActionName.IsNone())
    {
        return EBTNodeResult::Failed;
    }

    const FGameplayTag ActionTag = UGameplayTagsManager::Get().RequestGameplayTag(SelectedActionName, false);
    if (!ActionTag.IsValid())
    {
        return EBTNodeResult::Failed;
    }

    const bool bExecuted = ActionComp->ExecuteAction(ActionTag);
    return bExecuted ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
