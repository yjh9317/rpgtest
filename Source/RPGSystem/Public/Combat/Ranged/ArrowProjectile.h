#pragma once

#include "CoreMinimal.h"
#include "Combat/CombatData.h"
#include "GameFramework/Actor.h"
#include "ArrowProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UPrimitiveComponent;
class UStaticMeshComponent;
class UParticleSystem;

UCLASS()
class RPGSYSTEM_API AArrowProjectile : public AActor
{
    GENERATED_BODY()

public:
    AArrowProjectile();

    /** Launch data can be provided by bow/action code after spawn. */
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void InitializeArrowProjectile(const FDamageInfo& InDamageInfo, float InitialSpeed, float GravityScale);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USphereComponent> CollisionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

    /** Optional one-shot trail/tracer at spawn time. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "VFX")
    TObjectPtr<UParticleSystem> TracerTemplate;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    float LifeSeconds = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Projectile")
    bool bDestroyOnAnyHit = true;

    UPROPERTY(BlueprintReadOnly, Category = "Projectile")
    FDamageInfo DamageInfo;

    UFUNCTION()
    void OnArrowHit(
        UPrimitiveComponent* HitComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        FVector NormalImpulse,
        const FHitResult& Hit);
};
