// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/RPGSaveFunctionLibrary.h"
#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"
#include "SaveSystem/SaveGame/CustomSaveGame.h"

#include "SaveSystem/Data/RPGSavePaths.h"
#include "Kismet/KismetRenderingLibrary.h"

/**
Save Game Users
**/

void URPGSaveFunctionLibrary::SetCurrentSaveUserName(UObject* WorldContextObject, const FString& UserName)
{
	if (URPGSaveSubsystem* Subsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		Subsystem->SetCurrentSaveUserName(UserName);
	}
}

FString URPGSaveFunctionLibrary::GetCurrentSaveUser(UObject* WorldContextObject)
{
	if (URPGSaveSubsystem* Subsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		return Subsystem->GetCurrentSaveUserName();
	}

	return FString();
}

void URPGSaveFunctionLibrary::DeleteSaveUser(UObject* WorldContextObject, const FString& UserName)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		SaveSubsystem->DeleteAllSaveDataForUser(UserName);
	}
}

TArray<FString> URPGSaveFunctionLibrary::GetAllSaveUsers(UObject* WorldContextObject)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		return SaveSubsystem->GetAllSaveUsers();
	}

	return TArray<FString>();
}

/**
Save Slots
**/

void URPGSaveFunctionLibrary::SetCurrentSaveGameName(UObject* WorldContextObject, const FString & SaveGameName)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		SaveSubsystem->SetCurrentSaveGameName(SaveGameName);
	}
}

TArray<FString> URPGSaveFunctionLibrary::GetSortedSaveSlots(UObject* WorldContextObject)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		return SaveSubsystem->GetSortedSaveSlots();
	}

	return TArray<FString>();
}

UInfoSaveGame* URPGSaveFunctionLibrary::GetSlotInfoSaveGame(UObject* WorldContextObject, FString& SaveGameName)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		SaveGameName = SaveSubsystem->GetCurrentSaveGameName();
		return SaveSubsystem->GetSlotInfoObject(SaveGameName);
	}

	return nullptr;
}

UInfoSaveGame* URPGSaveFunctionLibrary::GetNamedSlotInfo(UObject* WorldContextObject, const FString& SaveGameName)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		return SaveSubsystem->GetSlotInfoObject(SaveGameName);
	}

	return nullptr;
}

bool URPGSaveFunctionLibrary::DoesSaveSlotExist(UObject* WorldContextObject, const FString& SaveGameName, bool bComplete)
{
	if (SaveGameName.IsEmpty())
	{
		return false;
	}

	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		if (bComplete)
		{
			if (SaveSubsystem->DoesFullSaveGameExist(SaveGameName))
			{
				return true;
			}

			return false;
		}

		if (SaveSubsystem->DoesSaveGameExist(SaveGameName))
		{
			return true;
		}
	}

	return false;
}

/**
File System
**/

void URPGSaveFunctionLibrary::DeleteAllSaveDataForSlot(UObject* WorldContextObject, const FString& SaveGameName)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		SaveSubsystem->DeleteAllSaveDataForSlot(SaveGameName);
	}
}

void URPGSaveFunctionLibrary::ClearMultiLevelSave(UObject* WorldContextObject)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		SaveSubsystem->ClearMultiLevelSave();
	}
}

/**
Thumbnail Saving
Simple saving as .png from a 2d scene capture render target source.
**/

UTexture2D* URPGSaveFunctionLibrary::ImportSaveThumbnail(UObject* WorldContextObject, const FString& SaveGameName)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		return SaveSubsystem->ImportSaveThumbnail(SaveGameName);
	}	

	return nullptr;
}

void URPGSaveFunctionLibrary::ExportSaveThumbnail(UObject* WorldContextObject, UTextureRenderTarget2D* TextureRenderTarget, const FString& SaveGameName)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		return SaveSubsystem->ExportSaveThumbnail(TextureRenderTarget, SaveGameName);
	}
}

/**
Other Functions
**/

void URPGSaveFunctionLibrary::SetActorSaveProperties(UObject* WorldContextObject, bool bSkipSave,  bool bPersistent, bool bSkipTransform, ELoadedStateMod LoadedState)
{
	AActor* SaveActor = Cast<AActor>(WorldContextObject);
	if (SaveActor)
	{
		if (bSkipSave)
		{
			SaveActor->Tags.AddUnique(RPGSave::SkipSaveTag);
		}
		else
		{
			SaveActor->Tags.Remove(RPGSave::SkipSaveTag);
		}

		if (bPersistent)
		{
			SaveActor->Tags.AddUnique(RPGSave::PersistentTag);
		}
		else
		{
			SaveActor->Tags.Remove(RPGSave::PersistentTag);
		}

		if (bSkipTransform)
		{
			SaveActor->Tags.AddUnique(RPGSave::SkipTransformTag);
		}
		else
		{
			SaveActor->Tags.Remove(RPGSave::SkipTransformTag);
		}

		if (LoadedState == ELoadedStateMod::Unloaded)
		{
			SaveActor->Tags.Remove(RPGSave::HasLoadedTag);
		}
		else if (LoadedState == ELoadedStateMod::Loaded)
		{
			SaveActor->Tags.Add(RPGSave::HasLoadedTag);
		}
	}
}

bool URPGSaveFunctionLibrary::IsSavingOrLoading(UObject* WorldContextObject)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		if (SaveSubsystem->IsPaused())
		{
			return false;
		}

		return SaveSubsystem->IsAsyncSaveOrLoadTaskActive(ESaveGameMode::MODE_All, EAsyncCheckType::CT_Both, false);
	}

	return false;
}

/**
Custom Objects
**/

bool URPGSaveFunctionLibrary::SaveCustom(UObject* WorldContextObject, UCustomSaveGame* SaveGame)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		if (SaveGame)
		{
			return SaveSubsystem->SaveCustom(SaveGame);
		}
		else
		{
			return SaveSubsystem->SaveAllCustomObjects();
		}
	}

	return false;
}

UCustomSaveGame* URPGSaveFunctionLibrary::GetCustomSave(UObject* WorldContextObject, TSubclassOf<UCustomSaveGame> SaveGameClass, FString SaveSlot, FString FileName)
{
	if (SaveGameClass)
	{
		if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
		{
			return SaveSubsystem->GetCustomSave(SaveGameClass, SaveSlot, FileName);
		}
	}

	return nullptr;
}

void URPGSaveFunctionLibrary::DeleteCustomSave(UObject* WorldContextObject, UCustomSaveGame* SaveGame)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		SaveSubsystem->DeleteCustomSave(SaveGame);
	}
}

void URPGSaveFunctionLibrary::ResetCustomSave(UObject* WorldContextObject, UCustomSaveGame* SaveGame, EResetCustomSaveType Type)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		SaveSubsystem->ResetCustomSave(SaveGame, Type);
	}
}

bool URPGSaveFunctionLibrary::DoesCustomSaveFileExist(UObject* WorldContextObject, FString SaveSlot, FString FileName)
{
	if (FileName.IsEmpty())
	{
		return false;
	}

	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		if (SaveSubsystem->DoesCustomSaveExist(SaveSlot, FileName))
		{
			return true;
		}
	}

	return false;
}


/**
World Partition
**/

bool URPGSaveFunctionLibrary::IsWorldPartition(UObject* WorldContextObject)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		return SaveSubsystem->AutoSaveLoadWorldPartition();
	}

	return false;
}

void URPGSaveFunctionLibrary::ClearWorldPartition(UObject* WorldContextObject)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		SaveSubsystem->ClearWorldPartition();
	}
}

/**
Raw Object Data
**/

bool URPGSaveFunctionLibrary::SaveRawObject(AActor* WorldContextActor, FRawObjectSaveData Data)
{
	if (Data.IsValidData() && IsValid(WorldContextActor))
	{
		if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextActor))
		{
			return SaveSubsystem->SaveRawObject(WorldContextActor, Data);
		}
	}

	return false;
}

UObject* URPGSaveFunctionLibrary::LoadRawObject(AActor* WorldContextActor, FRawObjectSaveData Data)
{
	if (Data.IsValidData() && IsValid(WorldContextActor))
	{
		if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextActor))
		{
			return SaveSubsystem->LoadRawObject(WorldContextActor, Data);
		}
	}

	return nullptr;
}

/**
Streaming
**/

bool URPGSaveFunctionLibrary::IsLevelStreamingActive(UObject* WorldContextObject)
{
	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		if (IsWorldPartition(WorldContextObject))
		{
			return !SaveSubsystem->InitWorldPartitionLoadComplete();
		}

		return SaveSubsystem->IsLevelStreaming();
	}

	return false;
}

/**
Custom Player
**/

bool URPGSaveFunctionLibrary::SavePlayerActorsCustom(AController* Controller, const FString& FileName)
{
	if (FileName.IsEmpty())
	{
		return false;
	}

	if (!Controller)
	{
		return false;
	}

	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(Controller))
	{
		return SaveSubsystem->SavePlayerActorsCustom(Controller, FileName);
	}

	return false;
}

bool URPGSaveFunctionLibrary::LoadPlayerActorsCustom(AController* Controller, const FString& FileName)
{
	if (FileName.IsEmpty())
	{
		return false;
	}

	if (!Controller)
	{
		return false;
	}

	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(Controller))
	{
		return SaveSubsystem->LoadPlayerActorsCustom(Controller, FileName);
	}

	return false;
}

bool URPGSaveFunctionLibrary::DeleteCustomPlayerFile(UObject* WorldContextObject, const FString& FileName)
{
	if (FileName.IsEmpty())
	{
		return false;
	}

	if (URPGSaveSubsystem* SaveSubsystem = URPGSaveSubsystem::Get(WorldContextObject))
	{
		return SaveSubsystem->DeleteCustomPlayerFile(FileName);
	}

	return false;
}

FString URPGSaveFunctionLibrary::GetBackupName(UObject* WorldContextObject, const FString& BaseName)
{
	return FSavePaths::GetBackupFileName(BaseName);
}



