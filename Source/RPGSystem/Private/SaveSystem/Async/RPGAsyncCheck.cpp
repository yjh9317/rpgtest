// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Async/RPGAsyncCheck.h"

#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"

URPGAsyncCheck::URPGAsyncCheck()
{
	Type = ESaveFileCheckType::CheckForGame;
	bCheckSuccess = false;
	bCheckGameVersion = false;
}

URPGAsyncCheck* URPGAsyncCheck::CheckSaveFiles(UObject* WorldContextObject, ESaveFileCheckType CheckType, FString CustomSaveName, bool bComplexCheck)
{
	if (URPGSaveSubsystem* Subsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		if (!Subsystem->IsAsyncSaveOrLoadTaskActive())
		{
			URPGAsyncCheck* CheckTask = NewObject<URPGAsyncCheck>(GetTransientPackage());
			CheckTask->RPGSaveSubsystem = Subsystem;
			CheckTask->Type = CheckType;
			CheckTask->bCheckGameVersion = bComplexCheck;
			CheckTask->SaveFileName = CustomSaveName;
			return CheckTask;
		}
	}

	return nullptr;
}

void URPGAsyncCheck::Activate()
{
	if (RPGSaveSubsystem)
	{
		UE_LOG(LogRPGSave, Log, TEXT("Current Package Version: %d"), GPackageFileUEVersion.ToValue());
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncCheck::StartCheck);
	}
}

void URPGAsyncCheck::StartCheck()
{
	switch (Type)
	{
	case ESaveFileCheckType::CheckForCustom:
		CheckCustom();
		break;

	case ESaveFileCheckType::CheckForCustomSlot:
		CheckCustomSlot();
		break;

	default:
		bCheckSuccess = RPGSaveSubsystem->CheckSaveGameIntegrity(EDataLoadType::DATA_Object, RPGSaveSubsystem->SlotInfoSaveFile(), bCheckGameVersion);

		if (Type == ESaveFileCheckType::CheckForSlotOnly)
		{
			RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncCheck::CompleteCheck);
		}
		else
		{
			RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncCheck::CheckPlayer);
		}
		break;
	}
}

void URPGAsyncCheck::CheckPlayer()
{
	bCheckSuccess = RPGSaveSubsystem->CheckSaveGameIntegrity(EDataLoadType::DATA_Player, RPGSaveSubsystem->PlayerSaveFile(), bCheckGameVersion);
	RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncCheck::CheckLevel);
}

void URPGAsyncCheck::CheckLevel()
{
	bCheckSuccess = RPGSaveSubsystem->CheckSaveGameIntegrity(EDataLoadType::DATA_Level, RPGSaveSubsystem->ActorSaveFile(), bCheckGameVersion);
	RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncCheck::CompleteCheck);
}

void URPGAsyncCheck::CheckCustom()
{
	bCheckSuccess = RPGSaveSubsystem->CheckSaveGameIntegrity(EDataLoadType::DATA_Object, RPGSaveSubsystem->CustomSaveFile(SaveFileName, FString()), bCheckGameVersion);
	RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncCheck::CompleteCheck);
}

void URPGAsyncCheck::CheckCustomSlot()
{
	bCheckSuccess = RPGSaveSubsystem->CheckSaveGameIntegrity(EDataLoadType::DATA_Object, RPGSaveSubsystem->CustomSaveFile(SaveFileName, RPGSaveSubsystem->GetCurrentSaveGameName()), bCheckGameVersion);
	RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncCheck::CompleteCheck);
}

void URPGAsyncCheck::CompleteCheck()
{
	SetReadyToDestroy();

	if (bCheckSuccess)
	{
		OnCompleted.Broadcast();
		return;
	}

	OnFailed.Broadcast();
}
