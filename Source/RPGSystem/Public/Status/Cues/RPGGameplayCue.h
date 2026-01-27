// RPGGameplayCue.h
// Base class for gameplay "cues" - the visual/audio feedback for effects
// Examples: particle effects, sounds, camera shakes, screen effects

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagContainer.h"
#include "RPGGameplayCue.generated.h"

class UNiagaraSystem;
class USoundBase;
class UCameraShakeBase;

/**
 * Context information for playing a cue
 * Contains all the "where, who, what" information needed to play effects
 */
USTRUCT(BlueprintType)
struct FCueContext
{
    GENERATED_BODY()

    /** Primary location where the cue should play */
    UPROPERTY(BlueprintReadWrite, Category = "Cue Context")
    FVector Location = FVector::ZeroVector;

    /** Optional rotation for directional effects */
    UPROPERTY(BlueprintReadWrite, Category = "Cue Context")
    FRotator Rotation = FRotator::ZeroRotator;

    /** Target actor (for attached effects or actor-specific cues) */
    UPROPERTY(BlueprintReadWrite, Category = "Cue Context")
    TWeakObjectPtr<AActor> TargetActor = nullptr;

    /** Source actor (who caused this cue) */
    UPROPERTY(BlueprintReadWrite, Category = "Cue Context")
    TWeakObjectPtr<AActor> SourceActor = nullptr;

    /** Optional magnitude/intensity (e.g., for scaling particles or sound volume) */
    UPROPERTY(BlueprintReadWrite, Category = "Cue Context")
    float Magnitude = 1.0f;

    /** Optional custom data for specialized cues */
    UPROPERTY(BlueprintReadWrite, Category = "Cue Context")
    TMap<FName, float> CustomParameters;

    FCueContext()
        : Magnitude(1.0f)
    {}
};

/**
 * How a cue executes
 * - Burst: Play once and forget (e.g., hit impact)
 * - Looping: Play and keep looping until stopped (e.g., burning effect)
 * - ActorSpawn: Spawn an actor that manages its own lifetime
 */
UENUM(BlueprintType)
enum class ECueExecutionType : uint8
{
    Burst       UMETA(DisplayName = "Burst (One-Shot)"),
    Looping     UMETA(DisplayName = "Looping (Continuous)"),
    ActorSpawn  UMETA(DisplayName = "Spawn Actor")
};

/**
 * Base class for Gameplay Cues
 * 
 * DESIGN PHILOSOPHY:
 * Cues are the "presentation layer" of gameplay. They're PURE FEEDBACK - they don't
 * affect gameplay logic. Think of them as the "flash and bang" that makes effects
 * feel good.
 * 
 * SEPARATION OF CONCERNS:
 * - Effects (URPGEffect) = WHAT happens (damage, buff, debuff)
 * - Cues (URPGGameplayCue) = HOW it looks/sounds
 * 
 * This separation allows:
 * - Artists to iterate on visuals without touching gameplay
 * - Same effect to have different visuals (e.g., fire poison vs ice poison)
 * - Client-side prediction (cues can play immediately, effects sync later)
 */
UCLASS(Blueprintable, BlueprintType, Abstract)
class RPGSYSTEM_API URPGGameplayCue : public UObject
{
    GENERATED_BODY()

public:
    URPGGameplayCue();

    // ========== IDENTIFICATION ==========
    
    /** Tag that triggers this cue (e.g., "Cue.Impact.Sword") */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Identity", meta = (Categories = "Cue"))
    FGameplayTag CueTag;

    /** Display name for debugging */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Identity")
    FText CueName;

    /** How this cue executes */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue|Execution")
    ECueExecutionType ExecutionType = ECueExecutionType::Burst;

    // ========== EXECUTION ==========
    
    /**
     * Execute the cue
     * @param Context Information about where/who/what to play
     * @return True if successfully executed
     */
    UFUNCTION(BlueprintCallable, Category = "Cue")
    bool Execute(const FCueContext& Context);

    /**
     * Stop a looping cue
     * @param TargetActor The actor to stop the cue on
     */
    UFUNCTION(BlueprintCallable, Category = "Cue")
    virtual void Stop(AActor* TargetActor);

    /**
     * Check if this cue is currently playing on an actor
     */
    UFUNCTION(BlueprintPure, Category = "Cue")
    virtual bool IsPlaying(AActor* TargetActor) const;

protected:
    // ========== IMPLEMENTATION HOOKS ==========
    
    /**
     * Override this to implement burst-style cues
     * Called once when the cue is triggered
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Cue")
    void OnExecuteBurst(const FCueContext& Context);
    virtual void OnExecuteBurst_Implementation(const FCueContext& Context) {}

    /**
     * Override this to implement looping cues (start)
     * Called when the cue starts looping
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Cue")
    void OnExecuteLooping(const FCueContext& Context);
    virtual void OnExecuteLooping_Implementation(const FCueContext& Context) {}

    /**
     * Override this to implement actor spawn cues
     * Should spawn and return an actor that manages itself
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Cue")
    AActor* OnExecuteSpawn(const FCueContext& Context);
    virtual AActor* OnExecuteSpawn_Implementation(const FCueContext& Context) { return nullptr; }

    /**
     * Override this to cleanup looping cues
     * Called when Stop() is invoked
     */
    UFUNCTION(BlueprintNativeEvent, Category = "Cue")
    void OnStop(AActor* TargetActor);
    virtual void OnStop_Implementation(AActor* TargetActor) {}

    // ========== HELPER FUNCTIONS ==========
    
    /**
     * Get the world context (for spawning effects)
     */
    UWorld* GetWorld() const;

    /**
     * Spawn a particle system at the given context
     */
    void SpawnParticleEffect(UNiagaraSystem* ParticleSystem, const FCueContext& Context, bool bAttach = false);

    /**
     * Play a sound at the given context
     */
    void PlaySound(USoundBase* Sound, const FCueContext& Context, bool bAttach = false);

    /**
     * Apply camera shake to nearby players
     */
    void ApplyCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, const FCueContext& Context, float InnerRadius = 0.0f, float OuterRadius = 1000.0f);
};

/**
 * Specialized cue for Niagara particle effects
 * Most common cue type - plays a particle effect and optionally a sound
 */
UCLASS(Blueprintable)
class RPGSYSTEM_API URPGCue_Niagara : public URPGGameplayCue
{
    GENERATED_BODY()

public:
    /** Niagara system to spawn */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Niagara")
    TObjectPtr<UNiagaraSystem> ParticleSystem = nullptr;

    /** Should the effect attach to the target actor? */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Niagara")
    bool bAttachToTarget = false;

    /** Socket name to attach to (if bAttachToTarget is true) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Niagara", meta = (EditCondition = "bAttachToTarget"))
    FName AttachSocketName = NAME_None;

    /** Scale the particle effect based on context magnitude */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Niagara")
    bool bScaleByMagnitude = false;

    /** Optional sound to play with the effect */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    TObjectPtr<USoundBase> Sound = nullptr;

    /** Optional camera shake */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
    TSubclassOf<UCameraShakeBase> CameraShake;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (EditCondition = "CameraShake != nullptr"))
    float ShakeInnerRadius = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera", meta = (EditCondition = "CameraShake != nullptr"))
    float ShakeOuterRadius = 1000.0f;

protected:
    virtual void OnExecuteBurst_Implementation(const FCueContext& Context) override;
    virtual void OnExecuteLooping_Implementation(const FCueContext& Context) override;
    virtual void OnStop_Implementation(AActor* TargetActor) override;

private:
    /** Track active looping particle components */
    UPROPERTY(Transient)
    TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<class UNiagaraComponent>> ActiveLoopingComponents;
};

/**
 * Simple sound-only cue
 */
UCLASS(Blueprintable)
class RPGSYSTEM_API URPGCue_Sound : public URPGGameplayCue
{
    GENERATED_BODY()

public:
    /** Sound to play */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    TObjectPtr<USoundBase> Sound = nullptr;

    /** Should the sound attach to the target actor? */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    bool bAttachToTarget = false;

    /** Volume multiplier */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    float VolumeMultiplier = 1.0f;

    /** Pitch multiplier */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
    float PitchMultiplier = 1.0f;

protected:
    virtual void OnExecuteBurst_Implementation(const FCueContext& Context) override;
};

/**
 * Camera shake cue
 */
UCLASS(Blueprintable)
class RPGSYSTEM_API URPGCue_CameraShake : public URPGGameplayCue
{
    GENERATED_BODY()

public:
    /** Camera shake to apply */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

    /** Inner radius (full intensity) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shake")
    float InnerRadius = 0.0f;

    /** Outer radius (falloff to zero) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shake")
    float OuterRadius = 1000.0f;

    /** Scale shake by magnitude */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera Shake")
    bool bScaleByMagnitude = false;

protected:
    virtual void OnExecuteBurst_Implementation(const FCueContext& Context) override;
};
