#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/Data/RPGSaveTypes.h"
#include "RPGSaveVersion.generated.h"

USTRUCT()
struct FSaveVersionInfo
{
	GENERATED_BODY()

public:

	FSaveVersionInfo()
	{
		Plugin = FString();
		Game = FString();
	}

	FSaveVersionInfo(const FString& InPlugin, const FString& InGame)
	{
		Plugin = InPlugin;
		Game = InGame;
	}

	friend FArchive& operator<<(FArchive& Ar, FSaveVersionInfo& VersionArchive)
	{
		Ar << VersionArchive.Plugin;
		Ar << VersionArchive.Game;
		return Ar;
	}

	bool operator!=(const FSaveVersionInfo& InInfo) const
	{
		return (RPGSave::EqualString(InInfo.Game, Game) && RPGSave::EqualString(InInfo.Plugin, Plugin)) == false;
	}

public:

	FString Plugin;
	FString Game;
};

class RPGSYSTEM_API FSaveVersion
{

public:

	static FString GetGameVersion();
	static FSaveVersionInfo MakeSaveFileVersion();
	static bool IsSaveGameVersionEqual(const FSaveVersionInfo& SaveVersion);

	static FPackageFileVersion GetStaticOldPackageVersion();
	static bool RequiresPerObjectPackageTag(const UObject* Object);

	static void WriteObjectPackageTag(TArray<uint8>& Data);
	static bool CheckObjectPackageTag(const TArray<uint8>& Data);

	static uint8 UpdateArchiveVersion(FArchive& Ar);

	static EFileValidity IsSaveFileValid(const FString& InSavePath, const bool bLog = true);
};

