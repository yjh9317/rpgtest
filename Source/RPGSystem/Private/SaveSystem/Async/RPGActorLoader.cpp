
#include "SaveSystem/Async/RPGActorLoader.h"
#include "SaveSystem/Subsystem/RPGSaveSubSystem.h"
#include "SaveSystem/Data/RPGSaveActors.h"
#include "Async/Async.h"
#include "TimerManager.h"

/**
Init
**/

//Always create via FRPGSaveLevelLoader::Create to ensure shared ownership.
FRPGSaveLevelLoader::FRPGSaveLevelLoader(URPGSaveSubsystem* URPGSaveSubSystem, const FLoaderInitData& InitData)
{
	RPGSaveSubsystem = URPGSaveSubSystem;

	SavedActors = InitData.InActorData;
	ActorMap = InitData.InActorMap;
	bIsWorldPartitionLoader = InitData.bIsWorldPartition;

	BatchSize = FMath::Max(1, int32(FSettingHelpers::GetLoadBatchSize()));
	CurrentIndex = 0;

	TotalProcessedCount = 0;
}

FRPGSaveLevelLoader::~FRPGSaveLevelLoader()
{
	//On Destroy
	if (!bCompleted)
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Loader was destroyed before being finished!"));
	}

	SavedActors.Empty();
	ActorMap.Empty();
	Batches.Empty();
}

TSharedRef<FRPGSaveLevelLoader> FRPGSaveLevelLoader::Create(URPGSaveSubsystem* URPGSaveSubSystem, const FLoaderInitData& InitData)
{
	return MakeShareable(new FRPGSaveLevelLoader(URPGSaveSubSystem, InitData));
}

void FRPGSaveLevelLoader::Start()
{
	if (!RPGSaveSubsystem)
	{
		return;
	}

	const bool bMulti = FSettingHelpers::IsMultiThreadLoading();
	const bool bDeferred = FSettingHelpers::IsDeferredLoading();

	//Distance based sorting
	FActorHelpers::SortLevelActors(SavedActors, RPGSaveSubsystem->GetPlayerController());

	SetLoaderTimer([bMulti, bDeferred](TSharedPtr<FRPGSaveLevelLoader> Loader)
	{
		if (Loader.IsValid())
		{
			if (bMulti)
			{
				Loader->LoadMultiThreaded();
			}
			else if (bDeferred)
			{
				Loader->LoadDeferred();
			}
			else
			{
				Loader->LoadDefault();
			}
		}
	});
}

/**
Default Loading
**/

void FRPGSaveLevelLoader::LoadDefault()
{
	for (const FActorSaveData& Data : SavedActors)
	{
		EvaluateAndProcess(Data);
	}

	FinishLoading();
}

/**
Multi-Thread Loading
**/

void FRPGSaveLevelLoader::LoadMultiThreaded()
{
	TWeakPtr<FRPGSaveLevelLoader> WeakPtr = AsShared();
	Async(EAsyncExecution::ThreadPool, [WeakPtr]()
	{
		if (TSharedPtr<FRPGSaveLevelLoader> Loader = WeakPtr.Pin())
		{
			Loader->GatherValidBatches();

			if (Loader->CheckCancel())
			{
				return;
			}

			Loader->ScheduleMainThreadBatchStart();
		}
	});
}

void FRPGSaveLevelLoader::GatherValidBatches()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("RPGSaveSubsystemActorLoader::GatherValidBatches"));
	SCOPE_CYCLE_COUNTER(STAT_RPGSaveLoader_Gather);

	TArray<TArray<FActorProcessData>> LocalBatches;
	TArray<FActorProcessData> CurrentBatch;

	int32 IterationCount = 0;

	for (const FActorSaveData& Data : SavedActors)
	{
		//Early exit
		if (CheckCancel())
		{
			return;
		}

		if (CheckActorLimits(IterationCount))
		{
			return;
		}

		TWeakObjectPtr<AActor> ActorPtr;
		if (ShouldEvaluateData(Data, ActorPtr))
		{
			CurrentBatch.Add({Data, ActorPtr});

			if (CurrentBatch.Num() >= BatchSize)
			{
				LocalBatches.Add(MoveTemp(CurrentBatch));
				CurrentBatch.Empty();
			}
		}
	}

	if (CurrentBatch.Num() > 0)
	{
		LocalBatches.Add(MoveTemp(CurrentBatch));
	}

	FScopeLock Lock(&LoadActorScope);

	Batches = MoveTemp(LocalBatches);
}

void FRPGSaveLevelLoader::ScheduleMainThreadBatchStart()
{
	if (CheckCancel())
	{
		return;
	}

	TWeakPtr<FRPGSaveLevelLoader> WeakPtr = AsShared();
	AsyncTask(ENamedThreads::GameThread, [WeakPtr]()
	{
		if (TSharedPtr<FRPGSaveLevelLoader> Loader = WeakPtr.Pin())
		{
			Loader->StartBatchTick();
		}
	});
}

void FRPGSaveLevelLoader::StartBatchTick()
{
	FScopeLock Lock(&LoadActorScope);

	if (!CheckCancel() && RPGSaveSubsystem && Batches.Num() > 0)
	{
		SetLoaderTimer([](TSharedPtr<FRPGSaveLevelLoader> Loader)
		{
			if (Loader.IsValid())
			{
				Loader->ProcessNextBatch();
			}
		});
	}
	else
	{
		FinishLoading();
	}
}

void FRPGSaveLevelLoader::ProcessNextBatch()
{
	if (CheckCancel())
	{
		return;
	}

	FScopeLock Lock(&LoadActorScope);

	if (bCompleted || Batches.Num() <= 0)
	{
		FinishLoading();
		return;
	}

	ProcessActorBatch(Batches[0]);
	Batches.RemoveAt(0);

	StartBatchTick();
}

void FRPGSaveLevelLoader::ProcessActorBatch(const TArray<FActorProcessData>& Batch)
{
	for (const FActorProcessData& Item : Batch)
	{
		ProcessActor(Item);
	}
}

/**
Deferred Loading
**/

void FRPGSaveLevelLoader::LoadDeferred()
{
	StartTick();
}

void FRPGSaveLevelLoader::StartTick()
{
	if (RPGSaveSubsystem)
	{
		SetLoaderTimer([](TSharedPtr<FRPGSaveLevelLoader> Loader)
		{
			if (Loader.IsValid())
			{
				Loader->Tick();
			}
		});
	}
}

void FRPGSaveLevelLoader::Tick()
{
	SCOPE_CYCLE_COUNTER(STAT_RPGSaveLoader_Tick);

	if (!RPGSaveSubsystem || bCompleted || SavedActors.Num() <= 0)
	{
		FinishLoading();
		return;
	}

	//Continue deferred load tick
	if (ShouldContinueTicking())
	{
		if (CurrentIndex < SavedActors.Num())
		{
			StartTick();
		}
		else
		{
			FinishLoading();
		}
	}
}

bool FRPGSaveLevelLoader::ShouldContinueTicking() 
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("RPGSaveSubsystemActorLoader::ShouldContinueTicking"));

	int32 IterationCount = 0;

	while (IterationCount < BatchSize && CurrentIndex < SavedActors.Num())
	{
		//Early exit
		if (CheckCancel()) 
		{ 
			return false; 
		}

		if(CheckActorLimits(IterationCount))
		{
			return false;
		}

		const FActorSaveData Data = SavedActors[CurrentIndex];
		EvaluateAndProcess(Data);

		++CurrentIndex;
	}

	return true;
}

/**
Modular helpers
**/

void FRPGSaveLevelLoader::EvaluateAndProcess(const FActorSaveData& Data)
{
	TWeakObjectPtr<AActor> ActorPtr;
	if (ShouldEvaluateData(Data, ActorPtr))
	{
		const FActorProcessData Item(Data, ActorPtr);
		ProcessActor(Item);
	}
}

void FRPGSaveLevelLoader::ProcessActor(const FActorProcessData& ProcessData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("RPGSaveSubsystemActorLoader::ProcessActor")); 

	AActor* Actor = ProcessData.ActorPtr.Get();
	const FActorSaveData ActorData = ProcessData.Data;
	const EActorType ActorType = EActorType(ActorData.Type);

	//Check for runtime Actors, make sure we never attempt to spawn any other type
	if (!IsValid(Actor) && FActorHelpers::IsRuntime(ActorType))
	{
		RPGSaveSubsystem->SpawnLevelActor(ActorData);
		return;
	}

	//Placed Actors
	if (IsValid(Actor) && !FActorHelpers::IsLoaded(Actor))
	{
		if (bIsWorldPartitionLoader)
		{
			RPGSaveSubsystem->LoadStreamingActor(Actor, ActorData);
		}
		else
		{
			RPGSaveSubsystem->ProcessLevelActor(Actor, ActorData);
		}
	}
}

bool FRPGSaveLevelLoader::ShouldEvaluateData(const FActorSaveData& Data, TWeakObjectPtr<AActor>& OutActorPtr) const
{
	const EActorType ActorType = EActorType(Data.Type);
	if (!FActorHelpers::IsLevelActor(ActorType, false))
	{
		return false;
	}

	//Find by simple name only. O(1)
	const FName Key = FActorHelpers::GetActorDataName(Data);
	if (const TWeakObjectPtr<AActor>* Ptr = ActorMap.Find(Key))
	{
		OutActorPtr = *Ptr;
	}
	else
	{
		OutActorPtr = nullptr;
	}

	const bool bValid = OutActorPtr.IsValid();

	if (!bValid && FActorHelpers::IsRuntime(ActorType))
	{
		return true;
	}

	if (bValid && !FActorHelpers::IsLoaded(OutActorPtr.Get()))
	{
		return true;
	}

	return false;
}

bool FRPGSaveLevelLoader::CheckActorLimits(int32& IterationCount)
{
    if (IterationCount >= MAX_ACTORS_PER_ITERATION)
    {
        UE_LOG(LogRPGSave, Warning, TEXT("Hit actor iteration limit"));
    }

    if (TotalProcessedCount >= MAX_TOTAL_ACTORS)
    {
        UE_LOG(LogRPGSave, Error, TEXT("Hit total actor limit"));
		FinishLoading();
        return true;
    }

    ++IterationCount;
    ++TotalProcessedCount;

    return false;
}

bool FRPGSaveLevelLoader::CheckCancel()
{
	FScopeLock Lock(&LoadActorScope);

	//Early exit if the load was canceled
	if (bCanceled || !RPGSaveSubsystem || !RPGSaveSubsystem->HasValidWorld() || IsEngineExitRequested())
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Exiting background load process: Session ended or world is invalid."));
		FinishLoading();
		return true;
	}

	return false;
}

void FRPGSaveLevelLoader::Destroy()
{
	bCanceled = true;

	SetLoaderTimer([](TSharedPtr<FRPGSaveLevelLoader> Loader)
	{
		if (Loader.IsValid())
		{
			Loader->FinishLoading();
		}
	});
}

template<typename FuncType>
void FRPGSaveLevelLoader::SetLoaderTimer(FuncType&& Callback)
{
	if (RPGSaveSubsystem)
	{
		TWeakPtr<FRPGSaveLevelLoader> WeakPtr = AsShared();
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick([WeakPtr, Callback = MoveTemp(Callback)]() mutable
		{
			if (TSharedPtr<FRPGSaveLevelLoader> Loader = WeakPtr.Pin())
			{
				Callback(Loader);
			}
		});
	}
}

/**
Finish 
**/

void FRPGSaveLevelLoader::FinishLoading()
{
	FScopeLock Lock(&LoadActorScope);

	if (bCompleted)
	{
		return;
	}

	bCompleted = true;

	//Capture a WeakPtr so we don’t keep the loader alive forever, but still fire the callback if it’s still around.
	TWeakPtr<FRPGSaveLevelLoader> WeakPtr = AsShared();

	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [WeakPtr]()
		{
			if (TSharedPtr<FRPGSaveLevelLoader> Pinned = WeakPtr.Pin())
			{
				Pinned->OnComplete.ExecuteIfBound();
			}
		});
	}
	else
	{
		if (TSharedPtr<FRPGSaveLevelLoader> Pinned = WeakPtr.Pin())
		{
			Pinned->OnComplete.ExecuteIfBound();
		}
	}
}

