// EffectComponent.cpp

#include "Status/Effects/EffectComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Status/StatsComponent.h"
#include "Status/Cues/CueManagerSubsystem.h"
#include "Status/Effects/RPGEffect.h"

UEffectComponent::UEffectComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UEffectComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache the Cue Manager for later use
    if (UWorld* World = GetWorld())
    {
        if (UGameInstance* GameInstance = World->GetGameInstance())
        {
            CueManager = GameInstance->GetSubsystem<UCueManagerSubsystem>();
        }
    }
    StatsComponent = GetOwner() ? GetOwner()->FindComponentByClass<UStatsComponent>() : nullptr;
}

void UEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateActiveEffects(DeltaTime);
}

void UEffectComponent::ApplyEffectStatModifiers(const FActiveEffectHandle& EffectHandle, bool bPersistentModifier)
{
    if (!StatsComponent || !EffectHandle.EffectDefinition)
    {
        return;
    }

    const float TotalMagnitude = EffectHandle.GetTotalMagnitude();

    for (const FEffectStatModifier& Modifier : EffectHandle.EffectDefinition->StatModifiers)
    {
        if (!Modifier.StatTag.IsValid())
        {
            continue;
        }

        const float FinalValue = Modifier.Value * TotalMagnitude;

        if (Modifier.ModifierType != EModifierSourceType::Flat)
        {
            UE_LOG(LogTemp, Verbose, TEXT("UEffectComponent::ApplyEffectStatModifiers - Non-flat modifier type is skipped for runtime delta application."));
            continue;
        }

        StatsComponent->ModifyStatValue(Modifier.StatTag, FinalValue);

        if (bPersistentModifier)
        {
            FAppliedEffectDelta Delta;
            Delta.StatTag = Modifier.StatTag;
            Delta.AppliedDelta = FinalValue;
            AppliedEffectDeltas.FindOrAdd(EffectHandle.InstanceID).Add(Delta);
        }
    }
}

void UEffectComponent::RemoveEffectStatModifiers(const FGuid& InstanceID)
{
    if (!StatsComponent)
    {
        return;
    }

    TArray<FAppliedEffectDelta>* AppliedDeltas = AppliedEffectDeltas.Find(InstanceID);
    if (!AppliedDeltas)
    {
        return;
    }

    for (const FAppliedEffectDelta& Delta : *AppliedDeltas)
    {
        if (Delta.StatTag.IsValid() && !FMath::IsNearlyZero(Delta.AppliedDelta))
        {
            StatsComponent->ModifyStatValue(Delta.StatTag, -Delta.AppliedDelta);
        }
    }

    AppliedEffectDeltas.Remove(InstanceID);
}

void UEffectComponent::ReapplyPersistentEffectStatModifiers(const FActiveEffectHandle& EffectHandle)
{
    RemoveEffectStatModifiers(EffectHandle.InstanceID);
    ApplyEffectStatModifiers(EffectHandle, true);
}

// ========== EFFECT APPLICATION ==========

FActiveEffectHandle UEffectComponent::ApplyEffect(URPGEffect* EffectToApply, const FEffectContext& Context)
{
    // Validation
    if (!EffectToApply)
    {
        UE_LOG(LogTemp, Warning, TEXT("UEffectComponent::ApplyEffect - Null effect provided"));
        return FActiveEffectHandle();
    }

    if (!CanApplyEffect(EffectToApply))
    {
        UE_LOG(LogTemp, Log, TEXT("UEffectComponent::ApplyEffect - Effect '%s' blocked by tags"), 
               *EffectToApply->EffectName.ToString());
        return FActiveEffectHandle();
    }

    // Handle instant effects separately
    if (EffectToApply->IsInstant())
    {
        ApplyInstantEffect(EffectToApply, Context);
        return FActiveEffectHandle(); // Instant effects don't return a handle (they expire immediately)
    }

    // Check if we already have this effect (for stacking)
    if (EffectToApply->CanStack())
    {
        FActiveEffectHandle* ExistingEffect = HandleStacking(EffectToApply, Context);
        if (ExistingEffect)
        {
            ReapplyPersistentEffectStatModifiers(*ExistingEffect);
            return *ExistingEffect; // Return the modified existing effect
        }
        // If HandleStacking returns nullptr and StackPolicy is None, we reject the effect
        if (EffectToApply->StackPolicy == EEffectStackPolicy::None)
        {
            return FActiveEffectHandle();
        }
    }

    // Create a new effect instance
    FActiveEffectHandle NewEffect;
    NewEffect.EffectDefinition = EffectToApply;
    NewEffect.Context = Context;
    NewEffect.RemainingDuration = EffectToApply->Duration;
    NewEffect.CurrentStacks = 1;
    
    // Set up periodic ticking if needed
    if (EffectToApply->IsPeriodic())
    {
        NewEffect.TimeUntilNextTick = EffectToApply->Period;
    }

    // Calculate magnitude
    NewEffect.RecalculateMagnitude();

    // Apply persistent stat modifiers immediately for first-time applications.
    ApplyEffectStatModifiers(NewEffect, true);
    
    // Add to active effects
    ActiveEffects.Add(NewEffect);

    // Update granted tags
    RefreshGrantedTags();

    // Broadcast event
    OnEffectApplied.Broadcast(EffectToApply, Context, NewEffect);

    // Play application cue
    if (EffectToApply->OnAppliedCue.IsValid())
    {
        PlayEffectCue(EffectToApply->OnAppliedCue, Context);
    }

    UE_LOG(LogTemp, Log, TEXT("UEffectComponent::ApplyEffect - Applied '%s' (Duration: %.1f, Magnitude: %.1f)"),
           *EffectToApply->EffectName.ToString(), NewEffect.RemainingDuration, NewEffect.GetTotalMagnitude());

    return NewEffect;
}

FActiveEffectHandle UEffectComponent::ApplyEffectFromSource(URPGEffect* EffectToApply, AActor* SourceActor)
{
    FEffectContext Context;
    Context.SourceActor = SourceActor;
    Context.TargetActor = GetOwner();
    Context.ApplicationTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    if (SourceActor)
    {
        Context.SourceLocation = SourceActor->GetActorLocation();
    }

    return ApplyEffect(EffectToApply, Context);
}

// ========== EFFECT REMOVAL ==========

bool UEffectComponent::RemoveEffect(const FActiveEffectHandle& Handle)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i] == Handle)
        {
            FActiveEffectHandle RemovedEffect = ActiveEffects[i];
            ActiveEffects.RemoveAt(i);
            OnEffectRemovedInternal(RemovedEffect);
            return true;
        }
    }
    return false;
}

int32 UEffectComponent::RemoveEffectsByTag(const FGameplayTag& EffectTag)
{
    int32 RemovedCount = 0;
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i].MatchesTag(EffectTag))
        {
            FActiveEffectHandle RemovedEffect = ActiveEffects[i];
            ActiveEffects.RemoveAt(i);
            OnEffectRemovedInternal(RemovedEffect);
            RemovedCount++;
        }
    }

    return RemovedCount;
}

int32 UEffectComponent::RemoveEffectsFromSource(AActor* SourceActor)
{
    if (!SourceActor)
    {
        return 0;
    }

    int32 RemovedCount = 0;
    
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i].Context.SourceActor.Get() == SourceActor)
        {
            FActiveEffectHandle RemovedEffect = ActiveEffects[i];
            ActiveEffects.RemoveAt(i);
            OnEffectRemovedInternal(RemovedEffect);
            RemovedCount++;
        }
    }

    return RemovedCount;
}

void UEffectComponent::RemoveAllEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        FActiveEffectHandle RemovedEffect = ActiveEffects[i];
        OnEffectRemovedInternal(RemovedEffect);
    }
    
    ActiveEffects.Empty();
    RefreshGrantedTags();
}

// ========== QUERIES ==========

bool UEffectComponent::HasEffectWithTag(const FGameplayTag& EffectTag) const
{
    for (const FActiveEffectHandle& Effect : ActiveEffects)
    {
        if (Effect.MatchesTag(EffectTag))
        {
            return true;
        }
    }
    return false;
}

TArray<FActiveEffectHandle> UEffectComponent::GetEffectsByTag(const FGameplayTag& EffectTag) const
{
    TArray<FActiveEffectHandle> MatchingEffects;
    
    for (const FActiveEffectHandle& Effect : ActiveEffects)
    {
        if (Effect.MatchesTag(EffectTag))
        {
            MatchingEffects.Add(Effect);
        }
    }
    
    return MatchingEffects;
}

float UEffectComponent::GetTotalMagnitudeForTag(const FGameplayTag& EffectTag) const
{
    float TotalMagnitude = 0.0f;
    
    for (const FActiveEffectHandle& Effect : ActiveEffects)
    {
        if (Effect.MatchesTag(EffectTag))
        {
            TotalMagnitude += Effect.GetTotalMagnitude();
        }
    }
    
    return TotalMagnitude;
}

// ========== TAGS ==========

bool UEffectComponent::HasGrantedTag(const FGameplayTag& Tag) const
{
    return GrantedTags.HasTag(Tag);
}

bool UEffectComponent::HasAnyGrantedTags(const FGameplayTagContainer& Tags) const
{
    return GrantedTags.HasAny(Tags);
}

bool UEffectComponent::HasAllGrantedTags(const FGameplayTagContainer& Tags) const
{
    return GrantedTags.HasAll(Tags);
}

// ========== INTERNAL LOGIC ==========

bool UEffectComponent::CanApplyEffect(URPGEffect* Effect) const
{
    if (!Effect)
    {
        return false;
    }

    // Check required tags
    if (Effect->RequiredTargetTags.Num() > 0)
    {
        if (!GrantedTags.HasAll(Effect->RequiredTargetTags))
        {
            return false;
        }
    }

    // Check blocked tags (immunity)
    if (Effect->BlockedByTags.Num() > 0)
    {
        if (GrantedTags.HasAny(Effect->BlockedByTags))
        {
            return false;
        }
    }

    return true;
}

FActiveEffectHandle* UEffectComponent::HandleStacking(URPGEffect* IncomingEffect, const FEffectContext& Context)
{
    // Find existing effect with matching tag
    for (FActiveEffectHandle& ExistingEffect : ActiveEffects)
    {
        if (ExistingEffect.EffectDefinition == IncomingEffect)
        {
            // Apply stacking policy
            switch (IncomingEffect->StackPolicy)
            {
                case EEffectStackPolicy::None:
                    // Don't stack, reject
                    return nullptr;

                case EEffectStackPolicy::Refresh:
                    // Reset duration
                    ExistingEffect.RemainingDuration = IncomingEffect->Duration;
                    if (IncomingEffect->IsPeriodic())
                    {
                        ExistingEffect.TimeUntilNextTick = IncomingEffect->Period;
                    }
                    return &ExistingEffect;

                case EEffectStackPolicy::StackDuration:
                    // Add durations
                    ExistingEffect.RemainingDuration += IncomingEffect->Duration;
                    return &ExistingEffect;

                case EEffectStackPolicy::StackMagnitude:
                    // Increase stacks (up to max)
                    if (ExistingEffect.CurrentStacks < IncomingEffect->MaxStacks)
                    {
                        ExistingEffect.CurrentStacks++;
                        ExistingEffect.RecalculateMagnitude();
                        OnEffectStackChanged.Broadcast(ExistingEffect, ExistingEffect.CurrentStacks);
                    }
                    // Also refresh duration
                    ExistingEffect.RemainingDuration = IncomingEffect->Duration;
                    return &ExistingEffect;
            }
        }
    }

    return nullptr;
}

void UEffectComponent::ApplyInstantEffect(URPGEffect* Effect, const FEffectContext& Context)
{
    // Create a temporary handle just for the event
    FActiveEffectHandle TempHandle;
    TempHandle.EffectDefinition = Effect;
    TempHandle.Context = Context;
    TempHandle.RecalculateMagnitude();

    ApplyEffectStatModifiers(TempHandle, false);
    
    // Broadcast event (listeners will handle the actual effect)
    OnEffectApplied.Broadcast(Effect, Context, TempHandle);

    // Play cue
    if (Effect->OnAppliedCue.IsValid())
    {
        PlayEffectCue(Effect->OnAppliedCue, Context);
    }

    // Immediately broadcast removal (for instant effects)
    OnEffectRemoved.Broadcast(Effect, Context, 0.0f);
}

void UEffectComponent::ExecuteEffectTick(FActiveEffectHandle& EffectHandle)
{
    if (!EffectHandle.EffectDefinition)
    {
        return;
    }

    EffectHandle.TickCount++;
    ApplyEffectStatModifiers(EffectHandle, false);
    
    // Broadcast tick event (listeners handle the actual damage/heal/etc.)
    OnEffectTick.Broadcast(EffectHandle, EffectHandle.GetTotalMagnitude());

    // Play tick cue
    if (EffectHandle.EffectDefinition->OnTickCue.IsValid())
    {
        PlayEffectCue(EffectHandle.EffectDefinition->OnTickCue, EffectHandle.Context);
    }
}

void UEffectComponent::UpdateActiveEffects(float DeltaTime)
{
    // Update timers and check for ticks/expiration
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        FActiveEffectHandle& Effect = ActiveEffects[i];
        
        // Update timers
        Effect.UpdateTimers(DeltaTime);

        // Check for periodic tick
        if (Effect.ShouldTick())
        {
            ExecuteEffectTick(Effect);
        }

        // Check for expiration
        if (Effect.IsExpired())
        {
            FActiveEffectHandle ExpiredEffect = Effect;
            ActiveEffects.RemoveAt(i);
            OnEffectRemovedInternal(ExpiredEffect);
        }
    }

    // Check if any effects should be removed due to tag conflicts
    CheckForRemovalTags();
}

void UEffectComponent::OnEffectRemovedInternal(const FActiveEffectHandle& RemovedEffect)
{
    RemoveEffectStatModifiers(RemovedEffect.InstanceID);
    // Broadcast removal event
    if (RemovedEffect.EffectDefinition)
    {
        OnEffectRemoved.Broadcast(
            RemovedEffect.EffectDefinition,
            RemovedEffect.Context,
            RemovedEffect.RemainingDuration
        );

        // Play removal cue
        if (RemovedEffect.EffectDefinition->OnRemovedCue.IsValid())
        {
            PlayEffectCue(RemovedEffect.EffectDefinition->OnRemovedCue, RemovedEffect.Context);
        }
    }

    // Update granted tags
    RefreshGrantedTags();
}

void UEffectComponent::RefreshGrantedTags()
{
    GrantedTags.Reset();

    for (const FActiveEffectHandle& Effect : ActiveEffects)
    {
        if (Effect.EffectDefinition)
        {
            GrantedTags.AppendTags(Effect.EffectDefinition->GrantedTags);
        }
    }
}

void UEffectComponent::CheckForRemovalTags()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        const FActiveEffectHandle& Effect = ActiveEffects[i];
        
        if (Effect.EffectDefinition && Effect.EffectDefinition->RemoveOnTags.Num() > 0)
        {
            // Check if we have any of the removal tags
            if (GrantedTags.HasAny(Effect.EffectDefinition->RemoveOnTags))
            {
                FActiveEffectHandle RemovedEffect = ActiveEffects[i];
                ActiveEffects.RemoveAt(i);
                OnEffectRemovedInternal(RemovedEffect);
            }
        }
    }
}

void UEffectComponent::PlayEffectCue(const FGameplayTag& CueTag, const FEffectContext& Context)
{
    if (CueManager && CueTag.IsValid())
    {
        // Create cue context from effect context
        FCueContext CueCtx;
        CueCtx.TargetActor = GetOwner();
        CueCtx.SourceActor = Context.SourceActor.Get();
        CueCtx.Location = Context.TargetActor.IsValid() ? Context.TargetActor->GetActorLocation() : Context.SourceLocation;

        CueManager->PlayCue(CueTag, CueCtx);
    }
}

// ========== DEBUGGING ==========

void UEffectComponent::DebugPrintActiveEffects() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Active Effects on %s ==="), *GetOwner()->GetName());
    
    for (const FActiveEffectHandle& Effect : ActiveEffects)
    {
        if (Effect.EffectDefinition)
        {
            UE_LOG(LogTemp, Log, TEXT("  [%s] Duration: %.1f, Stacks: %d, Magnitude: %.1f"),
                   *Effect.EffectDefinition->EffectName.ToString(),
                   Effect.RemainingDuration,
                   Effect.CurrentStacks,
                   Effect.GetTotalMagnitude());
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== Granted Tags ==="));
    for (const FGameplayTag& Tag : GrantedTags)
    {
        UE_LOG(LogTemp, Log, TEXT("  - %s"), *Tag.ToString());
    }
}

FString UEffectComponent::GetActiveEffectsDebugString() const
{
    FString DebugStr = FString::Printf(TEXT("Active Effects (%d):\n"), ActiveEffects.Num());
    
    for (const FActiveEffectHandle& Effect : ActiveEffects)
    {
        if (Effect.EffectDefinition)
        {
            DebugStr += FString::Printf(TEXT("  %s (%.1fs, x%d)\n"),
                                        *Effect.EffectDefinition->EffectName.ToString(),
                                        Effect.RemainingDuration,
                                        Effect.CurrentStacks);
        }
    }
    
    return DebugStr;
}
