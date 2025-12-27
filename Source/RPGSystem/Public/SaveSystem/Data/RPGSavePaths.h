
#pragma once

#include "CoreMinimal.h"

struct FSaveSlotInfo;

class FArchive;
class UTextureRenderTarget2D;

class RPGSYSTEM_API FSavePaths
{

public:

	static TArray<FString> GetDefaultSaveFiles(const FString& SaveGameName);

	static FString GetBackupSavePath(const FString& FullSavePath);
	static FString GetBackupFileName(const FString& BaseName);

	static TArray<FString> GetSortedSaveSlots(TArray<FSaveSlotInfo>& SaveSlots);

	static FString ValidateSaveName(const FString& SaveGameName);

	static FString GetThumbnailFormat();
	static FString GetThumbnailFileExtension();

	static void CheckForReadOnly(const FString& FullSavePath);

	static TArray<FString> GetConsoleSlotFiles(const TArray<FString>& SaveGameNames);
};

class RPGSYSTEM_API FSaveThumbnails
{

public:

	static bool HasRenderTargetResource(UTextureRenderTarget2D* TextureRenderTarget);
	static bool CompressRenderTarget(UTextureRenderTarget2D* TexRT, FArchive& Ar);
	static bool ExportRenderTarget(UTextureRenderTarget2D* TexRT, const FString& FileName);
};