// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Async/RPGAsyncSaveGame.h"
#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"


URPGAsyncSaveGame::URPGAsyncSaveGame()
{
	Mode = ESaveGameMode::MODE_All;
	Data = 0;
	bIsActive = false;
	bFinishedStep = false;
	bHasFailed = false;
	bAutoSaveLevel = false;
	bMemoryOnly = false;
}

URPGAsyncSaveGame* URPGAsyncSaveGame::AsyncSaveActors(UObject* WorldContextObject, int32 Data)
{
	if (URPGSaveSubsystem* EMSObject = URPGSaveSubsystem::Get(WorldContextObject))
	{
		const ESaveGameMode Mode = FAsyncSaveHelpers::GetMode(Data);

		if (EMSObject->IsAsyncSaveOrLoadTaskActive(Mode))
		{
			return nullptr;
		}

		URPGAsyncSaveGame* SaveTask = NewObject<URPGAsyncSaveGame>(GetTransientPackage());
		if (SaveTask)
		{
			SaveTask->RPGSaveSubsystem = EMSObject;
			SaveTask->Data = Data;
			SaveTask->Mode = Mode;
			return SaveTask;
		}	
	}

	return nullptr;
}

void URPGAsyncSaveGame::AutoSaveLevelActors(URPGSaveSubsystem* Subsystem)
{
	URPGAsyncSaveGame* SaveTask = NewObject<URPGAsyncSaveGame>(GetTransientPackage());
	if (SaveTask)
	{
		SaveTask->RPGSaveSubsystem = Subsystem;
		SaveTask->Data = ENUM_TO_FLAG(ESaveTypeFlags::SF_Level);
		SaveTask->Mode = ESaveGameMode::MODE_Level;
		SaveTask->bAutoSaveLevel = true;
		SaveTask->bMemoryOnly = FSettingHelpers::IsMemoryOnlySave();
		SaveTask->RegisterWithGameInstance(Subsystem);
		SaveTask->Activate();
	}
}

void URPGAsyncSaveGame::Activate()
{
	if (RPGSaveSubsystem)
	{
		//Warn and/or cancel when still streaming 
		if (RPGSaveSubsystem->HasStreamingLevels())
		{
			if (CheckLevelStreaming())
			{
				bHasFailed = true;
				FinishSaving();
				return;
			}
		}

		bIsActive = true;

		//For World Partition auto-saving, we only prepare runtime Actors, the preparation process can be skipped for placed Actors in cells
		const EPrepareType PrepareType = bAutoSaveLevel ? EPrepareType::PT_RuntimeOnly : EPrepareType::PT_Default;
		RPGSaveSubsystem->PrepareLoadAndSaveActors(Data, EAsyncCheckType::CT_Save, PrepareType);

		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncSaveGame::StartSaving);
	}
}

void URPGAsyncSaveGame::StartSaving()
{
	bHasFailed = false;

	if (RPGSaveSubsystem)
	{
		const bool bSaveToMemory = bAutoSaveLevel && bMemoryOnly;
		if (!bSaveToMemory)
		{
			//Save current slot
			RPGSaveSubsystem->SaveSlotInfoObject(RPGSaveSubsystem->GetCurrentSaveGameName());
		}

		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncSaveGame::SavePlayer);
	}
}

/**
Player
**/

void URPGAsyncSaveGame::SavePlayer()
{
	bFinishedStep = false;

	if (RPGSaveSubsystem)
	{
		if (FSettingHelpers::IsMultiThreadSaving())
		{
			AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this]()
			{
				InternalSavePlayer();
			});
		}
		else
		{
			InternalSavePlayer();
		}

		TryMoveToNextStep(ENextStepType::SaveLevel);
	}
}

void URPGAsyncSaveGame::InternalSavePlayer()
{
	if (Data & ENUM_TO_FLAG(ESaveTypeFlags::SF_Player))
	{
		if (!RPGSaveSubsystem->SavePlayerActors(RPGSaveSubsystem->GetPlayerController(), RPGSaveSubsystem->PlayerSaveFile()))
		{
			bHasFailed = true;
		}
	}

	bFinishedStep = true;
}

/**
Level
**/

void URPGAsyncSaveGame::SaveLevel()
{
	bFinishedStep = false;

	if (RPGSaveSubsystem)
	{
		if (FSettingHelpers::IsMultiThreadSaving())
		{
			AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [this]()
			{
				InternalSaveLevel();
			});
		}
		else
		{
			InternalSaveLevel();
		}

		TryMoveToNextStep(ENextStepType::FinishSave);
	}
}

void URPGAsyncSaveGame::InternalSaveLevel()
{
	if (Data & ENUM_TO_FLAG(ESaveTypeFlags::SF_Level))
	{
		const bool bPrevHasFailed = bHasFailed;

		if(RPGSaveSubsystem->SaveLevelActors(bMemoryOnly))
		{
			bHasFailed = false;
		}
		else
		{
			bHasFailed = bPrevHasFailed;
		}
	}

	bFinishedStep = true;
}

/**
Finish
**/

void URPGAsyncSaveGame::FinishSaving()
{
	if (RPGSaveSubsystem)
	{
		bIsActive = false;

		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncSaveGame::CompleteSavingTask);
	}
}

void URPGAsyncSaveGame::CompleteSavingTask()
{
	if (bHasFailed)
	{
		OnFailed.Broadcast();
	}
	else
	{
		OnCompleted.Broadcast();
	}

	SetReadyToDestroy();
}

/**
Helper Functions
**/

void URPGAsyncSaveGame::TryMoveToNextStep(ENextStepType Step)
{
	//This is used to delay further execution until multi-thread code finished, but without blocking.

	if (RPGSaveSubsystem)
	{
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([this, Step]()
		{
			if (bFinishedStep)
			{
				if (Step == ENextStepType::FinishSave)
				{
					RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncSaveGame::FinishSaving);
				}
				else
				{
					RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncSaveGame::SaveLevel);
				}
			}
			else
			{
				TryMoveToNextStep(Step);
			}
		});

		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(TimerDelegate);
	}
}

bool URPGAsyncSaveGame::CheckLevelStreaming()
{
	if (!bAutoSaveLevel && (Data & ENUM_TO_FLAG(ESaveTypeFlags::SF_Level)))
	{
		auto LogStreamingWarning = [](const FString& Msg)
		{
			UE_LOG(LogRPGSave, Warning, TEXT("%s"), *Msg);
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, Msg);
		};

		if (RPGSaveSubsystem->AutoSaveLoadWorldPartition())
		{
			//Handle World Partition
			if (!RPGSaveSubsystem->InitWorldPartitionLoadComplete())
			{
				if (!RPGSaveSubsystem->SkipInitialWorldPartitionLoad())
				{
					LogStreamingWarning(TEXT("World Partition is still loading (automatic load). Save operation cancelled to prevent data loss."));
					return true;
				}
				else
				{
					LogStreamingWarning(TEXT("Save occurred during World Partition load (manual load). Data was overwritten(!). Use 'Is Level Streaming Active' to check."));
				}
			}
		}
		else
		{
			//Handle traditional level streaming
			if (RPGSaveSubsystem->IsLevelStreaming())
			{
				LogStreamingWarning(TEXT("Level streaming is still active. Save operation cancelled to prevent data loss."));
				return true;
			}
		}
	}

	return false;
}