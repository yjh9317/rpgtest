// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Async/RPGAsyncWait.h"
#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"

URPGAsyncWait::URPGAsyncWait()
{
	Type = EAsyncCheckType::CT_Both;
	bSaveOrLoadTaskWasActive = false;
	CheckCounter = 0;
}

URPGAsyncWait* URPGAsyncWait::AsyncWaitForOperation(UObject* WorldContextObject, EAsyncCheckType CheckType)
{
	if (URPGSaveSubsystem* Subsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		URPGAsyncWait* WaitTask = NewObject<URPGAsyncWait>(GetTransientPackage());
		WaitTask->RPGSaveSubsystem = Subsystem;
		WaitTask->bSaveOrLoadTaskWasActive = false;
		WaitTask->Type = CheckType;
		return WaitTask;
	}

	return nullptr;
}

void URPGAsyncWait::Activate()
{
	if (RPGSaveSubsystem)
	{
		StartWaitTask();

		const float WaitTime = URPGSaveProjectSetting::Get()->AsyncWaitDelay;
		if (WaitTime > 0.f)
		{
			RPGSaveSubsystem->GetTimerManager().SetTimer(WaitTimerHandle, FTimerDelegate::CreateUObject(this, &URPGAsyncWait::ForceProceed), WaitTime, false);
		}
	}
}

void URPGAsyncWait::StartWaitTask()
{
	if (!RPGSaveSubsystem)
	{
		CompleteWaitTask();
		return;
	}

	//Wait as long as the current async operation is completed.
	if (RPGSaveSubsystem->IsAsyncSaveOrLoadTaskActive(ESaveGameMode::MODE_All, Type, false))
	{
		RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncWait::StartWaitTask);
		bSaveOrLoadTaskWasActive = true;
	}
	else
	{
		//Without save data, we will proceed  
		const bool bSaveFileExists = RPGSaveSubsystem->DoesSaveGameExist(RPGSaveSubsystem->GetCurrentSaveGameName());
		if (!bSaveFileExists)
		{
			//Wait a few frames, due to the async nature of Save and Load operations
			if (++CheckCounter > RPGSave::MinAsyncWaitFrames)
			{
				bSaveOrLoadTaskWasActive = true;
			}
		}

		if (bSaveOrLoadTaskWasActive)
		{
			RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncWait::CompleteWaitTask);
		}
		else
		{
			RPGSaveSubsystem->GetTimerManager().SetTimerForNextTick(this, &URPGAsyncWait::StartWaitTask);
		}
	}	
}

void URPGAsyncWait::ForceProceed()
{
	bSaveOrLoadTaskWasActive = true;
}

void URPGAsyncWait::CompleteWaitTask()
{
	if (RPGSaveSubsystem)
	{
		RPGSaveSubsystem->GetTimerManager().ClearAllTimersForObject(this);
	}

	OnCompleted.Broadcast();
	SetReadyToDestroy();
}
