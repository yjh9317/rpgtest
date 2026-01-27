// RPGGameplayCue.cpp

#include "Status/Cues/RPGGameplayCue.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

// ========== BASE CLASS ==========

URPGGameplayCue::URPGGameplayCue()
{
    ExecutionType = ECueExecutionType::Burst;
}

bool URPGGameplayCue::Execute(const FCueContext& Context)
{
    switch (ExecutionType)
    {
        case ECueExecutionType::Burst:
            OnExecuteBurst(Context);
            return true;

        case ECueExecutionType::Looping:
            OnExecuteLooping(Context);
            return true;

        case ECueExecutionType::ActorSpawn:
        {
            AActor* SpawnedActor = OnExecuteSpawn(Context);
            return SpawnedActor != nullptr;
        }

        default:
            return false;
    }
}

void URPGGameplayCue::Stop(AActor* TargetActor)
{
    if (ExecutionType == ECueExecutionType::Looping)
    {
        OnStop(TargetActor);
    }
}

bool URPGGameplayCue::IsPlaying(AActor* TargetActor) const
{
    // Default implementation - subclasses override for looping cues
    return false;
}

UWorld* URPGGameplayCue::GetWorld() const
{
    // Try to get world from outer object
    if (UObject* Outer = GetOuter())
    {
        return Outer->GetWorld();
    }
    return nullptr;
}

void URPGGameplayCue::SpawnParticleEffect(UNiagaraSystem* ParticleSystem, const FCueContext& Context, bool bAttach)
{
    if (!ParticleSystem || !GetWorld())
    {
        return;
    }

    if (bAttach && Context.TargetActor.IsValid())
    {
        UNiagaraFunctionLibrary::SpawnSystemAttached(
            ParticleSystem,
            Context.TargetActor->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            true
        );
    }
    else
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            ParticleSystem,
            Context.Location,
            Context.Rotation
        );
    }
}

void URPGGameplayCue::PlaySound(USoundBase* Sound, const FCueContext& Context, bool bAttach)
{
    if (!Sound || !GetWorld())
    {
        return;
    }

    if (bAttach && Context.TargetActor.IsValid())
    {
        UGameplayStatics::SpawnSoundAttached(
            Sound,
            Context.TargetActor->GetRootComponent()
        );
    }
    else
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            Sound,
            Context.Location
        );
    }
}

void URPGGameplayCue::ApplyCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, const FCueContext& Context, float InnerRadius, float OuterRadius)
{
    if (!ShakeClass || !GetWorld())
    {
        return;
    }

    UGameplayStatics::PlayWorldCameraShake(
        GetWorld(),
        ShakeClass,
        Context.Location,
        InnerRadius,
        OuterRadius
    );
}

// ========== NIAGARA CUE ==========

void URPGCue_Niagara::OnExecuteBurst_Implementation(const FCueContext& Context)
{
    if (ParticleSystem)
    {
        UNiagaraComponent* SpawnedComponent = nullptr;

        if (bAttachToTarget && Context.TargetActor.IsValid())
        {
            SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
                ParticleSystem,
                Context.TargetActor->GetRootComponent(),
                AttachSocketName,
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                EAttachLocation::KeepRelativeOffset,
                true
            );
        }
        else
        {
            SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                ParticleSystem,
                Context.Location,
                Context.Rotation
            );
        }

        // Scale by magnitude if enabled
        if (SpawnedComponent && bScaleByMagnitude)
        {
            float Scale = FMath::Clamp(Context.Magnitude, 0.1f, 10.0f);
            SpawnedComponent->SetWorldScale3D(FVector(Scale));
        }
    }

    // Play sound
    if (Sound)
    {
        PlaySound(Sound, Context, bAttachToTarget);
    }

    // Apply camera shake
    if (CameraShake)
    {
        ApplyCameraShake(CameraShake, Context, ShakeInnerRadius, ShakeOuterRadius);
    }
}

void URPGCue_Niagara::OnExecuteLooping_Implementation(const FCueContext& Context)
{
    if (!ParticleSystem || !Context.TargetActor.IsValid())
    {
        return;
    }

    // Spawn looping particle system
    UNiagaraComponent* LoopingComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
        ParticleSystem,
        Context.TargetActor->GetRootComponent(),
        AttachSocketName,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::KeepRelativeOffset,
        false  // Don't auto-destroy for looping
    );

    if (LoopingComponent)
    {
        // Store reference to component for later cleanup
        ActiveLoopingComponents.Add(Context.TargetActor, LoopingComponent);

        // Scale if needed
        if (bScaleByMagnitude)
        {
            float Scale = FMath::Clamp(Context.Magnitude, 0.1f, 10.0f);
            LoopingComponent->SetWorldScale3D(FVector(Scale));
        }
    }
}

void URPGCue_Niagara::OnStop_Implementation(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }

    // Find and destroy the looping component
    if (TWeakObjectPtr<UNiagaraComponent>* ComponentPtr = ActiveLoopingComponents.Find(TargetActor))
    {
        if (UNiagaraComponent* Component = ComponentPtr->Get())
        {
            Component->Deactivate();
            Component->DestroyComponent();
        }
        ActiveLoopingComponents.Remove(TargetActor);
    }
}

// ========== SOUND CUE ==========

void URPGCue_Sound::OnExecuteBurst_Implementation(const FCueContext& Context)
{
    if (!Sound || !GetWorld())
    {
        return;
    }

    if (bAttachToTarget && Context.TargetActor.IsValid())
    {
        UGameplayStatics::SpawnSoundAttached(
            Sound,
            Context.TargetActor->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset,
            false,
            VolumeMultiplier,
            PitchMultiplier
        );
    }
    else
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            Sound,
            Context.Location,
            VolumeMultiplier,
            PitchMultiplier
        );
    }
}

// ========== CAMERA SHAKE CUE ==========

void URPGCue_CameraShake::OnExecuteBurst_Implementation(const FCueContext& Context)
{
    if (!CameraShakeClass || !GetWorld())
    {
        return;
    }

    float FinalInnerRadius = InnerRadius;
    float FinalOuterRadius = OuterRadius;

    // Scale shake by magnitude if enabled
    if (bScaleByMagnitude)
    {
        float Scale = FMath::Clamp(Context.Magnitude, 0.1f, 10.0f);
        FinalInnerRadius *= Scale;
        FinalOuterRadius *= Scale;
    }

    UGameplayStatics::PlayWorldCameraShake(
        GetWorld(),
        CameraShakeClass,
        Context.Location,
        FinalInnerRadius,
        FinalOuterRadius
    );
}
