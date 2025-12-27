
#pragma once

#include "RPGSaveTypes.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

class RPGSYSTEM_API FSettingHelpers
{

public:

	static bool IsNormalMultiLevelSave();
	static bool IsStreamMultiLevelSave();
	static bool IsFullMultiLevelSave();
	static bool IsStackBasedMultiLevelSave();
	static bool IsContainingStreamMultiLevelSave();

	static bool IsDynamicLevelStreaming();

	static bool IsMemoryOnlySave();

	static bool IsConsoleFileSystem();

	static bool IsMultiThreadSaving();
	static bool IsMultiThreadLoading();
	static bool IsDeferredLoading();

	static uint32 GetLoadBatchSize();
};

class RPGSYSTEM_API FAsyncSaveHelpers
{

public:

	template<class T>
	static bool CheckLoadIterator(const T& It, const ESaveGameMode Mode, const bool bLog, const FString& DebugString);

	static bool IsAsyncSaveOrLoadTaskActive(const UWorld* InWorld, const ESaveGameMode Mode, const EAsyncCheckType CheckType, const bool bLog);
	static bool IsStreamAutoLoadActive(const ULevel* InLevel);
	static void DestroyStreamAutoLoadTask(const ULevel* InLevel);
	static void DestroyAsyncLoadLevelTask();

	static ESaveGameMode GetMode(const int32 Data);
};
