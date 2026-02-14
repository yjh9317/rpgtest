#pragma once

#include "CoreMinimal.h"
#include "Enemy/Behavior/Task/BTT_BaseActionTask.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BTT_ExecuteSelectedActionName.generated.h"

/** Executes action selected by utility scorer using Blackboard Name key. */
UCLASS()
class RPGSYSTEM_API UBTT_ExecuteSelectedActionName : public UBTT_BaseActionTask
{
    GENERATED_BODY()

public:
    UBTT_ExecuteSelectedActionName(const FObjectInitializer& ObjectInitializer);

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector SelectedActionNameKey;

    virtual EBTNodeResult::Type ExecuteActionTask(
        UBehaviorTreeComponent& OwnerComp,
        UActionComponent* ActionComp,
        APawn* ControlledPawn) override;
};
