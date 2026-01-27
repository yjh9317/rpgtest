// CueManagerSubsystem.cpp


#include "Status/Cues/CueManagerSubsystem.h"
#include "Engine/World.h"

// ========== LIFECYCLE ==========

void UCueManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // Auto-register cue sets specified in the Game Instance
    for (URPGCueSet* CueSet : CueSetsToRegister)
    {
        if (CueSet)
        {
            RegisterCueSet(CueSet);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CueManagerSubsystem: Initialized with %d registered cues"), RegisteredCues.Num());
}

void UCueManagerSubsystem::Deinitialize()
{
    // Cleanup all registered cues
    RegisteredCues.Empty();
    ActiveLoopingCues.Empty();

    Super::Deinitialize();
}

// ========== CUE REGISTRATION ==========

void UCueManagerSubsystem::RegisterCue(FGameplayTag CueTag, TSubclassOf<URPGGameplayCue> CueClass)
{
    if (!CueTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("CueManagerSubsystem::RegisterCue - Invalid CueTag"));
        return;
    }

    if (!CueClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("CueManagerSubsystem::RegisterCue - Invalid CueClass for tag %s"), *CueTag.ToString());
        return;
    }

    // Create instance of the cue
    URPGGameplayCue* CueInstance = NewObject<URPGGameplayCue>(this, CueClass);
    if (CueInstance)
    {
        CueInstance->CueTag = CueTag; // Ensure tag is set
        RegisteredCues.Add(CueTag, CueInstance);
        
        UE_LOG(LogTemp, Log, TEXT("CueManagerSubsystem: Registered cue '%s' -> %s"), 
               *CueTag.ToString(), *CueClass->GetName());
    }
}

void UCueManagerSubsystem::RegisterCueInstance(FGameplayTag CueTag, URPGGameplayCue* CueInstance)
{
    if (!CueTag.IsValid() || !CueInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("CueManagerSubsystem::RegisterCueInstance - Invalid parameters"));
        return;
    }

    // If cue already exists, unregister it first
    if (RegisteredCues.Contains(CueTag))
    {
        UnregisterCue(CueTag);
    }

    CueInstance->CueTag = CueTag;
    RegisteredCues.Add(CueTag, CueInstance);
    
    UE_LOG(LogTemp, Log, TEXT("CueManagerSubsystem: Registered cue instance '%s'"), *CueTag.ToString());
}

void UCueManagerSubsystem::UnregisterCue(FGameplayTag CueTag)
{
    if (RegisteredCues.Contains(CueTag))
    {
        RegisteredCues.Remove(CueTag);
        UE_LOG(LogTemp, Log, TEXT("CueManagerSubsystem: Unregistered cue '%s'"), *CueTag.ToString());
    }
}

void UCueManagerSubsystem::RegisterCueSet(URPGCueSet* CueSet)
{
    if (!CueSet)
    {
        UE_LOG(LogTemp, Warning, TEXT("CueManagerSubsystem::RegisterCueSet - Null CueSet"));
        return;
    }

    int32 RegisteredCount = 0;
    
    for (const auto& CuePair : CueSet->Cues)
    {
        if (CuePair.Key.IsValid() && CuePair.Value)
        {
            RegisterCue(CuePair.Key, CuePair.Value);
            RegisteredCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("CueManagerSubsystem: Registered CueSet '%s' (%d cues)"), 
           *CueSet->GetName(), RegisteredCount);
}

// ========== CUE EXECUTION ==========

bool UCueManagerSubsystem::PlayCue(FGameplayTag CueTag, const FCueContext& Context)
{
    auto* CuePtr = RegisteredCues.Find(CueTag);
    if (!CuePtr || !(*CuePtr))
    {
        UE_LOG(LogTemp, Warning, TEXT("CueManagerSubsystem::PlayCue - Cue not found for tag %s"), *CueTag.ToString());
        return false;
    }

    URPGGameplayCue* Cue = *CuePtr;
    
    // Execute the cue
    bool bSuccess = Cue->Execute(Context);

    // Track looping cues
    if (bSuccess && Cue->ExecutionType == ECueExecutionType::Looping && Context.TargetActor.IsValid())
    {
        TrackLoopingCue(CueTag, Context.TargetActor.Get());
    }

    return bSuccess;
}

bool UCueManagerSubsystem::PlayCueAtLocation(FGameplayTag CueTag, FVector Location, AActor* OptionalSource)
{
    FCueContext Context;
    Context.Location = Location;
    Context.SourceActor = OptionalSource;
    
    return PlayCue(CueTag, Context);
}

bool UCueManagerSubsystem::PlayCueOnActor(FGameplayTag CueTag, AActor* TargetActor, AActor* OptionalSource)
{
    if (!TargetActor)
    {
        return false;
    }

    FCueContext Context;
    Context.TargetActor = TargetActor;
    Context.SourceActor = OptionalSource;
    Context.Location = TargetActor->GetActorLocation();
    
    return PlayCue(CueTag, Context);
}

bool UCueManagerSubsystem::StopCue(FGameplayTag CueTag, AActor* TargetActor)
{
    if (!TargetActor)
    {
        return false;
    }

    auto* CuePtr = RegisteredCues.Find(CueTag);
    if (!CuePtr || !(*CuePtr))
    {
        return false;
    }

    URPGGameplayCue* Cue = *CuePtr;
    Cue->Stop(TargetActor);
    
    UntrackLoopingCue(CueTag, TargetActor);
    
    return true;
}

int32 UCueManagerSubsystem::StopAllCuesOnActor(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return 0;
    }

    TArray<FGameplayTag>* ActiveCues = ActiveLoopingCues.Find(TargetActor);
    if (!ActiveCues || ActiveCues->Num() == 0)
    {
        return 0;
    }

    int32 StoppedCount = 0;
    
    // Make a copy since StopCue modifies the array
    TArray<FGameplayTag> CuesToStop = *ActiveCues;
    
    for (const FGameplayTag& CueTag : CuesToStop)
    {
        if (StopCue(CueTag, TargetActor))
        {
            StoppedCount++;
        }
    }

    return StoppedCount;
}

// ========== QUERIES ==========

bool UCueManagerSubsystem::IsCueRegistered(FGameplayTag CueTag) const
{
    return RegisteredCues.Contains(CueTag);
}

bool UCueManagerSubsystem::IsCuePlaying(FGameplayTag CueTag, AActor* TargetActor) const
{
    if (!TargetActor)
    {
        return false;
    }

    const TArray<FGameplayTag>* ActiveCues = ActiveLoopingCues.Find(TargetActor);
    if (!ActiveCues)
    {
        return false;
    }

    return ActiveCues->Contains(CueTag);
}

TArray<FGameplayTag> UCueManagerSubsystem::GetAllRegisteredCueTags() const
{
    TArray<FGameplayTag> Tags;
    RegisteredCues.GetKeys(Tags);
    return Tags;
}

// ========== INTERNAL TRACKING ==========

void UCueManagerSubsystem::TrackLoopingCue(FGameplayTag CueTag, AActor* TargetActor)
{
    if (!TargetActor || !CueTag.IsValid())
    {
        return;
    }

    TArray<FGameplayTag>& ActiveCues = ActiveLoopingCues.FindOrAdd(TargetActor);
    ActiveCues.AddUnique(CueTag);
}

void UCueManagerSubsystem::UntrackLoopingCue(FGameplayTag CueTag, AActor* TargetActor)
{
    if (!TargetActor || !CueTag.IsValid())
    {
        return;
    }

    TArray<FGameplayTag>* ActiveCues = ActiveLoopingCues.Find(TargetActor);
    if (ActiveCues)
    {
        ActiveCues->Remove(CueTag);
        
        // Remove actor entry if no more active cues
        if (ActiveCues->Num() == 0)
        {
            ActiveLoopingCues.Remove(TargetActor);
        }
    }
}

// ========== DEBUGGING ==========

void UCueManagerSubsystem::DebugPrintRegisteredCues() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Registered Cues (%d) ==="), RegisteredCues.Num());
    
    for (const auto& CuePair : RegisteredCues)
    {
        URPGGameplayCue* Cue = CuePair.Value;
        if (Cue)
        {
            UE_LOG(LogTemp, Log, TEXT("  [%s] -> %s (%s)"),
                   *CuePair.Key.ToString(),
                   *Cue->CueName.ToString(),
                   *Cue->GetClass()->GetName());
        }
    }
}

void UCueManagerSubsystem::DebugPrintActiveLoopingCues() const
{
    UE_LOG(LogTemp, Log, TEXT("=== Active Looping Cues ==="));
    
    for (const auto& ActorPair : ActiveLoopingCues)
    {
        AActor* Actor = ActorPair.Key.Get();
        if (Actor)
        {
            UE_LOG(LogTemp, Log, TEXT("  Actor: %s"), *Actor->GetName());
            for (const FGameplayTag& CueTag : ActorPair.Value)
            {
                UE_LOG(LogTemp, Log, TEXT("    - %s"), *CueTag.ToString());
            }
        }
    }
}

// ========== CUESET ==========

TArray<FGameplayTag> URPGCueSet::GetCueTags() const
{
    TArray<FGameplayTag> Tags;
    Cues.GetKeys(Tags);
    return Tags;
}
