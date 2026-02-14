#include "Item/ArrowWorldItem.h"

#include "Combat/Ranged/ArrowProjectile.h"
#include "Engine/World.h"

AArrowProjectile* AArrowWorldItem::SpawnArrowProjectile(
    const FTransform& SpawnTransform,
    AActor* SourceActor,
    float BaseDamageOverride) const
{
    if (!ArrowProjectileClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("AArrowWorldItem::SpawnArrowProjectile - ArrowProjectileClass is null"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = SourceActor ? SourceActor : GetOwner();
    SpawnParams.Instigator = Cast<APawn>(SourceActor);

    AArrowProjectile* Projectile = World->SpawnActorDeferred<AArrowProjectile>(
        ArrowProjectileClass,
        SpawnTransform,
        SpawnParams.Owner,
        SpawnParams.Instigator,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

    if (!Projectile)
    {
        return nullptr;
    }

    FDamageInfo DamageInfo;
    DamageInfo.SourceActor = SourceActor ? SourceActor : GetOwner();
    DamageInfo.BaseDamage = BaseDamageOverride >= 0.0f ? BaseDamageOverride : ArrowBaseDamage;

    Projectile->InitializeArrowProjectile(DamageInfo, ArrowInitialSpeed, ArrowGravityScale);
    Projectile->FinishSpawning(SpawnTransform);

    return Projectile;
}
