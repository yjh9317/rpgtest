#pragma once

#include "SaveSystem/Data/RPGSaveData.h"
#include "SaveSystem/Data/RPGSaveMisc.h"
#include "SaveSystem/Data/RPGSaveLevel.h"
// #include "SaveSystem/Data/RPGSaveVersion.h"
#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/ThreadSafeBool.h"
#include "RPGActorLoader.generated.h"

class URPGSaveSubsystem;

DECLARE_DELEGATE(FOnLoaderComplete);

DECLARE_STATS_GROUP(TEXT("RPGSave"), STATGROUP_RPGSave, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("RPGSave Loader Tick"), STAT_RPGSaveLoader_Tick, STATGROUP_RPGSave);
DECLARE_CYCLE_STAT(TEXT("RPGSave Loader Gather"), STAT_RPGSaveLoader_Gather, STATGROUP_RPGSave);


USTRUCT()
struct FLoaderInitData
{
    GENERATED_BODY()

public:

    bool bIsWorldPartition;
    TArray<FActorSaveData> InActorData;

    UPROPERTY()
    TMap<FName, const TWeakObjectPtr<AActor>> InActorMap;

    FLoaderInitData()
        : bIsWorldPartition(false)
    {
    }

    FLoaderInitData(bool bInIsWorldPartition, const TArray<FActorSaveData>& InData, const TMap<FName, const TWeakObjectPtr<AActor>>& InMap)
        : bIsWorldPartition(bInIsWorldPartition), InActorData(InData), InActorMap(InMap)
    {
    }
};

class RPGSYSTEM_API FRPGSaveLevelLoader : public TSharedFromThis<FRPGSaveLevelLoader>
{

public:

    //Constructor and Destructor
    FRPGSaveLevelLoader(URPGSaveSubsystem* _RPGSaveSubsystem, const FLoaderInitData& InitData);
    ~FRPGSaveLevelLoader();

    //Factory
    static TSharedRef<FRPGSaveLevelLoader> Create(URPGSaveSubsystem* _RPGSaveSubsystem, const FLoaderInitData& InitData);

    //Start loading
    void Start();
    FOnLoaderComplete OnComplete;

private:

    //ProcessData holds save data + pre-resolved actor pointer
    struct FActorProcessData
    {
        FActorSaveData Data;
        TWeakObjectPtr<AActor> ActorPtr;

        FActorProcessData()
        {
        }

        FActorProcessData(const FActorSaveData& InData, const TWeakObjectPtr<AActor>& InActorPtr)
            : Data(InData), ActorPtr(InActorPtr)
        {
        }
    };

    void LoadDefault();
    void LoadMultiThreaded();
    void GatherValidBatches();
    void ScheduleMainThreadBatchStart();
    void LoadDeferred();
    void StartTick();
    void Tick();
    void FinishLoading();

    void StartBatchTick();
    void ProcessNextBatch();
    void ProcessActorBatch(const TArray<FActorProcessData>& Batch);

    void EvaluateAndProcess(const FActorSaveData& Data);
    bool ShouldEvaluateData(const FActorSaveData& Data, TWeakObjectPtr<AActor>& OutActorPtr) const;
    void ProcessActor(const FActorProcessData& ProcessData);
    bool ShouldContinueTicking();

    bool CheckActorLimits(int32& IterationCount);

    void Destroy();
    bool CheckCancel();

    template<typename FuncType>
    void SetLoaderTimer(FuncType&& Callback);

private:

    //Actor Load limits, likely never reached
    static constexpr int32 MAX_ACTORS_PER_ITERATION = 100000;
    static constexpr int32 MAX_TOTAL_ACTORS = 1000000;

    int32 TotalProcessedCount = 0;

    TObjectPtr<URPGSaveSubsystem> RPGSaveSubsystem;

    TArray<FActorSaveData> SavedActors;
    TArray<TArray<FActorProcessData>> Batches;

    TMap<FName, const TWeakObjectPtr<AActor>> ActorMap;

    FCriticalSection LoadActorScope;

    FThreadSafeBool bCanceled = false;
    FThreadSafeBool bCompleted = false;

    bool bIsWorldPartitionLoader = false;

    int32 CurrentIndex = 0;
    int32 BatchSize = 0;
};
