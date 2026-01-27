// RPGEffect.h
// Base class for all gameplay effects (buffs, debuffs, DoT, instant damage, etc.)
// This is a UDataAsset so designers can create effect definitions in the editor

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Status/StatData.h"
#include "RPGEffect.generated.h"

/**
 * Defines how multiple instances of the same effect should interact
 * 
 * Example: If a player is poisoned twice, what happens?
 * - None: Second poison is rejected (can't stack)
 * - Refresh: Second poison resets the timer back to full duration
 * - StackDuration: Both poison durations are added together
 * - StackMagnitude: Damage per tick increases (2x poison = 2x damage)
 */
UENUM(BlueprintType)
enum class EEffectStackPolicy : uint8
{
    None            UMETA(DisplayName = "Don't Stack (Reject)"),
    Refresh         UMETA(DisplayName = "Refresh Duration"),
    StackDuration   UMETA(DisplayName = "Stack Duration (Add Times)"),
    StackMagnitude  UMETA(DisplayName = "Stack Magnitude (Add Power)")
};

/**
 * How the effect applies its magnitude over time
 * - Instant: Applied once immediately (e.g., heal potion)
 * - Duration: Applied once and lasts (e.g., movement speed buff)
 * - Periodic: Applied repeatedly over duration (e.g., poison damage over time)
 */
UENUM(BlueprintType)
enum class EEffectDurationType : uint8
{
    Instant     UMETA(DisplayName = "Instant (One-Time)"),
    Duration    UMETA(DisplayName = "Duration (Continuous)"),
    Periodic    UMETA(DisplayName = "Periodic (Ticks)")
};

USTRUCT(BlueprintType)
struct FEffectStatModifier
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Character.Stats"))
	FGameplayTag StatTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EModifierSourceType ModifierType = EModifierSourceType::Flat;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Value = 0.0f;
};

/**
 * Base Effect Definition
 * 
 * This is the "recipe" for an effect, not the active instance.
 * Think of it like a blueprint: one DataAsset can be applied to multiple characters.
 * 
 * Example: "Poison_DataAsset" can be applied to 10 different enemies,
 * creating 10 separate active effect instances.
 */
UCLASS(Blueprintable, BlueprintType)
class RPGSYSTEM_API URPGEffect : public UDataAsset
{
    GENERATED_BODY()

public:
    URPGEffect();

    // ========== IDENTIFICATION ==========
    
    /** Unique identifier for this effect (e.g., "Effect.Debuff.Poison") */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Stats")
	TArray<FEffectStatModifier> StatModifiers;
	
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Identity", meta = (Categories = "Effect"))
    FGameplayTag EffectTag;

    /** Display name for UI (e.g., "Deadly Poison") */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Identity")
    FText EffectName;

    /** Description shown in tooltips */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Identity", meta = (MultiLine = true))
    FText Description;

    /** Icon shown in UI */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Identity")
    TObjectPtr<UTexture2D> Icon;

    // ========== DURATION & TIMING ==========
    
    /** How this effect applies over time */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Duration")
    EEffectDurationType DurationType = EEffectDurationType::Instant;

    /** 
     * How long the effect lasts (in seconds)
     * - For Instant: Ignored (always 0)
     * - For Duration: Total duration before expiring
     * - For Periodic: Total duration (ticks happen within this time)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Duration", 
              meta = (EditCondition = "DurationType != EEffectDurationType::Instant", ClampMin = "0.0"))
    float Duration = 0.0f;

    /** 
     * For Periodic effects: Time between ticks
     * Example: Poison with Period=1.0 ticks every 1 second
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Duration",
              meta = (EditCondition = "DurationType == EEffectDurationType::Periodic", ClampMin = "0.1"))
    float Period = 1.0f;

    // ========== MAGNITUDE (POWER) ==========
    
    /** 
     * Base power of the effect
     * - For damage: damage amount
     * - For heal: heal amount
     * - For buff: stat modifier value
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Magnitude")
    float BaseMagnitude = 0.0f;

    /** 
     * Optional: Scale magnitude by source attribute
     * Example: Poison damage scales with caster's Intelligence
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Magnitude", meta = (Categories = "Attribute"))
    FGameplayTag MagnitudeScalingAttribute;

    /** 
     * Scaling coefficient (if using attribute scaling)
     * FinalMagnitude = BaseMagnitude + (SourceAttribute * ScalingCoefficient)
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Magnitude", 
              meta = (EditCondition = "MagnitudeScalingAttribute.IsValid()", ClampMin = "0.0"))
    float ScalingCoefficient = 0.0f;

    // ========== STACKING ==========
    
    /** How multiple applications of this effect interact */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Stacking")
    EEffectStackPolicy StackPolicy = EEffectStackPolicy::Refresh;

    /** Maximum number of stacks allowed (only relevant for StackMagnitude policy) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Stacking",
              meta = (EditCondition = "StackPolicy == EEffectStackPolicy::StackMagnitude", ClampMin = "1", ClampMax = "99"))
    int32 MaxStacks = 1;

    // ========== TAGS & CONDITIONS ==========
    
    /** Tags this effect grants to the target while active (e.g., "Status.Poisoned") */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Tags")
    FGameplayTagContainer GrantedTags;

    /** Effect will be removed if target gains any of these tags (e.g., cleanse effect) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Tags")
    FGameplayTagContainer RemoveOnTags;

    /** Effect can only be applied if target has these tags */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Tags")
    FGameplayTagContainer RequiredTargetTags;

    /** Effect cannot be applied if target has any of these tags (immunity) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Tags")
    FGameplayTagContainer BlockedByTags;

    // ========== GAMEPLAY CUES ==========
    
    /** 
     * Visual/audio cues to play when this effect is applied
     * Example: Particle effects, sound effects, screen shakes
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Presentation")
    FGameplayTag OnAppliedCue;

    /** Cue played when effect ticks (for Periodic effects) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Presentation",
              meta = (EditCondition = "DurationType == EEffectDurationType::Periodic"))
    FGameplayTag OnTickCue;

    /** Cue played when effect is removed/expires */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect|Presentation")
    FGameplayTag OnRemovedCue;

    // ========== UTILITY FUNCTIONS ==========
    
    UFUNCTION(BlueprintPure, Category = "Effect")
    bool IsInstant() const { return DurationType == EEffectDurationType::Instant; }

    UFUNCTION(BlueprintPure, Category = "Effect")
    bool IsPeriodic() const { return DurationType == EEffectDurationType::Periodic; }

    UFUNCTION(BlueprintPure, Category = "Effect")
    bool CanStack() const { return StackPolicy != EEffectStackPolicy::None; }

    /**
     * Calculate the final magnitude considering source attributes and stacks
     * @param SourceAttributeValue Value of the scaling attribute (if any)
     * @param CurrentStacks Number of stacks currently active
     * @return Final calculated magnitude
     */
    UFUNCTION(BlueprintPure, Category = "Effect")
    float CalculateMagnitude(float SourceAttributeValue, int32 CurrentStacks = 1) const;

    /**
     * Get total number of times a periodic effect will tick
     * @return Number of ticks (Duration / Period)
     */
    UFUNCTION(BlueprintPure, Category = "Effect")
    int32 GetTotalTicks() const;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
