// CueManagerSubsystem.h
// Game Instance Subsystem that manages all gameplay cues globally
// This is a singleton that lives for the entire game session

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "RPGGameplayCue.h"
#include "CueManagerSubsystem.generated.h"

/**
 * Global manager for Gameplay Cues
 * 
 * RESPONSIBILITIES:
 * - Register cue definitions (tag -> cue class mapping)
 * - Play cues when requested (by tag)
 * - Stop looping cues
 * - Manage cue pooling/optimization (future)
 * 
 * DESIGN PATTERN: Singleton Subsystem
 * This is a game-wide service that any system can access to play cues.
 * It decouples "who wants to play a cue" from "how cues are played".
 * 
 * USAGE:
 * ```cpp
 * UCueManagerSubsystem* CueManager = GetWorld()->GetGameInstance()->GetSubsystem<UCueManagerSubsystem>();
 * CueManager->PlayCue(FGameplayTag::RequestGameplayTag("Cue.Impact.Sword"), Context);
 * ```
 */
UCLASS()
class RPGSYSTEM_API UCueManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ========== LIFECYCLE ==========
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    /**
     * Map of GameplayTag -> Cue Instance
     * Example: "Cue.Impact.Sword" -> URPGCue_Niagara instance
     */
    UPROPERTY()
    TMap<FGameplayTag, TObjectPtr<URPGGameplayCue>> RegisteredCues;

    /**
     * Map of TargetActor -> Active Looping Cues
     * Tracks which looping cues are playing on which actors
     */
    TMap<TWeakObjectPtr<AActor>, TArray<FGameplayTag>> ActiveLoopingCues;

    /**
     * Data assets containing cue definitions to auto-register
     * Set this in your Game Instance class
     */
    UPROPERTY(EditDefaultsOnly, Category = "Cue Manager")
    TArray<TObjectPtr<class URPGCueSet>> CueSetsToRegister;

public:
    // ========== CUE REGISTRATION ==========
    
    /**
     * Register a cue class to a specific tag
     * This creates an instance of the cue and stores it for later use
     * 
     * @param CueTag The tag that will trigger this cue
     * @param CueClass The cue class to instantiate
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    void RegisterCue(FGameplayTag CueTag, TSubclassOf<URPGGameplayCue> CueClass);

    /**
     * Register a cue instance directly
     * Useful if you've already created a configured cue instance
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    void RegisterCueInstance(FGameplayTag CueTag, URPGGameplayCue* CueInstance);

    /**
     * Unregister a cue
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    void UnregisterCue(FGameplayTag CueTag);

    /**
     * Register all cues from a CueSet data asset
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    void RegisterCueSet(URPGCueSet* CueSet);

    // ========== CUE EXECUTION ==========
    
    /**
     * Play a cue by tag
     * 
     * @param CueTag The tag of the cue to play (must be registered first)
     * @param Context Information about where/who/what to play
     * @return True if cue was found and played successfully
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    bool PlayCue(FGameplayTag CueTag, const FCueContext& Context);

    /**
     * Convenience function: Play a cue at a location
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    bool PlayCueAtLocation(FGameplayTag CueTag, FVector Location, AActor* OptionalSource = nullptr);

    /**
     * Convenience function: Play a cue on an actor
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    bool PlayCueOnActor(FGameplayTag CueTag, AActor* TargetActor, AActor* OptionalSource = nullptr);

    /**
     * Stop a looping cue on a specific actor
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    bool StopCue(FGameplayTag CueTag, AActor* TargetActor);

    /**
     * Stop all looping cues on an actor
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager")
    int32 StopAllCuesOnActor(AActor* TargetActor);

    // ========== QUERIES ==========
    
    /**
     * Check if a cue is registered
     */
    UFUNCTION(BlueprintPure, Category = "Cue Manager")
    bool IsCueRegistered(FGameplayTag CueTag) const;

    /**
     * Check if a looping cue is currently playing on an actor
     */
    UFUNCTION(BlueprintPure, Category = "Cue Manager")
    bool IsCuePlaying(FGameplayTag CueTag, AActor* TargetActor) const;

    /**
     * Get all registered cue tags
     */
    UFUNCTION(BlueprintPure, Category = "Cue Manager")
    TArray<FGameplayTag> GetAllRegisteredCueTags() const;

protected:
    /**
     * Track that a looping cue started on an actor
     */
    void TrackLoopingCue(FGameplayTag CueTag, AActor* TargetActor);

    /**
     * Remove tracking for a looping cue
     */
    void UntrackLoopingCue(FGameplayTag CueTag, AActor* TargetActor);

public:
    // ========== DEBUGGING ==========
    
    /**
     * Print all registered cues to log
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager|Debug")
    void DebugPrintRegisteredCues() const;

    /**
     * Print all active looping cues to log
     */
    UFUNCTION(BlueprintCallable, Category = "Cue Manager|Debug")
    void DebugPrintActiveLoopingCues() const;
};

/**
 * Data Asset that contains a set of cue definitions
 * Allows designers to group related cues together
 * 
 * Example: "CombatCues" asset contains all hit effects, blood splatter, etc.
 */
UCLASS(BlueprintType)
class RPGSYSTEM_API URPGCueSet : public UDataAsset
{
    GENERATED_BODY()

public:
    /** Array of cue class definitions */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cue Set")
    TMap<FGameplayTag, TSubclassOf<URPGGameplayCue>> Cues;

    /**
     * Get all cue tags in this set
     */
    UFUNCTION(BlueprintPure, Category = "Cue Set")
    TArray<FGameplayTag> GetCueTags() const;
};
