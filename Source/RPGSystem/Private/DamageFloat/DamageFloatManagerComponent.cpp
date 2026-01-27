// Fill out your copyright notice in the Description page of Project Settings.


#include "DamageFloat/DamageFloatManagerComponent.h"

UDamageFloatManagerComponent::UDamageFloatManagerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Every frame
}

void UDamageFloatManagerComponent::BeginPlay()
{
    Super::BeginPlay();
    PreallocatePool();
}

void UDamageFloatManagerComponent::PreallocatePool()
{
    ActiveFloats.Reserve(MaxActiveDamageFloats);
    UE_LOG(LogTemp, Log, TEXT("[DamageFloatManager] Pre-allocated pool for %d floats"), MaxActiveDamageFloats);
}

void UDamageFloatManagerComponent::SpawnDamageFloat(float DamageAmount, FVector WorldPosition, bool bIsCritical)
{
    // Validation
    if (!ensure(DamageAmount >= 0.0f))
    {
        UE_LOG(LogTemp, Warning, TEXT("[DamageFloatManager] Invalid damage: %.2f"), DamageAmount);
        return;
    }

    // FIFO removal if max capacity reached
    if (ActiveFloats.Num() >= MaxActiveDamageFloats)
    {
        ActiveFloats.RemoveAt(0);
    }

    // Create new float with FIXED world position
    FDamageFloatData NewFloat(DamageAmount, WorldPosition, bIsCritical);
    NewFloat.RemainingLifetime = DamageFloatLifetime;
    NewFloat.SpawnTime = CurrentWorldTime;

    ActiveFloats.Add(NewFloat);

#if !UE_BUILD_SHIPPING
    // Debug visualization
    if (UWorld* World = GetWorld())
    {
        FColor DebugColor = bIsCritical ? FColor::Red : FColor::Yellow;
        DrawDebugSphere(World, WorldPosition, 15.0f, 8, DebugColor, false, 0.5f, 0, 2.0f);
        
        // Draw position marker
        DrawDebugString(World, WorldPosition + FVector(0, 0, 50), 
                        FString::Printf(TEXT("%.0f"), DamageAmount), 
                        nullptr, DebugColor, 0.5f, false, 1.2f);
    }
#endif

    UE_LOG(LogTemp, VeryVerbose, TEXT("[DamageFloatManager] Spawned float: Damage=%.0f, Pos=%s, Critical=%d, Total=%d"),
           DamageAmount, *WorldPosition.ToString(), bIsCritical, ActiveFloats.Num());
}

void UDamageFloatManagerComponent::SpawnDamageFloatAtActor(AActor* TargetActor, float DamageAmount, float HeightOffset, bool bIsCritical)
{
    if (!IsValid(TargetActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[DamageFloatManager] SpawnDamageFloatAtActor called with invalid actor"));
        return;
    }

    // Get actor's CURRENT position (공격받은 순간의 위치!)
    FVector SpawnPosition = TargetActor->GetActorLocation() + FVector(0, 0, HeightOffset);
    
    SpawnDamageFloat(DamageAmount, SpawnPosition, bIsCritical);
}

void UDamageFloatManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                  FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ActiveFloats.Num() == 0)
    {
        return;
    }

    CurrentWorldTime += DeltaTime;

    // Update all floats
    TArray<int32> IndicesToRemove;
    IndicesToRemove.Reserve(ActiveFloats.Num() / 4);

    for (int32 i = 0; i < ActiveFloats.Num(); ++i)
    {
        FDamageFloatData& FloatData = ActiveFloats[i];

        // Update lifetime
        FloatData.RemainingLifetime -= DeltaTime;

        if (FloatData.RemainingLifetime <= 0.0f)
        {
            IndicesToRemove.Add(i);
            continue;
        }

        // Animate: Float upwards (독립적으로!)
        FloatData.VerticalOffset += VerticalFloatSpeed * DeltaTime;

        // Fade out
        float LifetimePercent = FloatData.RemainingLifetime / DamageFloatLifetime;
        FloatData.TextColor.A = FMath::Clamp(LifetimePercent, 0.0f, 1.0f);
    }

    // Remove expired floats (reverse iteration)
    for (int32 i = IndicesToRemove.Num() - 1; i >= 0; --i)
    {
        ActiveFloats.RemoveAt(IndicesToRemove[i]);
    }

    // Optional: Cull off-screen floats
    if (bCullOffScreenFloats)
    {
        CullDistantFloats();
    }

    // Shrink if over-allocated
    if (ActiveFloats.Max() > MaxActiveDamageFloats * 2 && ActiveFloats.Num() < MaxActiveDamageFloats / 2)
    {
        ActiveFloats.Shrink();
    }
}

void UDamageFloatManagerComponent::CullDistantFloats()
{
    UWorld* World = GetWorld();
    if (!World) return;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!IsValid(PC)) return;

    APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
    if (!IsValid(CameraManager)) return;

    FVector CameraLocation = CameraManager->GetCameraLocation();

    // Remove floats too far from camera
    ActiveFloats.RemoveAll([&](const FDamageFloatData& FloatData)
    {
        float Distance = FVector::Dist(FloatData.GetCurrentWorldPosition(), CameraLocation);
        return Distance > MaxDistanceFromCamera;
    });
}