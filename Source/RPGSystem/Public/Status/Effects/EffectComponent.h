// EffectComponent.h
// Component that MANAGES all effects on an actor (like a container/manager)
// This is what you add to your character to make them able to receive effects

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ActiveEffectHandle.h"
#include "EffectComponent.generated.h"

class URPGEffect;
class UCueManagerSubsystem;
class UStatsComponent;

USTRUCT()
struct FAppliedEffectDelta
{
    GENERATED_BODY()

    UPROPERTY()
    FGameplayTag StatTag;

    UPROPERTY()
    float AppliedDelta = 0.0f;
};


/**
 * Delegates for broadcasting effect events
 * These allow other systems (UI, animations, etc.) to react to effects
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEffectApplied, URPGEffect*, Effect, const FEffectContext&, Context, const FActiveEffectHandle&, Handle);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEffectRemoved, URPGEffect*, Effect, const FEffectContext&, Context, float, RemainingDuration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectTick, const FActiveEffectHandle&, Handle, float, Magnitude);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectStackChanged, const FActiveEffectHandle&, Handle, int32, NewStacks);

/**
 * Component that manages all gameplay effects applied to an actor
 * 
 * RESPONSIBILITIES:
 * - Apply new effects (with stacking rules)
 * - Update effect timers each frame
 * - Execute periodic effect ticks
 * - Remove expired effects
 * - Broadcast effect events to listeners
 * - Manage granted gameplay tags
 * 
 * DESIGN PATTERN: This is a Manager/Container pattern.
 * It doesn't define WHAT effects are, it manages ACTIVE effects.
 * Think of it like a playlist manager: it doesn't create songs, it manages what's playing.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class RPGSYSTEM_API UEffectComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UEffectComponent();

protected:
    /** All effects currently active on this actor */
    UPROPERTY(BlueprintReadOnly, Category = "Effects")
    TArray<FActiveEffectHandle> ActiveEffects;

    /** Tags granted by currently active effects (for quick queries) */
    UPROPERTY(BlueprintReadOnly, Category = "Effects")
    FGameplayTagContainer GrantedTags;

    /** Cached reference to Cue Manager for playing effects */
    UPROPERTY()
    TObjectPtr<UCueManagerSubsystem> CueManager = nullptr;

    /** Cached reference to stat component for effect stat modification */
    UPROPERTY()
    TObjectPtr<UStatsComponent> StatsComponent = nullptr;

    /** Runtime mapping of effect instance -> applied stat modifier tags (runtime-only, not reflected). */
    TMap<FGuid, TArray<FAppliedEffectDelta>> AppliedEffectDeltas;


public:
    // ========== LIFECYCLE ==========
    
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== EFFECT APPLICATION ==========
    
    /**
     * Apply an effect to this actor
     * 
     * @param EffectToApply The effect definition to apply
     * @param Context Who/what/where information about the application
     * @return Handle to the applied effect (or existing one if stacking occurred)
     */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    FActiveEffectHandle ApplyEffect(URPGEffect* EffectToApply, const FEffectContext& Context);

    /**
     * Apply an effect from a source actor to this actor
     * Convenience function that auto-fills the context
     */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    FActiveEffectHandle ApplyEffectFromSource(URPGEffect* EffectToApply, AActor* SourceActor);

    // ========== EFFECT REMOVAL ==========
    
    /**
     * Remove a specific effect instance by its handle
     */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    bool RemoveEffect(const FActiveEffectHandle& Handle);

    /**
     * Remove all effects matching a specific tag
     * Example: RemoveEffectsByTag("Effect.Debuff") removes all debuffs
     */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    int32 RemoveEffectsByTag(const FGameplayTag& EffectTag);

    /**
     * Remove all effects granted by a specific source actor
     * Example: When a buff shrine expires, remove all its buffs
     */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    int32 RemoveEffectsFromSource(AActor* SourceActor);

    /**
     * Remove all effects (typically used on death or cleanup)
     */
    UFUNCTION(BlueprintCallable, Category = "Effects")
    void RemoveAllEffects();

    // ========== QUERIES ==========
    
    /**
     * Check if this actor has an effect matching a specific tag
     */
    UFUNCTION(BlueprintPure, Category = "Effects")
    bool HasEffectWithTag(const FGameplayTag& EffectTag) const;

    /**
     * Get all active effects matching a tag
     */
    UFUNCTION(BlueprintPure, Category = "Effects")
    TArray<FActiveEffectHandle> GetEffectsByTag(const FGameplayTag& EffectTag) const;

    /**
     * Get the total magnitude of all effects matching a tag
     * Example: If you have 2 "Damage Buff" effects (10 dmg each), returns 20
     */
    UFUNCTION(BlueprintPure, Category = "Effects")
    float GetTotalMagnitudeForTag(const FGameplayTag& EffectTag) const;

    /**
     * Get the number of active effects
     */
    UFUNCTION(BlueprintPure, Category = "Effects")
    int32 GetActiveEffectCount() const { return ActiveEffects.Num(); }

    /**
     * Get all currently active effects (read-only)
     */
    UFUNCTION(BlueprintPure, Category = "Effects")
    TArray<FActiveEffectHandle> GetAllActiveEffects() const { return ActiveEffects; }

    // ========== TAGS ==========
    
    /**
     * Check if this actor has a tag granted by an active effect
     * Example: HasGrantedTag("Status.Poisoned") returns true if any poison effect is active
     */
    UFUNCTION(BlueprintPure, Category = "Effects|Tags")
    bool HasGrantedTag(const FGameplayTag& Tag) const;

    /**
     * Check if this actor has any of the specified tags
     */
    UFUNCTION(BlueprintPure, Category = "Effects|Tags")
    bool HasAnyGrantedTags(const FGameplayTagContainer& Tags) const;

    /**
     * Check if this actor has all of the specified tags
     */
    UFUNCTION(BlueprintPure, Category = "Effects|Tags")
    bool HasAllGrantedTags(const FGameplayTagContainer& Tags) const;

    /**
     * Get all tags currently granted by effects
     */
    UFUNCTION(BlueprintPure, Category = "Effects|Tags")
    FGameplayTagContainer GetGrantedTags() const { return GrantedTags; }

    // ========== EVENTS / DELEGATES ==========
    
    /** Broadcast when a new effect is applied */
    UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
    FOnEffectApplied OnEffectApplied;

    /** Broadcast when an effect is removed/expires */
    UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
    FOnEffectRemoved OnEffectRemoved;

    /** Broadcast when a periodic effect ticks */
    UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
    FOnEffectTick OnEffectTick;

    /** Broadcast when an effect's stack count changes */
    UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
    FOnEffectStackChanged OnEffectStackChanged;

protected:
    // ========== INTERNAL LOGIC ==========
    
    /**
     * Check if an effect can be applied based on tag requirements
     * Returns true if all conditions are met, false otherwise
     */
    bool CanApplyEffect(URPGEffect* Effect) const;

    /**
     * Handle applying an effect that matches an existing one (stacking logic)
     * Returns pointer to the existing effect that was modified (or nullptr if stack rejected)
     */
    FActiveEffectHandle* HandleStacking(URPGEffect* IncomingEffect, const FEffectContext& Context);

    /**
     * Apply an instant effect (damage/heal/etc.) and immediately remove it
     */
    void ApplyInstantEffect(URPGEffect* Effect, const FEffectContext& Context);

    /**
     * Execute a periodic tick for an effect
     */
    void ExecuteEffectTick(FActiveEffectHandle& EffectHandle);

    /**
     * Update all active effects (timers, ticks, expiration)
     */
    void UpdateActiveEffects(float DeltaTime);

    /**
     * Internal cleanup when an effect is removed
     */
    void OnEffectRemovedInternal(const FActiveEffectHandle& RemovedEffect);

    /**
     * Update the granted tags cache based on active effects
     */
    void RefreshGrantedTags();

    /**
     * Check if any active effects should be removed due to tag conflicts
     */
    void CheckForRemovalTags();

    /**
     * Play the appropriate gameplay cue for an effect event
     */
    void PlayEffectCue(const FGameplayTag& CueTag, const FEffectContext& Context);

public:
    // ========== DEBUGGING ==========
    
    /**
     * Debug function to print all active effects to log
     */
    UFUNCTION(BlueprintCallable, Category = "Effects|Debug")
    void DebugPrintActiveEffects() const;

    /**
     * Get a debug string of all active effects (for on-screen display)
     */
    UFUNCTION(BlueprintPure, Category = "Effects|Debug")
    FString GetActiveEffectsDebugString() const;
};
