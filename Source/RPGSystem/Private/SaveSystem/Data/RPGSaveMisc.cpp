// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Data/RPGSaveMisc.h"


#include "SaveSystem/Data/RPGSaveMisc.h"
#include "../ProjectSettings/RPGSaveProjectSetting.h"
#include "SaveSystem/Async/RPGAsyncLoadGame.h"
#include "SaveSystem/Async/RPGAsyncSaveGame.h"
#include "SaveSystem/Async/RPGAsyncStream.h"
#include "Engine/World.h"
#include "UObject/UObjectIterator.h"

/**
FSettingHelpers
**/

bool FSettingHelpers::IsNormalMultiLevelSave()
{
	return URPGSaveProjectSetting::Get()->MultiLevelSaving == EMultiLevelSaveMethod::ML_Normal;
}

bool FSettingHelpers::IsStreamMultiLevelSave()
{
	return URPGSaveProjectSetting::Get()->MultiLevelSaving == EMultiLevelSaveMethod::ML_Stream;
}

bool FSettingHelpers::IsFullMultiLevelSave()
{
	return URPGSaveProjectSetting::Get()->MultiLevelSaving == EMultiLevelSaveMethod::ML_Full;
}

bool FSettingHelpers::IsStackBasedMultiLevelSave()
{
	return IsFullMultiLevelSave() || IsNormalMultiLevelSave();
}

bool FSettingHelpers::IsContainingStreamMultiLevelSave()
{
	return IsFullMultiLevelSave() || IsStreamMultiLevelSave();
}

bool FSettingHelpers::IsDynamicLevelStreaming()
{
	return IsContainingStreamMultiLevelSave() && URPGSaveProjectSetting::Get()->bDynamicLevelStreaming;
}

bool FSettingHelpers::IsMemoryOnlySave()
{
	return URPGSaveProjectSetting::Get()->WorldPartitionSaving == EWorldPartitionMethod::MemoryOnly;
}

bool FSettingHelpers::IsConsoleFileSystem()
{
	return URPGSaveProjectSetting::Get()->FileSaveMethod == EFileSaveMethod::FM_Console;
}

bool FSettingHelpers::IsMultiThreadSaving()
{
	return URPGSaveProjectSetting::Get()->bMultiThreadSaving && FPlatformProcess::SupportsMultithreading();
}

bool FSettingHelpers::IsMultiThreadLoading()
{
	return URPGSaveProjectSetting::Get()->LoadMethod == ELoadMethod::LM_Thread && FPlatformProcess::SupportsMultithreading();
}

bool FSettingHelpers::IsDeferredLoading()
{
	return URPGSaveProjectSetting::Get()->LoadMethod == ELoadMethod::LM_Deferred;
}

uint32 FSettingHelpers::GetLoadBatchSize()
{
	return FMath::Max(1, URPGSaveProjectSetting::Get()->DeferredLoadStackSize);
}

/**
Async Node Helpers
**/

template<class T>
bool FAsyncSaveHelpers::CheckLoadIterator(const T& It, const ESaveGameMode Mode, const bool bLog, const FString& DebugString)
{
	if (It && It->IsActive() && (It->Mode == Mode || Mode == ESaveGameMode::MODE_All))
	{
		if (bLog)
		{
			UE_LOG(LogRPGSave, Warning, TEXT("%s is active while trying to save or load."), *DebugString);
		}

		return true;
	}

	return false;
}

bool FAsyncSaveHelpers::IsAsyncSaveOrLoadTaskActive(const UWorld* InWorld, const ESaveGameMode Mode, const EAsyncCheckType CheckType, const bool bLog)
{
	//This will prevent the functions from being executed at all during pause.
	if (InWorld->IsPaused())
	{
		if (bLog)
		{
			UE_LOG(LogRPGSave, Warning, TEXT("Async save or load called during pause. Operation was canceled."));
		}

		return true;
	}

	if (CheckType == EAsyncCheckType::CT_Both || CheckType == EAsyncCheckType::CT_Load)
	{
		for (TObjectIterator<URPGAsyncLoadGame> It; It; ++It)
		{
			if (CheckLoadIterator(It, Mode, bLog, "Load Game Actors"))
			{
				return true;
			}
		}

		for (TObjectIterator<URPGAsyncStream> It; It; ++It)
		{
			if (CheckLoadIterator(It, Mode, bLog, "Load Stream Level Actors"))
			{
				return true;
			}
		}
	}

	if (CheckType == EAsyncCheckType::CT_Both || CheckType == EAsyncCheckType::CT_Save)
	{
		for (TObjectIterator<URPGAsyncSaveGame> It; It; ++It)
		{
			if (CheckLoadIterator(It, Mode, bLog, "Save Game Actors"))
			{
				return true;
			}
		}
	}

	return false;
}

bool FAsyncSaveHelpers::IsStreamAutoLoadActive(const ULevel* InLevel)
{
	for (TObjectIterator<URPGAsyncStream> It; It; ++It)
	{
		if (It && It->IsActive() && It->StreamingLevel == InLevel)
		{
			return true;
		}
	}

	return false;
}

void FAsyncSaveHelpers::DestroyStreamAutoLoadTask(const ULevel* InLevel)
{
	for (TObjectIterator<URPGAsyncStream> It; It; ++It)
	{
		if (It && It->IsActive() && It->StreamingLevel == InLevel)
		{
			It->ForceDestroy();
		}
	}
}

void FAsyncSaveHelpers::DestroyAsyncLoadLevelTask()
{
	for (TObjectIterator<URPGAsyncLoadGame> It; It; ++It)
	{
		if (It && It->IsActive())
		{
			It->ForceDestroy();
		}
	}
}

ESaveGameMode FAsyncSaveHelpers::GetMode(const int32 Data)
{
	if (Data & ENUM_TO_FLAG(ESaveTypeFlags::SF_Player))
	{
		if (Data & ENUM_TO_FLAG(ESaveTypeFlags::SF_Level))
		{
			return ESaveGameMode::MODE_All;
		}
		else
		{
			return ESaveGameMode::MODE_Player;
		}
	}

	return ESaveGameMode::MODE_Level;
}
