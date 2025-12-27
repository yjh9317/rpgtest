// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Data/RPGSavePaths.h"
#include "SaveSystem/Data/RPGSaveMisc.h"
#include "SaveSystem/Data/RPGSaveData.h"
#include "../ProjectSettings/RPGSaveProjectSetting.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/Paths.h"
#include "Serialization/BufferArchive.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "ImageUtils.h"


TArray<FString> FSavePaths::GetDefaultSaveFiles(const FString& SaveGameName)
{
	TArray<FString> AllFiles;

	using namespace RPGSave;
	{
		const FString PlayerFile = SaveGameName + UnderscoreFile + PlayerSuffix;
		const FString LevelFile = SaveGameName + UnderscoreFile + ActorSuffix;
		const FString SlotFile = SaveGameName + UnderscoreFile + SlotSuffix;
		const FString ThumbFile = SaveGameName + UnderscoreFile + ThumbSuffix;

		AllFiles.Add(PlayerFile);
		AllFiles.Add(LevelFile);
		AllFiles.Add(SlotFile);
		AllFiles.Add(ThumbFile);
	}

	return AllFiles;
}

FString FSavePaths::GetBackupSavePath(const FString& FullSavePath)
{
	FString SavePath = FullSavePath;

	if (FSettingHelpers::IsConsoleFileSystem())
	{
		//Uses prefix
		const FString BackupTag = RPGSave::BackupTag + RPGSave::UnderscoreFile;
		SavePath = BackupTag + FullSavePath;
	}
	else
	{
		//Uses suffix
		const int32 SlashIndex = SavePath.Find(RPGSave::Slash);
		if (SlashIndex != INDEX_NONE)
		{
			const FString BackupTag = RPGSave::UnderscoreFile + RPGSave::BackupTag;
			const FString Directory = SavePath.Left(SlashIndex) + BackupTag;
			const FString FileName = SavePath.Mid(SlashIndex + 1);
			SavePath = Directory + RPGSave::Slash + FileName;
		}
	}

	return SavePath;
}

FString FSavePaths::GetBackupFileName(const FString& BaseName)
{
	if (FSettingHelpers::IsConsoleFileSystem())
	{
		return RPGSave::BackupTag + RPGSave::UnderscoreFile + BaseName;
	}

	return BaseName + RPGSave::UnderscoreFile + RPGSave::BackupTag;
}

TArray<FString> FSavePaths::GetSortedSaveSlots(TArray<FSaveSlotInfo>& SaveSlots)
{
	SaveSlots.Sort([](const FSaveSlotInfo& A, const FSaveSlotInfo& B)
	{
		return A.TimeStamp > B.TimeStamp;
	});

	TArray<FString> SaveSlotNames;
	for (const FSaveSlotInfo& SlotInfo : SaveSlots)
	{
		//Skip backup tag
		if (SlotInfo.Name.Contains(RPGSave::BackupTag))
		{
			continue;
		}

		SaveSlotNames.Add(SlotInfo.Name);
	}

	return SaveSlotNames;
}

FString FSavePaths::ValidateSaveName(const FString& SaveGameName)
{
	FString CurrentSave = SaveGameName;
	CurrentSave = CurrentSave.Replace(TEXT(" "), *RPGSave::UnderscoreFile);
	CurrentSave = CurrentSave.Replace(TEXT("."), *RPGSave::UnderscoreFile);

	return FPaths::MakeValidFileName(*CurrentSave);
}

FString FSavePaths::GetThumbnailFormat()
{
	if (URPGSaveProjectSetting::Get()->ThumbnailFormat == EThumbnailImageFormat::Png)
	{
		return RPGSave::ImgFormatPNG;
	}

	return RPGSave::ImgFormatJPG;
}

FString FSavePaths::GetThumbnailFileExtension()
{
	if (FSettingHelpers::IsConsoleFileSystem())
	{
		return RPGSave::SaveType;
	}

	return TEXT(".") + GetThumbnailFormat();
}

void FSavePaths::CheckForReadOnly(const FString& FullSavePath)
{
	const FString NativePath = FString::Printf(RPGSave::NativeDesktopSavePath, *FPaths::ProjectSavedDir(), *FullSavePath);
	const bool bReadOnly = IFileManager::Get().IsReadOnly(*NativePath);
	if (bReadOnly)
	{
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*NativePath, false);
		UE_LOG(LogRPGSave, Warning, TEXT("File access read only was set to false: %s"), *NativePath);
	}
}

TArray<FString> FSavePaths::GetConsoleSlotFiles(const TArray<FString>& SaveGameNames)
{
	const FString FullSlotSuffix = RPGSave::UnderscoreFile + RPGSave::SlotSuffix;

	//Filter out slots
	TArray<FString> SlotNames;
	for (const FString& ActualFileName : SaveGameNames)
	{
		const bool bIsActualSlot = ActualFileName.Contains(FullSlotSuffix);
		if (bIsActualSlot)
		{
			//Get actual name without suffix
			const int32 Index = ActualFileName.Find(FullSlotSuffix, ESearchCase::IgnoreCase, ESearchDir::FromEnd, INDEX_NONE);
			const int32 Count = FullSlotSuffix.Len();

			FString ReducedFileName = ActualFileName;
			ReducedFileName.RemoveAt(Index, Count);
			SlotNames.Add(ReducedFileName);
		}
	}

	return SlotNames;
}

/**
Thumbnail Functions
**/

bool FSaveThumbnails::HasRenderTargetResource(UTextureRenderTarget2D* TextureRenderTarget)
{
	return TextureRenderTarget->GetResource() != nullptr;
}

bool FSaveThumbnails::CompressRenderTarget(UTextureRenderTarget2D* TexRT, FArchive& Ar)
{
	FImage Image;
	if (!FImageUtils::GetRenderTargetImage(TexRT, Image))
	{
		return false;
	}

	TArray64<uint8> CompressedData;
	if (!FImageUtils::CompressImage(CompressedData, *FSavePaths::GetThumbnailFormat(), Image, 90))
	{
		return false;
	}

	Ar.Serialize((void*)CompressedData.GetData(), CompressedData.GetAllocatedSize());

	return true;
}

bool FSaveThumbnails::ExportRenderTarget(UTextureRenderTarget2D* TexRT, const FString& FileName)
{
	FArchive* Ar = IFileManager::Get().CreateFileWriter(*FileName);
	if (Ar)
	{
		FBufferArchive Buffer;
		if (CompressRenderTarget(TexRT, Buffer))
		{
			Ar->Serialize(const_cast<uint8*>(Buffer.GetData()), Buffer.Num());
			delete Ar;

			return true;
		}
	}

	return false;
}
