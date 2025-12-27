#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RPGSaveTypes.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRPGSave, Log, All);

#define RPG_VERSION_NUMBER 174
#define RPG_ENGINE_MIN_UE55 (ENGINE_MAJOR_VERSION == 5 && ENGINE_MINOR_VERSION >= 5)
#define RPG_PLATFORM_DESKTOP (PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX)

#define ENUM_TO_FLAG(Enum) (1 << static_cast<uint8>(Enum)) 

/**
Types
**/

namespace RPGSave
{
	static const FName HasLoadedTag(TEXT("RPG_HasLoaded"));
	static const FName SkipSaveTag(TEXT("RPG_SkipSave"));
	static const FName PersistentTag(TEXT("RPG_Persistent"));
	static const FName SkipTransformTag(TEXT("RPG_SkipTransform"));

	//Used for filenames only. Playstation requires an alphanumeric character like "x" 
	//Set bAllowConfidentialPlatformDefines = true; in the plugin build.cs file.
	static const FString UnderscoreFile(
#if defined(PLATFORM_PS5) && PLATFORM_PS5
		TEXT("x")
#elif defined(PLATFORM_PS4) && PLATFORM_PS4
		TEXT("x")
#else
		TEXT("_")
#endif
	);
	
	//Used for internal names only
	static const FString UnderscoreInt(TEXT("_"));  
	
	static const FString Slash(TEXT("/"));

	static const FString PlayerSuffix(TEXT("Player"));
	static const FString ActorSuffix(TEXT("Level"));
	static const FString SlotSuffix(TEXT("Slot"));
	static const FString ThumbSuffix(TEXT("Thumb"));

	static const FString RawObjectTag(TEXT("@raw"));

	static const FString BackupTag(TEXT("bak0"));

	static const FString SaveGamesFolder(TEXT("SaveGames/"));
	static const FString SaveType(TEXT(".sav"));

	static const FName PersistentActors(TEXT("VirtualPersistentActorLevel"));

	static constexpr TCHAR RuntimeLevelInstance[] = (TEXT("LevelStreamingDynamic"));

	static constexpr TCHAR NativeDesktopSavePath[] = TEXT("%sSaveGames/%s.sav");

	//Same as MetaClass from URPGPluginSettings->SlotInfoSaveGameClass
	static const FSoftClassPath DefaultSlotClass(TEXT("/Script/RPGSystem.RPGInfoSaveGame"));

	constexpr int32 BigNumber = 100000;
	constexpr float ShortDelay = 0.1f;

	constexpr uint32 MinAsyncWaitFrames = 5;

	static const FString ImgFormatPNG(TEXT("png"));
	static const FString ImgFormatJPG(TEXT("jpg"));

	static const FString VerPlugin(TEXT("RPG_"));
	static const FString VerGame(TEXT("SAVEGAME_"));
	static const FString VersionLogText(TEXT("Save File Version"));

	//With each new UE package version, we may also need to change this tag
	#define RPG_PKG_TAG_SIZE 8
	static const uint8 UE_OBJECT_PACKAGE_TAG[RPG_PKG_TAG_SIZE] = { 0xAB, 0xCD, 0xEF, 0x12, 0x34, 0x56, 0x78, 0x9A };
	static const int UE_SAVEGAME_FILE_TYPE_TAG = 0x53415647; // "SAVG"

	static const int ARCHIVE_DATA_TAG = 0x41534456; // "ASDV"  
	static const uint32 ACTOR_DATA_VERSION = 1;

	template <typename TArrayType>
	inline static bool ArrayEmpty(const TArrayType& InArray) { return InArray.Num() <= 0; }

	inline static bool EqualString(const FString& A, const FString& B) { return A.Equals(B, ESearchCase::IgnoreCase); }
}

/**
Enums
**/

//Serialized values, never change order or add entries in the middle.
UENUM()
enum class EActorType : uint8
{
	AT_Runtime,
	AT_Placed,
	AT_LevelScript,
	AT_PlayerActor,
	AT_PlayerPawn,
	AT_GameObject,
	AT_Persistent,
	AT_Destroyed,
};

UENUM()
enum class EDataLoadType : uint8
{
	DATA_Level,
	DATA_Player,
	DATA_Object,
};

UENUM()
enum class ESaveGameMode : uint8
{
	MODE_Player,
	MODE_Level,
	MODE_All,
};

UENUM()
enum class EAsyncCheckType : uint8
{
	CT_Both UMETA(DisplayName = "Both"),
	CT_Save UMETA(DisplayName = "Save Only"),
	CT_Load UMETA(DisplayName = "Load Only"),
};

UENUM(BlueprintType)
enum class ESaveErrorType : uint8
{
	ER_Player UMETA(DisplayName = "Player Actors"),
	ER_Level  UMETA(DisplayName = "Level Actors"),
	ER_Object UMETA(DisplayName = "Object"),
};

UENUM()
enum class EPrepareType : uint8
{
	PT_Default,
	PT_RuntimeOnly,
	PT_FullReload,
};

UENUM()
enum class EFileValidity : uint8
{
	FILE_VALID,
	FILE_MISSING,
	FILE_INVALID,
};

UENUM()
enum class ENextStepType : uint8
{
	SaveLevel,
	FinishSave,
};

UENUM()
enum class ELoadMethod : uint8
{
	/** Useful for small amounts of Actors. Blocks the game thread during load. */
	LM_Default   UMETA(DisplayName = "Default"),

	/** Useful large amounts of runtime spawned Actors. */
	LM_Deferred  UMETA(DisplayName = "Deferred"),

	/** Useful for large amounts of placed Actors.	*/
	LM_Thread   UMETA(DisplayName = "Multi-Thread"),
};

UENUM()
enum class EFileSaveMethod : uint8
{
	/** Each slot has it's own folder. */
	FM_Desktop  UMETA(DisplayName = "Desktop"),

	/** No folders. No Compression. Each slot has it's own files with '_SlotName' suffix. */
	FM_Console   UMETA(DisplayName = "Console"),
};

UENUM(BlueprintType, meta = (Bitflags))
enum class ESaveTypeFlags : uint8
{
	/** Save Player Controller, Pawn and Player State. */
	SF_Player = 0		UMETA(DisplayName = "Player Actors"),

	/** Save Level Actors and Level Blueprints. */
	SF_Level = 1		UMETA(DisplayName = "Level Actors"),
};

UENUM(BlueprintType, meta = (Bitflags))
enum class ELoadTypeFlags : uint8
{
	/** Load Player Controller, Pawn and Player State. */
	LF_Player = 0		UMETA(DisplayName = "Player Actors"),

	/** Load Level Actors and Level Blueprints. */
	LF_Level = 1		UMETA(DisplayName = "Level Actors"),
};

UENUM()
enum class EMultiLevelSaveMethod : uint8
{
	/** Multi Level Saving Disabled. */
	ML_Disabled   UMETA(DisplayName = "Disabled"),

	/** For multiple Persistent Levels only. */
	ML_Normal  UMETA(DisplayName = "Basic"),

	/** For one Persistent Level with World Partition or Streaming Sub-Levels. */
	ML_Stream  UMETA(DisplayName = "Streaming"),

	/** For multiple Persistent Levels with World Partition and/or Streaming Sub-Levels. */
	ML_Full  UMETA(DisplayName = "Full"),

};

UENUM()
enum class EThumbnailImageFormat : uint8
{
	/** Use high quality, but slow png compression. */
	Png,

	/** Use lower quality, but fast jpeg compression. */
	Jpeg,
};

UENUM(BlueprintType)
enum class ESaveFileCheckType : uint8
{
	/** Check Current Slot, Player and Level Files.*/
	CheckForGame UMETA(DisplayName = "Default"),

	/** Check Custom Save File without considering Slots.*/
	CheckForCustom UMETA(DisplayName = "Custom Save"),

	/** Check Custom Save File or Custom Player within the Current Save Slot.*/
	CheckForCustomSlot UMETA(DisplayName = "Custom Save in Slot"),

	/** Will Check only for the Slot Info of the Current Save Slot.*/
	CheckForSlotOnly UMETA(DisplayName = "Slot Only"),
};

UENUM()
enum class EOldPackageEngine : uint8
{
	/** Unreal Engine 4. Package Version 555. */
	EN_UE40 UMETA(DisplayName = "Unreal Engine 4"),

	/** Unreal Engine 5.0 - 5.3. Package Version 1009. */
	EN_UE50 UMETA(DisplayName = "Unreal Engine 5.0+"),

	/** Unreal Engine 5.4 and newer. Package Version 1012 or higher. */
	EN_UE54 UMETA(DisplayName = "Unreal Engine 5.4+"),
};

UENUM()
enum class EWorldPartitionMethod : uint8
{
	/** Automatic World Partition saving/loading is enabled. */
	Enabled,

	/**
	Automatic World Partition loading is enabled. Data persists in memory during the session.
	Improves stability and performance, but saving to disk must be done manually.
	*/
	MemoryOnly,

	/** Automatic World Partition loading is enabled. All saving must be done manually. */
	LoadOnly,

	/** Automatic World Partition saving/loading is disabled. */
	Disabled,
};

UENUM()
enum class EWorldPartitionInit : uint8
{
	/** Automatically loads Level Actors on begin. */
	Default UMETA(DisplayName = "Default"),

	/** Will not automatically load the Level at the beginning. */
	Skip UMETA(DisplayName = "Skip Initial Load"),
};

UENUM()
enum class ELoadedStateMod : uint8
{
	/** The loaded state of the Actor is not changed. */
	NoModify UMETA(DisplayName = "Do Not Change"),

	/** Set the state of the Actor to unloaded. */
	Unloaded UMETA(DisplayName = "Unloaded"),

	/** Set the state of the Actor to loaded. */
	Loaded UMETA(DisplayName = "Loaded"),
};

UENUM()
enum class EResetCustomSaveType : uint8
{
	/** Resets all properties to their default values and clears all memory data, allowing the Custom Save to be fully reloaded from disk. */
	Full UMETA(DisplayName = "Memory and Object"),

	/** Clears only the memory data while keeping the current property values intact. */
	ClearMemory UMETA(DisplayName = "Memory Only"),

	/** Resets all properties to their default values for the object in memory. */
	ResetDefault UMETA(DisplayName = "Object Only"),
};