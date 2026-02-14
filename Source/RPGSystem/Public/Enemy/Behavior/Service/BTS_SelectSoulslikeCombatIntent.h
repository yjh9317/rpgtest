#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTS_SelectSoulslikeCombatIntent.generated.h"

UENUM(BlueprintType)
enum class ESoulslikeCombatPhase : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Approach UMETA(DisplayName = "Approach"),
    Strafe UMETA(DisplayName = "Strafe"),
    Attack UMETA(DisplayName = "Attack"),
    Recover UMETA(DisplayName = "Recover")
};

/**
 * BT service sample for soulslike decision layering.
 * - Enum key: coarse combat phase
 * - Name key: fine-grained intent tag (data-driven action lookup key)
 */
UCLASS()
class RPGSYSTEM_API UBTS_SelectSoulslikeCombatIntent : public UBTService
{
    GENERATED_BODY()

public:
    UBTS_SelectSoulslikeCombatIntent();

    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector DistanceKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector HasLineOfSightKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector CombatPhaseKey;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector ActionIntentNameKey;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta=(ClampMin="0.0"))
    float AttackRange = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config", meta=(ClampMin="0.0"))
    float StrafeRange = 650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    FName CloseAttackIntent = TEXT("AI.Intent.Attack.Light");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    FName GapCloseIntent = TEXT("AI.Intent.Movement.Approach");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    FName StrafeIntent = TEXT("AI.Intent.Movement.Strafe");
};
