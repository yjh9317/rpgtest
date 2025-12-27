// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Async/RPGAsyncStream.h"

#include "SaveSystem/Async/RPGActorLoader.h"
#include "SaveSystem/Data/RPGSaveActors.h"
#include "SaveSystem/Data/RPGSaveMisc.h"
#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"

/**
Init
**/

URPGAsyncStream::URPGAsyncStream()
{
	Mode = ESaveGameMode::MODE_All;
	PrunedData = FMultiLevelStreamingData();
	bIsActive = false;
}

bool URPGAsyncStream::InitStreamingLoadTask(URPGSaveSubsystem* Subsystem, ULevel* InLevel)
{
	//Check to see if an instance with the same streaming level is active.
	if (FAsyncSaveHelpers::IsStreamAutoLoadActive(InLevel))
	{
		UE_LOG(LogRPGSave, Log, TEXT("Skipped loading streaming level while same async task is already active"));
		return false;
	}

	URPGAsyncStream* LoadTask = NewObject<URPGAsyncStream>(GetTransientPackage());
	if (LoadTask)
	{
		LoadTask->StreamingLevel = InLevel;
		LoadTask->RPGSaveSubsystem = Subsystem;
		LoadTask->RegisterWithGameInstance(Subsystem);
		LoadTask->Activate();
		return true;
	}

	return false;
}

void URPGAsyncStream::Activate()
{
	if (RPGSaveSubsystem)
	{
		bIsActive = true;
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncStream::StartLoad);
	}
}

void URPGAsyncStream::StartLoad()
{
	if (RPGSaveSubsystem)
	{
		if (!SetupLevelActors())
		{
			Deactivate();
			return;
		}

		//Spawn Loader
		const FLoaderInitData LoaderData = FLoaderInitData(true, PrunedData.ActorArray, StreamActorsMap);
		Loader = FRPGSaveLevelLoader::Create(RPGSaveSubsystem, LoaderData);
		Loader->OnComplete.BindUObject(this, &URPGAsyncStream::FinishLoadingStreamLevel);
		Loader->Start();
	}
}

bool URPGAsyncStream::SetupLevelActors()
{
	if (!RPGSaveSubsystem || !StreamingLevel || RPGSave::ArrayEmpty(StreamingLevel->Actors))
	{
		return false;
	}

	const int32 EstimatedSize = StreamingLevel->Actors.Num();
	StreamActorsMap.Reserve(EstimatedSize);

	//This is somewhat like the Prepare Actors function
	for (AActor* Actor : StreamingLevel->Actors)
	{
		if (RPGSaveSubsystem->IsValidActor(Actor) 
			&& FActorHelpers::IsPlacedActor(Actor) 
			&& !FActorHelpers::IsLoaded(Actor) 
			&& !FActorHelpers::IsSkipSave(Actor))
		{
			const FName ActorName(*FActorHelpers::GetFullActorName(Actor));
			StreamActorsMap.Add(ActorName, Actor);
		}
	}

	if (RPGSave::ArrayEmpty(StreamActorsMap))
	{
		return false;
	}

	//Prune saved Actor data to prevent looking up huge arrays
	TArray<FActorSaveData> PrunedActors = RPGSaveSubsystem->GetMultiLevelStreamData().ActorArray;
	FActorHelpers::PruneSavedActors(StreamActorsMap, PrunedActors);
	PrunedData.CopyActors(PrunedActors);

	if (!PrunedData.HasLevelActors())
	{
		return false;
	}

	TotalActors = StreamActorsMap.Num();
	RPGSaveSubsystem->AllocateRealLoadedActors(TotalActors);

	return true;
}

/**
Finish
**/

void URPGAsyncStream::FinishLoadingStreamLevel()
{
	UE_LOG(LogRPGSave, Log, TEXT("Loaded %d World Partition Actors"), TotalActors);
	EndTask(true);
}

void URPGAsyncStream::ForceDestroy()
{
	if (RPGSaveSubsystem)
	{
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncStream::Deactivate);
	}
	else
	{
		Deactivate();
	}
}

void URPGAsyncStream::Deactivate()
{
	EndTask(false);
}

void URPGAsyncStream::EndTask(const bool bBroadcastFinish)
{
	if (bBroadcastFinish && RPGSaveSubsystem)
	{
		RPGSaveSubsystem->BroadcastOnPartitionLoaded();
	}

	bIsActive = false;
	SetReadyToDestroy();
}

