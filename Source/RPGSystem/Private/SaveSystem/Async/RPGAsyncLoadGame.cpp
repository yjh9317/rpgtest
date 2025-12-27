// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Async/RPGAsyncLoadGame.h"

#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"

URPGAsyncLoadGame::URPGAsyncLoadGame()
{
	Mode = ESaveGameMode::MODE_All;
	Data = 0;
	bIsActive = false;
	bLoadFailed = false;
	bFullReload = false;
	bAutoLoadLevel = false;
}

URPGAsyncLoadGame* URPGAsyncLoadGame::AsyncLoadActors(UObject* WorldContextObject, int32 Data, bool bFullReload)
{
	if (URPGSaveSubsystem* RPGSaveSubsystemObject = URPGSaveSubsystem::Get(WorldContextObject))
	{
		const ESaveGameMode Mode = FAsyncSaveHelpers::GetMode(Data);

		if (RPGSaveSubsystemObject->IsAsyncSaveOrLoadTaskActive(Mode))
		{
			return nullptr;
		}

		URPGAsyncLoadGame* LoadTask = NewObject<URPGAsyncLoadGame>(GetTransientPackage());		
		if(LoadTask)
		{
			LoadTask->RPGSaveSubsystem = RPGSaveSubsystemObject;
			LoadTask->Data = Data;
			LoadTask->Mode = Mode;
			LoadTask->bFullReload = bFullReload;
			return LoadTask;
		}
	}

	return nullptr;
}

void URPGAsyncLoadGame::AutoLoadLevelActors(URPGSaveSubsystem* Subsystem)
{
	URPGAsyncLoadGame* LoadTask = NewObject<URPGAsyncLoadGame>(GetTransientPackage());
	if (LoadTask)
	{
		LoadTask->RPGSaveSubsystem = Subsystem;
		LoadTask->Data = ENUM_TO_FLAG(ELoadTypeFlags::LF_Level);
		LoadTask->Mode = ESaveGameMode::MODE_Level;
		LoadTask->bFullReload = false;
		LoadTask->bAutoLoadLevel = true;
		LoadTask->RegisterWithGameInstance(Subsystem);
		LoadTask->Activate();
	}
}

void URPGAsyncLoadGame::Activate()
{
	if (RPGSaveSubsystem)
	{
		const bool bIsManualLevelLoad = (Data & ENUM_TO_FLAG(ELoadTypeFlags::LF_Level)) && !bAutoLoadLevel;
		if (bIsManualLevelLoad)
		{
			//Warn when still streaming
			if (RPGSaveSubsystem->IsLevelStreaming())
			{
				UE_LOG(LogRPGSave, Warning, TEXT("Async load while streaming. Use 'Is Level Streaming Active' to check."));
			}

			//Warn when trying to manually load WP
			if (RPGSaveSubsystem->AutoSaveLoadWorldPartition() && !RPGSaveSubsystem->SkipInitialWorldPartitionLoad())
			{
				UE_LOG(LogRPGSave, Warning, TEXT("Manual World Partition load without 'Skip Initial Load' enabled in the settings. This can lead to issues."));
			}
		}

		bIsActive = true;
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::PreLoading);
	}
}

/**
Load Actors
**/

void URPGAsyncLoadGame::PreLoading()
{
	if (!RPGSaveSubsystem)
	{
		return;
	}

	if (Data & ENUM_TO_FLAG(ELoadTypeFlags::LF_Player))
	{
		if (!RPGSaveSubsystem->HasValidPlayer())
		{
			RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::PreLoading);
			return;
		}
	}

	if (Data & ENUM_TO_FLAG(ELoadTypeFlags::LF_Level))
	{
		if (!RPGSaveSubsystem->HasValidGameMode())
		{
			RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::PreLoading);
			return;
		}
	}

	const EPrepareType PrepareType = bFullReload ? EPrepareType::PT_FullReload : EPrepareType::PT_Default;
	RPGSaveSubsystem->PrepareLoadAndSaveActors(Data, EAsyncCheckType::CT_Load, PrepareType);

	RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::StartLoading);

	const float WaitTime = URPGSaveProjectSetting::Get()->AsyncWaitDelay;
	if (WaitTime > 0.f)
	{
		RPGSaveSubsystem->GetTimerManager().SetTimer(FailedTimerHandle, FTimerDelegate::CreateUObject(this, &URPGAsyncLoadGame::FailLoadingTask), WaitTime, false);
	}
}

void URPGAsyncLoadGame::ClearFailTimer()
{
	if (RPGSaveSubsystem)
	{
		if (FailedTimerHandle.IsValid())
		{
			RPGSaveSubsystem->GetTimerManager().ClearTimer(FailedTimerHandle);
		}
	}
}

void URPGAsyncLoadGame::StartLoading()
{
	PreparePlayer();
}

/**
Player Actors
**/

void URPGAsyncLoadGame::PreparePlayer()
{
	bLoadFailed = true;

	if (!RPGSaveSubsystem)
	{
		return;
	}

	if (Data & ENUM_TO_FLAG(ELoadTypeFlags::LF_Player))
	{
		if (RPGSaveSubsystem->TryLoadPlayerFile())
		{
			SetLoadNotFailed();
			RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::LoadPlayer);
			return;
		}
	}

	RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::PrepareLevel);
}

void URPGAsyncLoadGame::LoadPlayer()
{
	if (RPGSaveSubsystem)
	{
		RPGSaveSubsystem->LoadPlayerActors(RPGSaveSubsystem->GetPlayerController());
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::PrepareLevel);
	}
}

/**
Level Actors
**/

void URPGAsyncLoadGame::PrepareLevel()
{ 
	if (!RPGSaveSubsystem)
	{
		return;
	}

	if (Data & ENUM_TO_FLAG(ELoadTypeFlags::LF_Level))
	{
		if (RPGSaveSubsystem->TryLoadLevelFile())
		{
			SetLoadNotFailed();
			RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::LoadGameMode);
		}
		else
		{
			FinishLoading();
		}
	}
	else
	{
		FinishLoading();
	}
}

void URPGAsyncLoadGame::LoadGameMode()
{
	if (RPGSaveSubsystem)
	{
		RPGSaveSubsystem->LoadGameMode();
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::LoadLevelScripts);
	}
}

void URPGAsyncLoadGame::LoadLevelScripts()
{
	if (RPGSaveSubsystem)
	{
		RPGSaveSubsystem->LoadLevelScripts();
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::PrepareLevelActors);
	}
}

void URPGAsyncLoadGame::PrepareLevelActors()
{
	if (RPGSaveSubsystem)
	{
		RPGSaveSubsystem->PrepareLevelActors();
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::StartLoadLevelActors);
	}
}

void URPGAsyncLoadGame::StartLoadLevelActors()
{
	if (RPGSaveSubsystem)
	{
		RPGSaveSubsystem->LoadLevelActors(this);
	}
}

void URPGAsyncLoadGame::LoadLevelActors(const FLoaderInitData& LoaderData)
{
	//Spawn Loader
	Loader = FRPGSaveLevelLoader::Create(RPGSaveSubsystem, LoaderData);
	Loader->OnComplete.BindUObject(this, &URPGAsyncLoadGame::FinishLoading);
	Loader->Start();
}

/**
Finish
**/

void URPGAsyncLoadGame::FinishLoading()
{
	//Has to be a tick before broadcast.
	bIsActive = false;
	ClearFailTimer();

	if (!RPGSaveSubsystem)
	{
		SetReadyToDestroy();
		return;
	}

	if (Data & ENUM_TO_FLAG(ELoadTypeFlags::LF_Level))
	{
		RPGSaveSubsystem->FinishLoadingLevel(!bLoadFailed);
	}

	if (bLoadFailed)
	{
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::FailLoadingTask);
	}
	else
	{
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncLoadGame::CompleteLoadingTask);
	}
}

void URPGAsyncLoadGame::ForceDestroy()
{
	bIsActive = false;
	SetReadyToDestroy();
}

void URPGAsyncLoadGame::CompleteLoadingTask()
{
	OnCompleted.Broadcast();
	SetReadyToDestroy();
}

void URPGAsyncLoadGame::FailLoadingTask()
{
	OnFailed.Broadcast();
	SetReadyToDestroy();
}
