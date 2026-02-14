#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatData.h"
#include "Item/WorldItem.h"
#include "ArrowWorldItem.generated.h"

class AArrowProjectile;

/**
 * World item specialization for arrows.
 * Reuses all pickup logic from AWorldItem and provides a projectile spawn helper.
 */
UCLASS()
class RPGSYSTEM_API AArrowWorldItem : public AWorldItem
{
    GENERATED_BODY()

public:
    /** Spawns an arrow projectile and initializes its damage/ballistic parameters. */
    UFUNCTION(BlueprintCallable, Category = "Arrow")
    AArrowProjectile* SpawnArrowProjectile(
        const FTransform& SpawnTransform,
        AActor* SourceActor,
        float BaseDamageOverride = -1.0f) const;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arrow")
    TSubclassOf<AArrowProjectile> ArrowProjectileClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arrow")
    float ArrowBaseDamage = 35.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arrow")
    float ArrowInitialSpeed = 5200.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Arrow")
    float ArrowGravityScale = 1.0f;
};
