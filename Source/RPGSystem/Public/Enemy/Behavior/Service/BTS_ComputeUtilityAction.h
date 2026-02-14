#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "GameplayTagContainer.h"
#include "BTS_ComputeUtilityAction.generated.h"

USTRUCT(BlueprintType)
struct FUtilityActionOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility")
    FGameplayTag ActionTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility", meta=(ClampMin="0.0"))
    float BaseScore = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility")
    bool bRequireLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility", meta=(ClampMin="0.0"))
    float MinDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility", meta=(ClampMin="0.0"))
    float MaxDistance = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility", meta=(ClampMin="0.0"))
    float PreferredDistance = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility", meta=(ClampMin="0.0"))
    float PreferredDistanceWeight = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility", meta=(ClampMin="0.0"))
    float RandomJitter = 0.15f;
};

/**
 * Utility scorer for action selection.
 * Outputs selected action into a blackboard Name key (GameplayTag name string).
 */
UCLASS()
class RPGSYSTEM_API UBTS_ComputeUtilityAction : public UBTService
{
    GENERATED_BODY()

public:
    UBTS_ComputeUtilityAction();
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DistanceKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector HasLineOfSightKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector SelectedActionNameKey;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility")
    TArray<FUtilityActionOption> ActionOptions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility")
    bool bUseWeightedRandomFromTopN = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Utility", meta=(ClampMin="1"))
    int32 TopNForRandom = 3;
};
