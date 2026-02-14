#include "Combat/Ranged/ArrowProjectile.h"

#include "Combat/Combatable.h"
#include "Combat/Components/CombatComponentBase.h"
#include "Components/SphereComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"

AArrowProjectile::AArrowProjectile()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
    CollisionComponent->SetSphereRadius(8.0f);
    CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
    RootComponent = CollisionComponent;

    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileMovement->InitialSpeed = 4000.0f;
    ProjectileMovement->MaxSpeed = 4000.0f;
    ProjectileMovement->ProjectileGravityScale = 1.0f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
}

void AArrowProjectile::BeginPlay()
{
    Super::BeginPlay();

    CollisionComponent->OnComponentHit.AddDynamic(this, &AArrowProjectile::OnArrowHit);
    SetLifeSpan(LifeSeconds);

    if (TracerTemplate)
    {
        UGameplayStatics::SpawnEmitterAttached(TracerTemplate, MeshComponent);
    }
}

void AArrowProjectile::InitializeArrowProjectile(const FDamageInfo& InDamageInfo, float InitialSpeed, float GravityScale)
{
    DamageInfo = InDamageInfo;

    if (ProjectileMovement)
    {
        ProjectileMovement->InitialSpeed = InitialSpeed;
        ProjectileMovement->MaxSpeed = InitialSpeed;
        ProjectileMovement->ProjectileGravityScale = GravityScale;
    }
}

void AArrowProjectile::OnArrowHit(
    UPrimitiveComponent* HitComp,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (!OtherActor || OtherActor == this || OtherActor == DamageInfo.SourceActor.Get())
    {
        return;
    }

    float AppliedDamage = 0.0f;
    if (ICombatable* Combatable = Cast<ICombatable>(OtherActor))
    {
        AppliedDamage = Combatable->ReceiveDamage(DamageInfo);
    }
    else if (UCombatComponentBase* CombatComp = OtherActor->FindComponentByClass<UCombatComponentBase>())
    {
        AppliedDamage = CombatComp->ReceiveDamage(DamageInfo);
    }
    else
    {
        AppliedDamage = OtherActor->TakeDamage(DamageInfo.BaseDamage, FDamageEvent(), nullptr, DamageInfo.SourceActor.Get());
    }

    if (AppliedDamage > 0.0f)
    {
        UE_LOG(LogTemp, Verbose, TEXT("Arrow hit %s for %.1f"), *OtherActor->GetName(), AppliedDamage);
    }

    if (bDestroyOnAnyHit)
    {
        Destroy();
    }
}