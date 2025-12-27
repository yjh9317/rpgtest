// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

class ULevelStreamingDynamic;
class ULevel;
class ILevelInstanceInterface;

class RPGSYSTEM_API FLevelHelpers
{

public:

	static FString StripPIEPrefix(const UWorld* InWorld, const FString& LevelName);

	static FName GetWorldLevelName(const UWorld* InWorld);

	static ULevelStreamingDynamic* GetRuntimeLevelInstance(const AActor* Actor);
	static FName GetStableDynamicLevelInstanceID(const ILevelInstanceInterface* LevelIf, const FString& LevelAssetName);
	static FName GetFullRuntimeLevelInstanceName(const AActor* Actor);
	static FString GetLevelInstanceNameAsString(const AActor* Actor);

	static bool InPersistentLevel(const AActor* Actor);
	static ULevel* GetLoadedStreamingLevel(const UObject* WorldContext, const FName& InLevelName);
	static FName GetStreamingLevelName(const AActor* Actor);
};

class RPGSYSTEM_API FStreamHelpers
{

public:

	static bool AutoSaveLoadWorldPartition(const UWorld* InWorld = nullptr);
	static bool HasStreamingLevels(const UWorld* InWorld);
	static bool IsLevelStillStreaming(const UWorld* InWorld);
	static bool IsWorldPartitionInit(const UWorld* InWorld);
	static bool CanProcessWorldPartition(const UWorld* InWorld);
};