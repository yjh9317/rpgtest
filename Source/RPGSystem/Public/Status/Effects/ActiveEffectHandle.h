// ActiveEffectHandle.h
// Represents a SPECIFIC INSTANCE of an effect currently applied to an actor
// Think of this as: URPGEffect = recipe, FActiveEffectHandle = the actual cake baking in the oven

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "RPGEffect.h"
#include "ActiveEffectHandle.generated.h"

class URPGEffect;
class AActor;

/**
 * Context information about an effect application
 * Contains all the "who, what, where, when" details
 */
USTRUCT(BlueprintType)
struct FEffectContext
{
    GENERATED_BODY()

    /** Who is applying this effect? (e.g., the mage who cast poison) */
    UPROPERTY(BlueprintReadWrite, Category = "Effect Context")
    TWeakObjectPtr<AActor> SourceActor = nullptr;

    /** Who is receiving this effect? (e.g., the enemy being poisoned) */
    UPROPERTY(BlueprintReadWrite, Category = "Effect Context")
    TWeakObjectPtr<AActor> TargetActor = nullptr;

    /** The location where the effect was applied (useful for area effects) */
    UPROPERTY(BlueprintReadWrite, Category = "Effect Context")
    FVector SourceLocation = FVector::ZeroVector;

    /** When was this effect applied? (World time) */
    UPROPERTY(BlueprintReadWrite, Category = "Effect Context")
    float ApplicationTime = 0.0f;

    /** Optional: Source attribute value used for scaling (cached at application time) */
    UPROPERTY(BlueprintReadWrite, Category = "Effect Context")
    float CachedSourceAttributeValue = 0.0f;

    /** Optional: Additional metadata (e.g., critical hit flag, bonus multiplier) */
    UPROPERTY(BlueprintReadWrite, Category = "Effect Context")
    TMap<FGameplayTag, float> CustomData;

    FEffectContext()
        : ApplicationTime(0.0f)
        , CachedSourceAttributeValue(0.0f)
    {}
};

/**
 * An ACTIVE instance of an effect currently applied to a target
 * 
 * This is the "running" version of an effect. While URPGEffect is the definition,
 * FActiveEffectHandle tracks the real-time state:
 * - How much time is left?
 * - How many stacks are active?
 * - When does it tick next?
 * - What's the current magnitude?
 */
USTRUCT(BlueprintType)
struct FActiveEffectHandle
{
    GENERATED_BODY()

    /** The effect definition (the "recipe") */
    UPROPERTY(BlueprintReadOnly, Category = "Active Effect")
    TObjectPtr<URPGEffect> EffectDefinition = nullptr;

    /** Context information about this application */
    UPROPERTY(BlueprintReadOnly, Category = "Active Effect")
    FEffectContext Context;

    /** How much duration remains before this effect expires (in seconds) */
    UPROPERTY(BlueprintReadOnly, Category = "Active Effect")
    float RemainingDuration = 0.0f;

    /** For periodic effects: time until next tick */
    UPROPERTY(BlueprintReadOnly, Category = "Active Effect")
    float TimeUntilNextTick = 0.0f;

    /** Current stack count (relevant for StackMagnitude policy) */
    UPROPERTY(BlueprintReadOnly, Category = "Active Effect")
    int32 CurrentStacks = 1;

    /** Cached magnitude (updated when stacks change) */
    UPROPERTY(BlueprintReadOnly, Category = "Active Effect")
    float CachedMagnitude = 0.0f;

    /** How many times has this effect ticked? (for periodic effects) */
    UPROPERTY(BlueprintReadOnly, Category = "Active Effect")
    int32 TickCount = 0;

    /** Unique identifier for this specific effect instance */
    UPROPERTY(BlueprintReadOnly, Category = "Active Effect")
    FGuid InstanceID;

    FActiveEffectHandle()
        : RemainingDuration(0.0f)
        , TimeUntilNextTick(0.0f)
        , CurrentStacks(1)
        , CachedMagnitude(0.0f)
        , TickCount(0)
    {
        InstanceID = FGuid::NewGuid();
    }

    /** Check if this effect has expired */
    bool IsExpired() const
    {
        // Instant effects expire immediately after application
        if (EffectDefinition && EffectDefinition->IsInstant())
        {
            return true;
        }
        return RemainingDuration <= 0.0f;
    }

    /** Check if this effect should tick now (for periodic effects) */
    bool ShouldTick() const
    {
        if (!EffectDefinition || !EffectDefinition->IsPeriodic())
        {
            return false;
        }
        return TimeUntilNextTick <= 0.0f && RemainingDuration > 0.0f;
    }

    /** Get the total magnitude considering stacks */
    float GetTotalMagnitude() const
    {
        return CachedMagnitude;
    }

    /** Recalculate magnitude (called when stacks change) */
    void RecalculateMagnitude()
    {
        if (EffectDefinition)
        {
            CachedMagnitude = EffectDefinition->CalculateMagnitude(
                Context.CachedSourceAttributeValue,
                CurrentStacks
            );
        }
    }

    /** Update timers (called every frame by EffectComponent) */
    void UpdateTimers(float DeltaTime)
    {
        RemainingDuration -= DeltaTime;

        if (EffectDefinition && EffectDefinition->IsPeriodic())
        {
            TimeUntilNextTick -= DeltaTime;
            
            // Reset tick timer when it reaches zero
            if (TimeUntilNextTick <= 0.0f && RemainingDuration > 0.0f)
            {
                TimeUntilNextTick = EffectDefinition->Period;
            }
        }
    }

    /** Equality operator for finding effects */
    bool operator==(const FActiveEffectHandle& Other) const
    {
        return InstanceID == Other.InstanceID;
    }

    /** Check if this handle matches a specific effect tag */
    bool MatchesTag(const FGameplayTag& Tag) const
    {
        return EffectDefinition && EffectDefinition->EffectTag.MatchesTag(Tag);
    }
};
