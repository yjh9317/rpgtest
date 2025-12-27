// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Subsystem/RPGSaveSubsystemBase.h"
#include "SaveSystem/Data/RPGSavePaths.h"
#include "../ProjectSettings/RPGSaveProjectSetting.h"
#include "SaveSystem/SaveGame/CustomSaveGame.h"
#include "SaveSystem/SaveGame/InfoSaveGame.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "ImageUtils.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Serialization/ArchiveLoadCompressedProxy.h"
#include "GameFramework/SaveGame.h"
#include "SaveGameSystem.h"
#include "Misc/Paths.h"
#include "UObject/Class.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "PlatformFeatures.h"

/**
Init
**/

URPGSaveSubsystemBase::URPGSaveSubsystemBase()
{
	bLoadFromMemory = false;
	LoadedPackageVersion = GPackageFileUEVersion;
	LastSlotSaveTime = 0.f;
}

void URPGSaveSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogRPGSave, Log, TEXT("RPG Save Initialized"));

	const FString VersionNum = VERSION_STRINGIFY(EMS_VERSION_NUMBER);
	UE_LOG(LogRPGSave, Log, TEXT("RPG Save Version: %s"), *VersionNum);

	UE_LOG(LogRPGSave, Log, TEXT("Current Save Game Slot: %s"), *GetCurrentSaveGameName());
}

UWorld* URPGSaveSubsystemBase::GetWorld() const
{
	return GetGameInstance()->GetWorld();
}

const bool URPGSaveSubsystemBase::HasValidWorld() const
{
	const TWeakObjectPtr<UWorld> TheWorld = MakeWeakObjectPtr(GetWorld());
	return TheWorld.IsValid();
}

/**
Save Object Creation
**/

template <class TSaveGame>
TSaveGame* URPGSaveSubsystemBase::CreateNewSaveObject(const FString& FullSavePath, const FSoftClassPath& InClassName)
{
	if (FullSavePath.IsEmpty())
	{
		UE_LOG(LogRPGSave, Error, TEXT("Save path is empty"));
		return nullptr;
	}

	//Try to load the class from the provided class name
	const FSoftClassPath LocalClassName = InClassName;
	TSubclassOf<TSaveGame> Class = LocalClassName.TryLoadClass<TSaveGame>();

	if (!Class)
	{
		UE_LOG(LogRPGSave, Error, TEXT("Invalid Save Game Object Class: %s"), *LocalClassName.ToString());
		return nullptr;
	}

	//Load save game object 
	USaveGame* SaveGame = NewObject<USaveGame>(this, Class);
	if (!SaveGame)
	{
		UE_LOG(LogRPGSave, Error, TEXT("Failed to load Save Game Object: %s"), *FullSavePath);
		return nullptr;
	}

	TSaveGame* SaveGameObject = Cast<TSaveGame>(SaveGame);
	if (!SaveGameObject)
	{
		UE_LOG(LogRPGSave, Error, TEXT("Invalid Save Game Object: %s"), *FullSavePath);
		return nullptr;
	}

	//Check integrity and only unpack when loading(when its actually there), this is the safe way for the unified getter functions
	const EFileValidity ValidCheck = IsSaveFileValid(FullSavePath, false);
	if (ValidCheck == EFileValidity::FILE_VALID)
	{
		LoadBinaryArchive(EDataLoadType::DATA_Object, FullSavePath, SaveGameObject);
	}

	return SaveGameObject;
}

/**
Custom Save Objects
**/

UCustomSaveGame* URPGSaveSubsystemBase::GetCustomSave(const TSubclassOf<UCustomSaveGame>& SaveGameClass, const FString& InSlotName, const FString& InFileName)
{
	if (!SaveGameClass || SaveGameClass->HasAnyClassFlags(CLASS_Abstract))
	{
		if (SaveGameClass)
		{
			UE_LOG(LogRPGSave, Warning, TEXT("Custom Save Class '%s' is abstract and cannot be used."), *SaveGameClass->GetName());
		}

		return nullptr;
	}

	const UCustomSaveGame* CustomClass = Cast<UCustomSaveGame>(SaveGameClass->GetDefaultObject());
	if (!CustomClass)
	{
		return nullptr;
	}

	//Allow for custom file names
	const FString CustomSaveName = [InFileName, CustomClass]() -> FString
	{
		if (InFileName.IsEmpty())
		{
			return CustomClass->SaveGameName.IsEmpty() ? CustomClass->GetName() : CustomClass->SaveGameName;
		}
		else
		{
			return InFileName;
		}
	}();

	const bool bUseSlot = CustomClass->bUseSaveSlot;
	const bool bCustomSlotName = bUseSlot && !InSlotName.IsEmpty();

	const FString ActualSlot = bCustomSlotName ? InSlotName : GetCurrentSaveGameName();
	const FString SlotName = bUseSlot ? ActualSlot : FString();
	const FString CachedRefName = CustomSaveName + SlotName;

	if (UCustomSaveGame* CachedObject = CachedCustomSaves.FindRef(CachedRefName))
	{
		if (bUseSlot) CachedObject->SlotName = SlotName;
		return CachedObject;
	}

	const FString SaveFile = CustomSaveFile(CustomSaveName, SlotName);
	const FSoftClassPath SaveClass = CustomClass->GetClass();

	if (UCustomSaveGame* NewObject = CreateNewSaveObject<UCustomSaveGame>(SaveFile, SaveClass))
	{
		NewObject->SaveGameName = CustomSaveName;
		NewObject->SlotName = SlotName;
		CachedCustomSaves.Add(CachedRefName, NewObject);
		return NewObject;
	}

	return nullptr;
}

bool URPGSaveSubsystemBase::SaveCustom(UCustomSaveGame* SaveGame)
{
	if (!IsValid(SaveGame))
	{
		return false;
	}

	const bool bUseSlot = SaveGame->bUseSaveSlot;
	const FString SlotName = bUseSlot ? SaveGame->SlotName : FString();
	const FString CustomSaveName = SaveGame->SaveGameName;

	if (SaveObject(*CustomSaveFile(CustomSaveName, SlotName), SaveGame))
	{
		if (bUseSlot)
		{
			SaveSlotInfoObject(SlotName);
			UE_LOG(LogRPGSave, Log, TEXT("Custom Save Game saved: %s for Slot: %s"), *CustomSaveName, *SlotName);
		}
		else
		{
			UE_LOG(LogRPGSave, Log, TEXT("Custom Save Game saved: %s"), *CustomSaveName);
		}

		return true;
	}

	UE_LOG(LogRPGSave, Warning, TEXT("Failed to save Custom Save Game: %s"), *CustomSaveName);

	return false;
}

bool URPGSaveSubsystemBase::SaveAllCustomObjects()
{
	bool bSuccess = false;

	for (auto& CustomObjectPair : CachedCustomSaves)
	{
		UCustomSaveGame* SaveGame = CustomObjectPair.Value;
		if (!SaveGame)
		{
			continue;
		}

		//It can only auto-save objects in the current slot, but you can still manually save in other slots
		if (SaveGame->bUseSaveSlot)
		{
			if (SaveGame->SlotName != GetCurrentSaveGameName())
			{
				continue;
			}
		}

		bSuccess |= SaveCustom(SaveGame);
	}

	return bSuccess;
}


FCustomSaveInfo URPGSaveSubsystemBase::GetCustomSaveInfo(const UCustomSaveGame* SaveGame) const 
{
	FCustomSaveInfo Info;  

	if (!IsValid(SaveGame))
	{
		Info.bValid = false; 
		return Info;
	}

	Info.bValid = true;
	Info.bUseSlot = SaveGame->bUseSaveSlot;
	Info.CustomSaveName = SaveGame->SaveGameName;
	Info.SlotName = Info.bUseSlot ? SaveGame->SlotName : FString();
	Info.SaveFile = CustomSaveFile(Info.CustomSaveName, Info.SlotName);
	Info.CachedRefName = Info.bUseSlot ? Info.CustomSaveName + Info.SlotName : Info.CustomSaveName;

	return Info;
}

void URPGSaveSubsystemBase::DeleteCustomSave(UCustomSaveGame* SaveGame)
{
	const FCustomSaveInfo Info = GetCustomSaveInfo(SaveGame);
	if (!Info.bValid)
	{
		return;
	}

	if (DoesFileExist(Info.SaveFile))
	{
		ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
		if (SaveSystem->DeleteGame(false, *Info.SaveFile, PlayerIndex))
		{
			ClearCustomSaveByRef(Info.CachedRefName);
			UE_LOG(LogRPGSave, Log, TEXT("Custom Save Game deleted: %s"), *Info.CustomSaveName);
		}
	}
}

void URPGSaveSubsystemBase::ResetCustomSave(UCustomSaveGame* SaveGame, const EResetCustomSaveType Type)
{
	const FCustomSaveInfo Info = GetCustomSaveInfo(SaveGame);
	if (!Info.bValid)
	{
		return;
	}

	switch (Type)
	{
	case EResetCustomSaveType::Full:
		ClearCustomSaveByRef(Info.CachedRefName);
		FSaveHelpers::ResetObjectToDefaults(SaveGame, false);
		break;

	case EResetCustomSaveType::ResetDefault:
		FSaveHelpers::ResetObjectToDefaults(SaveGame, false);
		break;

	case EResetCustomSaveType::ClearMemory:
		ClearCustomSaveByRef(Info.CachedRefName);
		break;
	}

	UE_LOG(LogRPGSave, Log, TEXT("Custom Save Game reset: %s with option %d"), *Info.CustomSaveName, uint8(Type));
}

void URPGSaveSubsystemBase::ClearCustomSaveByRef(const FString& CachedRefName)
{
	if (CachedRefName.IsEmpty())
	{
		return;
	}

	CachedCustomSaves.Remove(CachedRefName);
}

void URPGSaveSubsystemBase::ClearCachedCustomSaves()
{
	CachedCustomSaves.Empty();
}

void URPGSaveSubsystemBase::ClearCustomSavesDesktop(const FString& SaveGameName)
{
	//Only clears the cache
	TArray<FString> TempArray;
	ClearCustomSavesConsole(SaveGameName, false, TempArray);
}

void URPGSaveSubsystemBase::ClearCustomSavesConsole(const FString& SaveGameName, const bool bAddFiles, TArray<FString>& OutFiles)
{
	TMap<FString, TObjectPtr<UCustomSaveGame>> TempCustomSaves = CachedCustomSaves;

	for (auto It = TempCustomSaves.CreateIterator(); It; ++It)
	{
		if (It->Key.Contains(SaveGameName))
		{
			const TObjectPtr<UCustomSaveGame> CustomSaveGame = It->Value;
			if (CustomSaveGame)
			{
				//@TODO cannot delete if the slot is not cached, non-cached custom saves must be removed manually on console.
				//We cannot iterate the files, so we need another solution.
				if (bAddFiles)
				{
					//Used to delete custom saves for a slot with the console file system
					const FString CustomName = SaveGameName + RPGSave::UnderscoreInt + CustomSaveGame->SaveGameName;
					OutFiles.Add(CustomName);
				}

				CachedCustomSaves.Remove(It->Key);
			}
		}
	}
}

bool URPGSaveSubsystemBase::DoesCustomSaveExist(const FString& InSlotName, const FString& InFileName) const
{
	return DoesFileExist(CustomSaveFile(InFileName, InSlotName));
}

/**
Save Slots
**/

FString URPGSaveSubsystemBase::GetCurrentSaveGameName() const
{
	const FString DefaultName = URPGSaveProjectSetting::Get()->DefaultSaveGameName;

	if (CurrentSaveGameName.IsEmpty())
	{
		return DefaultName;
	}

	return CurrentSaveGameName;
}

void URPGSaveSubsystemBase::SetCurrentSaveGameName(const FString& SaveGameName)
{
	if (CurrentSaveGameName != SaveGameName)
	{
		//When switching slots, we want to always load from disk
		ClearLoadFromMemory();

		CurrentSaveGameName = FSavePaths::ValidateSaveName(SaveGameName);
		SaveConfig();

		UE_LOG(LogRPGSave, Log, TEXT("New Current Save Game Slot is: %s"), *CurrentSaveGameName);
	}
}

TArray<FString> URPGSaveSubsystemBase::GetSortedSaveSlots() const
{
	TArray<FString> SaveGameNames;

	if (IsConsoleFileSystem())
	{
		//Files
		SaveGameNames = FSavePaths::GetConsoleSlotFiles(GetAllSaveGames());
	}
	else
	{
		//Folders
		IFileManager::Get().FindFiles(SaveGameNames, *FPaths::Combine(BaseSaveDir(), TEXT("*")), false, true);
	}

	//Return list sorted by time
	TArray<FSaveSlotInfo> SaveSlots = GetSlotInfos(SaveGameNames);
	return FSavePaths::GetSortedSaveSlots(SaveSlots);
}

TArray<FString> URPGSaveSubsystemBase::GetAllSaveGames() const
{
	TArray<FString> SaveGameNames;

	//Might not be available on all platforms
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	SaveSystem->GetSaveGameNames(SaveGameNames, PlayerIndex);

	return SaveGameNames;
}

TArray<FSaveSlotInfo> URPGSaveSubsystemBase::GetSlotInfos(const TArray<FString>& SaveGameNames) const
{
	//Fill with proper info
	TArray<FSaveSlotInfo> SaveSlots;
	for (const FString& SlotName : SaveGameNames)
	{
		FSaveSlotInfo SlotInfo;
		const FString SlotPath = SlotFilePath(SlotName);
		SlotInfo.TimeStamp = IFileManager::Get().GetTimeStamp(*SlotPath);
		SlotInfo.Name = SlotName;

		SaveSlots.Add(SlotInfo);
	}

	return SaveSlots;
}

UInfoSaveGame* URPGSaveSubsystemBase::GetSlotInfoObject(const FString& SaveGameName)
{
	const FString ValidSaveName = FSavePaths::ValidateSaveName(SaveGameName);

	//Try to get a cached slot or create a new one
	UInfoSaveGame* Slot = CachedSaveSlots.FindRef(ValidSaveName);
	if (!Slot)
	{
		Slot = MakeSlotInfoObject(ValidSaveName);
		if (!Slot)
		{
			return nullptr;
		}

		CachedSaveSlots.Add(ValidSaveName, Slot);
	}

	//Keep level list when switching slots
	if (IsStackBasedMultiLevelSave())
	{
		Slot->KeepMultiLevelSave(MultiSaveLevels);
	}

	return Slot;
}

UInfoSaveGame* URPGSaveSubsystemBase::MakeSlotInfoObject(const FString& SaveGameName)
{
	const FString SaveSlotFile = SlotInfoSaveFile(SaveGameName);

	//Workaround for when class cannot be loaded from DefaultEngine.ini in cooked builds
	FSoftClassPath SaveSlotClass = URPGSaveProjectSetting::Get()->SlotInfoSaveGameClass;
	if (!SaveSlotClass.IsValid())
	{
		SaveSlotClass = RPGSave::DefaultSlotClass;
		UE_LOG(LogRPGSave, Warning, TEXT("Slot Info Class could not be read from settings, using default."));
	}

	return CreateNewSaveObject<UInfoSaveGame>(SaveSlotFile, SaveSlotClass);
}

void URPGSaveSubsystemBase::SaveSlotInfoObject(const FString& SaveGameName)
{
	if (SaveGameName.IsEmpty())
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Trying to save slot with an empty name"));
		return;
	}

	//Prevent redundant operations if slot exists and delay hasn't passed, using platform time
	if (FPlatformTime::Seconds() - LastSlotSaveTime < RPGSave::ShortDelay)
	{
		if (DoesSaveGameExist(SaveGameName))
		{
			return;
		}
	}

	if (!VerifyOrCreateDirectory(SaveGameName))
	{
		UE_LOG(LogRPGSave, Error, TEXT("Failed to verify or create directory for: %s"), *SaveGameName);
		return;
	}

	//Retrieve or create save object
	UInfoSaveGame* SaveGame = GetSlotInfoObject(SaveGameName);

	if (!SaveGame)
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Invalid Save Object for: %s"), *SaveGameName);
		return;
	}

	//Update save info
	SaveGame->SlotInfo.Name = SaveGameName;
	SaveGame->SlotInfo.TimeStamp = FDateTime::Now();

	const FName LevelName = GetLevelName();
	SaveGame->SlotInfo.Level = LevelName;

	//Maintain list of all saved level names
	if (IsStackBasedMultiLevelSave())
	{
		SaveGame->UpdateMultiLevelSave(LevelName, MultiSaveLevels);
	}

	TArray<FString> PlayerNames;
	FSaveHelpers::ExtractPlayerNames(GetWorld(), PlayerNames);
	SaveGame->SlotInfo.Players = PlayerNames;

	//Update cache, otherwise it will never overwrite the data during a session
	CachedSaveSlots.Add(SaveGameName, SaveGame);

	//Save object
	if (SaveObject(*SlotInfoSaveFile(SaveGameName), SaveGame))
	{
		UE_LOG(LogRPGSave, Log, TEXT("Slot Info saved: %s"), *SaveGameName);
	}
	else
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Slot Info could not be saved: %s"), *SaveGameName);
	}

	LastSlotSaveTime = FPlatformTime::Seconds();
}

void URPGSaveSubsystemBase::ClearMultiSaveLevels()
{
	const FString SaveGameName = GetCurrentSaveGameName();
	UInfoSaveGame* SaveGame = GetSlotInfoObject(SaveGameName);
	if (SaveGame)
	{
		MultiSaveLevels.Empty();
		SaveGame->SlotInfo.Levels.Empty();
	}
}

bool URPGSaveSubsystemBase::DoesSaveGameExist(const FString& SaveGameName) const
{
	return DoesFileExist(SlotInfoSaveFile(SaveGameName));
}

bool URPGSaveSubsystemBase::DoesFullSaveGameExist(const FString& SaveGameName) const
{
	const bool bHasSlotFile = DoesFileExist(SlotInfoSaveFile(SaveGameName));
	const bool bHasPlayerFile = DoesFileExist(PlayerSaveFile(SaveGameName));
	const bool bHasLevelFile = DoesFileExist(ActorSaveFile(SaveGameName));

	return bHasSlotFile && bHasPlayerFile && bHasLevelFile;
}

void URPGSaveSubsystemBase::DeleteAllSaveDataForSlot(const FString& SaveGameName)
{
	bool bSuccess = false;

	//Console uses files and not folders
	if (IsConsoleFileSystem())
	{
		//Hardcoded default files, since we cannot iterate through them
		TArray<FString> AllFiles = FSavePaths::GetDefaultSaveFiles(SaveGameName);

		//Parse the custom save objects and clear their cache
		ClearCustomSavesConsole(SaveGameName, true, AllFiles);

		//Use native delete 
		ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
		for (const FString& FileName : AllFiles)
		{
			if (*FileName)
			{
				SaveSystem->DeleteGame(false, *FileName, PlayerIndex);
				bSuccess = true;
			}
		}

		if (bSuccess)
		{
			UE_LOG(LogRPGSave, Log, TEXT("Save Game Data removed for: %s"), *SaveGameName);
		}
	}
	else
	{
		const FString SaveFile = FPaths::Combine(BaseSaveDir(), SaveGameName);

		bSuccess = IFileManager::Get().DeleteDirectory(*SaveFile, true, true);
		if (bSuccess)
		{
			UE_LOG(LogRPGSave, Log, TEXT("Save Game Data removed for: %s"), *SaveGameName);
		}

		//Delete the cached custom save objects
		ClearCustomSavesDesktop(SaveGameName);
	}

	//Remove Cached Slot
	CachedSaveSlots.Remove(SaveGameName);
}

void URPGSaveSubsystemBase::ClearCachedSlots()
{
	CachedSaveSlots.Empty();
}

/**
Save Users
**/

FString URPGSaveSubsystemBase::GetCurrentSaveUserName() const
{
	return CurrentSaveUserName;
}

bool URPGSaveSubsystemBase::HasSaveUserName() const
{
	return !GetCurrentSaveUserName().IsEmpty();
}

void URPGSaveSubsystemBase::SetCurrentSaveUserName(const FString& UserName)
{
	const bool bIsConsole = IsConsoleFileSystem();
	const bool bRequestedEmpty = UserName.IsEmpty();

	//Both console and empty input result in clearing
	if (bIsConsole || bRequestedEmpty)
	{
		if (!CurrentSaveUserName.IsEmpty())
		{
			ClearUserData();
			CurrentSaveUserName.Reset();
			SaveConfig();
		}

		UE_LOG(LogRPGSave, Log, TEXT("Save User Name Cleared"));

		if (bIsConsole)
		{
			UE_LOG(LogRPGSave, Warning, TEXT("Save Users are not supported when using the console file system."));
		}

		return;
	}

	const FString NewName = FSavePaths::ValidateSaveName(UserName);

	if (CurrentSaveUserName != NewName)
	{
		ClearUserData();
		CurrentSaveUserName = NewName;
		SaveConfig();

		UE_LOG(LogRPGSave, Log, TEXT("New Current Save User Name is: %s"), *NewName);
	}
}

void URPGSaveSubsystemBase::DeleteAllSaveDataForUser(const FString& UserName)
{
	ClearUserData();

	const FString UserSaveFile = SaveUserDir() + UserName;
	bool bSuccess = false;

	//Try removing folder	
	bSuccess = IFileManager::Get().DeleteDirectory(*UserSaveFile, true, true);
	if (bSuccess)
	{
		UE_LOG(LogRPGSave, Log, TEXT("Save Game User Data removed for: %s"), *UserName);
	}
}

TArray<FString> URPGSaveSubsystemBase::GetAllSaveUsers() const
{
	TArray<FString> SaveUserNames;
	IFileManager::Get().FindFiles(SaveUserNames, *FPaths::Combine(SaveUserDir(), TEXT("*")), false, true);

	return SaveUserNames;
}

void URPGSaveSubsystemBase::ClearUserData()
{
	ClearCachedSlots();
	ClearCachedCustomSaves();
}

/**
Save and Load Archive Functions
**/

bool URPGSaveSubsystemBase::SaveObject(const FString& FullSavePath, UObject* SaveGameObject) const
{
	bool bSuccess = false;

	if (SaveGameObject)
	{
		TArray<uint8> Data;

		FMemoryWriter MemoryWriter(Data, true);
		FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
		SaveGameObject->Serialize(Ar);

		FBufferArchive Archive;
		WritePackageInfo(Archive);
		Archive << Data;

		//Check archive errors directly
		if (!FSaveHelpers::HasSaveArchiveError(Archive, ESaveErrorType::ER_Object))
		{
			bSuccess = SaveBinaryArchive(Archive, *FullSavePath);
		}
	}

	return bSuccess;
}

bool URPGSaveSubsystemBase::SaveBinaryData(const TArray<uint8>& SavedData, const FString& FullSavePath) const
{
	//Auto backup 
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	if (URPGSaveProjectSetting::Get()->bAutoBackup && SaveSystem->DoesSaveGameExist(*FullSavePath, PlayerIndex))
	{
		PerformAutoBackup(FullSavePath);
	}

	//Save new data
	const bool bSaveSuccess = SaveSystem->SaveGame(false, *FullSavePath, PlayerIndex, SavedData);
	if (!bSaveSuccess)
	{
		UE_LOG(LogRPGSave, Error, TEXT("Failed to save game data to: %s"), *FullSavePath);
	}

	return bSaveSuccess;
}

void URPGSaveSubsystemBase::PerformAutoBackup(const FString& FullSavePath) const
{
	const EFileValidity ValidCheck = IsSaveFileValid(FullSavePath);
	if (ValidCheck == EFileValidity::FILE_INVALID)
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Auto backup skipped due to invalid save file: %s"), *FullSavePath);
		return;
	}
	else if (ValidCheck == EFileValidity::FILE_MISSING)
	{
		return;
	}

	//Load existing save data
	TArray<uint8> BinaryData;
	if (!LoadBinaryData(FullSavePath, BinaryData))
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Auto backup failed, unable to load existing save: %s"), *FullSavePath);
		return;
	}

	const FString BackupPath = FSavePaths::GetBackupSavePath(FullSavePath);
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	if (!SaveSystem->SaveGame(false, *BackupPath, PlayerIndex, BinaryData))
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Auto backup failed to save: %s"), *BackupPath);
	}
}

bool URPGSaveSubsystemBase::SaveBinaryArchive(FBufferArchive& BinaryData, const FString& FullSavePath) const
{

#if RPG_PLATFORM_DESKTOP
	FSavePaths::CheckForReadOnly(FullSavePath);
#endif

	bool bSuccess = false;
	const bool bNoCompression = IsConsoleFileSystem();

	WriteGameVersionInfo(BinaryData);

	if (bNoCompression)
	{
		bSuccess = SaveBinaryData(BinaryData, FullSavePath);
	}
	else
	{
		//Compress and save
		TArray<uint8> CompressedData;
		FArchiveSaveCompressedProxy Compressor(CompressedData, NAME_Oodle);

		if (Compressor.GetError())
		{
			UE_LOG(LogRPGSave, Error, TEXT("Cannot save, compressor error: %s"), *FullSavePath);
			return false;
		}

		Compressor << BinaryData;
		Compressor.Flush(); //Finalizes compression

		if (CompressedData.Num() == 0)
		{
			UE_LOG(LogRPGSave, Error, TEXT("Compression failed: Data is empty after compression: %s"), *FullSavePath);
			return false;
		}

		bSuccess = SaveBinaryData(CompressedData, FullSavePath);
		if (!bSuccess)
		{
			UE_LOG(LogRPGSave, Error, TEXT("Failed to save compressed data: %s"), *FullSavePath);
			return false;
		}

		Compressor.Close(); //Ensures compression finalization
	}

	BinaryData.Empty(); //Clears the buffer

	return bSuccess;
}

bool URPGSaveSubsystemBase::LoadBinaryData(const FString& FullSavePath, TArray<uint8>& OutBinaryData) const
{
	if (!DoesFileExist(FullSavePath))
	{
		OutBinaryData.Empty();
		return false;
	}

	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	if (!SaveSystem->LoadGame(false, *FullSavePath, PlayerIndex, OutBinaryData))
	{
		UE_LOG(LogRPGSave, Warning, TEXT("%s could not be loaded"), *FullSavePath);
		OutBinaryData.Empty();
		return false;
	}

	if (RPGSave::ArrayEmpty(OutBinaryData))
	{
		UE_LOG(LogRPGSave, Warning, TEXT("No binary data found for %s"), *FullSavePath);
		OutBinaryData.Empty();
		return false;
	}

	return true;
}

bool URPGSaveSubsystemBase::LoadBinaryArchive(const EDataLoadType LoadType, const FString& FullSavePath, UObject* Object, const bool bReadVersion)
{
	TArray<uint8> BinaryData;
	if (!LoadBinaryData(FullSavePath, BinaryData))
	{
		return false;
	}

	bool bSuccess = false;
	const bool bNoCompression = IsConsoleFileSystem();

	//Eliminates duplication between the compressed and uncompressed code paths.
	const auto ReadFromArchive = [this, LoadType, Object, bReadVersion](const TArray<uint8>& Archive) -> bool
	{
		FMemoryReader MemoryReader(Archive, true);
		ReadPackageInfo(MemoryReader, true);

		const bool bSuccess = UnpackBinaryArchive(LoadType, MemoryReader, Object);

		if (bReadVersion)
		{
			ReadGameVersionInfo(MemoryReader);
		}

		return bSuccess;
	};

	if (bNoCompression)
	{
		bSuccess = ReadFromArchive(BinaryData);
	}
	else
	{
		FArchiveLoadCompressedProxy Decompressor = FArchiveLoadCompressedProxy(BinaryData, NAME_Oodle);

		if (Decompressor.GetError())
		{
			UE_LOG(LogRPGSave, Error, TEXT("Cannot load, file might not be compressed: %s"), *FullSavePath);
			return false;
		}

		FBufferArchive DecompressedBinary;
		Decompressor << DecompressedBinary;

		bSuccess = ReadFromArchive(DecompressedBinary);

		//Proper cleanup
		Decompressor.Close();        //Close decompressor
		DecompressedBinary.Empty();  //Clear decompressed memory
	}

	return bSuccess;
}

/**
Base Serialize Functions
**/

void URPGSaveSubsystemBase::SerializeToBinary(UObject* Object, TArray<uint8>& OutData) const
{
	FMemoryWriter MemoryWriter(OutData, true);
	FSaveGameArchive Ar(MemoryWriter);
	Object->Serialize(Ar);

	//Write Multi-Level package tag
	if (FSaveVersion::RequiresPerObjectPackageTag(Object))
	{
		FSaveVersion::WriteObjectPackageTag(OutData);
	}
}

void URPGSaveSubsystemBase::SerializeFromBinary(UObject* Object, const TArray<uint8>& InData)
{
	FMemoryReader MemoryReader(InData, true);
	ReadPackageInfo(MemoryReader);

	//Check for Multi-Level package version tag
	if (FSaveVersion::RequiresPerObjectPackageTag(Object))
	{
		if (!FSaveVersion::CheckObjectPackageTag(InData))
		{
			//Without tag, we assume the old package version.
			const FPackageFileVersion OldPackage = FSaveVersion::GetStaticOldPackageVersion();
			MemoryReader.SetUEVer(OldPackage);
		}
	}

	FSaveGameArchive Ar(MemoryReader);
	Object->Serialize(Ar);
}

/**
Versioning Functions
**/

void URPGSaveSubsystemBase::WritePackageInfo(FBufferArchive& ToBinary) const
{
	//Package info is written at the beginning of the file as first entry to the top-level FBufferArchive for Player, Level, Object
	int32 FileTag = RPGSave::UE_SAVEGAME_FILE_TYPE_TAG;
	FPackageFileVersion Version = GPackageFileUEVersion;
	FEngineVersion EngineVersion = FEngineVersion::Current();

	ToBinary << FileTag;
	ToBinary << Version;
	ToBinary << EngineVersion;
}

void URPGSaveSubsystemBase::ReadPackageInfo(FMemoryReader& MemoryReader, const bool bSeekInitialVersion)
{
	//This is done once when initially reading the file
	if (bSeekInitialVersion)
	{
		int32 FileTag;
		FPackageFileVersion FileVersion;
		FEngineVersion EngineVersion;

		MemoryReader << FileTag;

		//No file tag means an old file.
		if (FileTag != RPGSave::UE_SAVEGAME_FILE_TYPE_TAG)
		{
			//Start from beginning
			MemoryReader.Seek(0);

			LoadedPackageVersion = FSaveVersion::GetStaticOldPackageVersion();
			LoadedEngineVersion = FEngineVersion();

			UE_LOG(LogRPGSave, Warning, TEXT("File version empty. Using 'Old Save Package Version': %d"), LoadedPackageVersion.ToValue());
		}
		else
		{
			MemoryReader << FileVersion;
			MemoryReader << EngineVersion;

			LoadedPackageVersion = FileVersion;
			LoadedEngineVersion = EngineVersion;
		}
	}

	//Sub-archives also require the correct version to be set, so we use the initial version globally 
	MemoryReader.SetUEVer(LoadedPackageVersion);
	MemoryReader.SetEngineVer(LoadedEngineVersion);
}

void URPGSaveSubsystemBase::WriteGameVersionInfo(FBufferArchive& ToBinary) const
{
	//Game version info is written to the end of the file
	FSaveVersionInfo GameVersion = FSaveVersion::MakeSaveFileVersion();
	ToBinary << GameVersion;
}

void URPGSaveSubsystemBase::ReadGameVersionInfo(FMemoryReader& FromBinary)
{
	//Called from LoadBinaryArchive
	FromBinary << LastReadVersion;

	UE_LOG(LogRPGSave, Log, TEXT("%s | Package: %d | Engine: %s | Plugin: %s | Game: %s"),
		*RPGSave::VersionLogText, LoadedPackageVersion.ToValue(), *LoadedEngineVersion.ToString(), *LastReadVersion.Plugin, *LastReadVersion.Game);
}

bool URPGSaveSubsystemBase::CheckSaveGameIntegrity(const EDataLoadType Type, const FString& FullSavePath, const bool bComplexCheck)
{
	const EFileValidity ValidCheck = IsSaveFileValid(FullSavePath);

	//A missing file can return true, as there is nothing to check
	bool bSuccess = ValidCheck != EFileValidity::FILE_INVALID;

	//Integrity check, logging handled internally
	if (ValidCheck == EFileValidity::FILE_VALID)
	{
		if (bComplexCheck)
		{
			//Use a fire and forget dummy object, to keep the check independent
			UInfoSaveGame* DummyObject = Type == EDataLoadType::DATA_Object ? NewObject<UInfoSaveGame>(GetTransientPackage()) : nullptr;
			bSuccess = LoadBinaryArchive(Type, FullSavePath, DummyObject, true);

			if (!FSaveVersion::IsSaveGameVersionEqual(LastReadVersion))
			{
				UE_LOG(LogRPGSave, Warning, TEXT("Save Game Version mismatch: File: %s | Settings: %s"), *LastReadVersion.Game, *FSaveVersion::GetGameVersion());
				bSuccess = false;
			}
		}
	}

	//Make sure we are not loading memory-only data later
	ClearLoadFromMemory();

	return bSuccess;
}

/**
File System
**/

bool URPGSaveSubsystemBase::VerifyOrCreateDirectory(const FString& NewDir) const
{
	//Not required for console
	if (IsConsoleFileSystem())
	{
		return true;
	}

	const FString SaveFile = FPaths::Combine(BaseSaveDir(), NewDir);
	if (IFileManager::Get().DirectoryExists(*SaveFile))
	{
		return true;
	}

	return IFileManager::Get().MakeDirectory(*SaveFile, true);
}

FString URPGSaveSubsystemBase::SaveUserDir()
{
	return FPaths::ProjectSavedDir() + TEXT("SaveGames/Users/");
}

FString URPGSaveSubsystemBase::UserSubDir() const
{
	// Takes into account the already defined path from ISaveGame
	return TEXT("Users/") + GetCurrentSaveUserName() + RPGSave::Slash;
}

FString URPGSaveSubsystemBase::BaseSaveDir() const
{
	if (HasSaveUserName())
	{
		return SaveUserDir() + GetCurrentSaveUserName() + RPGSave::Slash;
	}

	return FPaths::ProjectSavedDir() + RPGSave::SaveGamesFolder;
}

FString URPGSaveSubsystemBase::ConsoleSaveDir()
{
	return FPaths::ProjectSavedDir() + RPGSave::SaveGamesFolder;
}

FString URPGSaveSubsystemBase::GetThumbnailFormat()
{
	return FSavePaths::GetThumbnailFormat();
}

FString URPGSaveSubsystemBase::GetThumbnailFileExtension()
{
	return FSavePaths::GetThumbnailFileExtension();
}

FString URPGSaveSubsystemBase::AllThumbnailFiles() const
{
	return BaseSaveDir() + TEXT("*") + GetThumbnailFileExtension();
}

FString URPGSaveSubsystemBase::GetFolderOrFile() const
{
	//Console uses file names and not folders, "/" will automatically create a new folder.
	const bool bFile = IsConsoleFileSystem();
	const FString FolderOrFile = bFile ? RPGSave::UnderscoreFile : RPGSave::Slash;

	return FolderOrFile;
}

FString URPGSaveSubsystemBase::FullSaveDir(const FString& DataType, FString SaveGameName) const
{
	if (SaveGameName.IsEmpty())
	{
		SaveGameName = GetCurrentSaveGameName();
	}

	// *.sav is added by ISaveInterface
	const FString FullName = SaveGameName + GetFolderOrFile() + DataType;

	if (HasSaveUserName())
	{
		return UserSubDir() + FullName;
	}

	return FullName;
}

FString URPGSaveSubsystemBase::CustomSaveFile(const FString& CustomSaveName, const FString& SlotName) const
{
	//Bound to a save slot, use default dir.
	if (!SlotName.IsEmpty())
	{
		return FullSaveDir(CustomSaveName, SlotName);
	}

	//Not bound to slot, so we just save in the base folder. with user if desired.
	if (HasSaveUserName())
	{
		return UserSubDir() + CustomSaveName;
	}

	return CustomSaveName;
}

FString URPGSaveSubsystemBase::SlotInfoSaveFile(const FString& SaveGameName) const
{
	return FullSaveDir(RPGSave::SlotSuffix, SaveGameName);
}

FString URPGSaveSubsystemBase::ActorSaveFile(const FString& SaveGameName) const
{
	return FullSaveDir(RPGSave::ActorSuffix, SaveGameName);
}

FString URPGSaveSubsystemBase::PlayerSaveFile(const FString& SaveGameName) const
{
	return FullSaveDir(RPGSave::PlayerSuffix, SaveGameName);
}

FString URPGSaveSubsystemBase::ThumbnailSaveFile(const FString& SaveGameName) const
{
	const FString ThumbnailPath = BaseSaveDir() + SaveGameName + GetFolderOrFile();
	return ThumbnailPath + RPGSave::ThumbSuffix + GetThumbnailFileExtension();
}

FString URPGSaveSubsystemBase::SlotFilePath(const FString& SaveGameName) const
{
	// This is only used for sorting.
	return BaseSaveDir() + SlotInfoSaveFile(SaveGameName) + RPGSave::SaveType;
}

bool URPGSaveSubsystemBase::DoesFileExist(const FString& InFullFilePath) const
{
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	const bool bHasFile = SaveSystem->DoesSaveGameExist(*InFullFilePath, PlayerIndex);
	return bHasFile;
}

/**
Thumbnail Saving
Export from a 2d scene capture render target source.
**/

UTexture2D* URPGSaveSubsystemBase::ImportSaveThumbnail(const FString& SaveGameName)
{
	const FString SaveThumbnailName = ThumbnailSaveFile(SaveGameName);

	//Suppress warning messages when we dont have a thumb yet.
	if (FPaths::FileExists(SaveThumbnailName))
	{
		return FImageUtils::ImportFileAsTexture2D(SaveThumbnailName);
	}

	return nullptr;
}

void URPGSaveSubsystemBase::ExportSaveThumbnail(UTextureRenderTarget2D* TextureRenderTarget, const FString& SaveGameName)
{
	if (!TextureRenderTarget)
	{
		UE_LOG(LogRPGSave, Warning, TEXT("ExportSaveThumbnailRT: TextureRenderTarget must be non-null"));
	}
	else if (!FSaveThumbnails::HasRenderTargetResource(TextureRenderTarget))
	{
		UE_LOG(LogRPGSave, Warning, TEXT("ExportSaveThumbnailRT: Render target has been released"));
	}
	else if (SaveGameName.IsEmpty())
	{
		UE_LOG(LogRPGSave, Warning, TEXT("ExportSaveThumbnailRT: FileName must be non-empty"));
	}
	else
	{
		const FString SaveThumbnailName = ThumbnailSaveFile(SaveGameName);
		const bool bSuccess = FSaveThumbnails::ExportRenderTarget(TextureRenderTarget, SaveThumbnailName);

		if (!bSuccess)
		{
			UE_LOG(LogRPGSave, Warning, TEXT("ExportSaveThumbnailRT: FileWrite failed to create"));
		}
	}
}

