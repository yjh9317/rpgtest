// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RPGSaveSubsystemBase.h"
#include "RPGSystem/ProjectSettings/RPGSaveProjectSetting.h"
#include "SaveSystem/Data/RPGSaveData.h"
#include "SaveSystem/Data/RPGSaveLevel.h"
#include "Serialization/BufferArchive.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RPGSaveSubsystem.generated.h"

/**
 * 
 */
class UInfoSaveGame;
class UCustomSaveGame;
class URPGAsyncSaveGame;
class URPGAsyncLoadGame;
class FBufferArchive;
class FMemoryReader;
class APawn;
class APlayerController;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRPGLoadPlayerComplete, const APlayerController*, LoadedPlayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRPGLoadLevelComplete, const TArray<TSoftObjectPtr<AActor>>&, LoadedActors);

UCLASS(BlueprintType, meta = (DisplayName = "RPG Save SubSystem", Keywords = "Save, RPGSave"))
class RPGSYSTEM_API URPGSaveSubsystem : public URPGSaveSubsystemBase
{
	GENERATED_BODY()

protected:
	URPGSaveSubsystem();
	
/** Variables */

public:

	UPROPERTY(BlueprintAssignable, Category = "RPG Save | Delegates")
	FRPGLoadPlayerComplete OnPlayerLoaded;

	UPROPERTY(BlueprintAssignable, Category = "RPG Save | Delegates")
	FRPGLoadLevelComplete OnLevelLoaded;

	UPROPERTY(BlueprintAssignable, Category = "RPG Save | Delegates")
	FRPGLoadLevelComplete OnPartitionLoaded;

private:

	FCriticalSection SaveActorsScope;
	FCriticalSection LoadActorScope;

	uint8 bInitWorldPartitionLoad : 1;
	uint8 bLoadPartition : 1;
	uint8 bSavePartition : 1;

	float WorldPartitionInitTimer;

	FDelegateHandle ActorDestroyedDelegate;

private:

	UPROPERTY(Transient)
	TSet<TWeakObjectPtr<AActor>> ActorList;

	UPROPERTY(Transient)
	TMap<FName, const TWeakObjectPtr<AActor>> ActorMap;

private:

	UPROPERTY(Transient)
	TArray<FLevelArchive> LevelArchiveList;	

	UPROPERTY(Transient)
	FMultiLevelStreamingData MultiLevelStreamData;
	
	UPROPERTY(Transient)
	TArray<FActorSaveData> SavedActors;

	UPROPERTY(Transient)
	TArray<FActorSaveData> SavedActorsPruned;

	UPROPERTY(Transient)
	TArray<FLevelScriptSaveData> SavedScripts;

	UPROPERTY(Transient)
	FGameObjectSaveData SavedGameMode;

	UPROPERTY(Transient)
	FGameObjectSaveData SavedGameState;

private:

	UPROPERTY(Transient)
	TSet<FActorSaveData> WorldPartitionActors;

	UPROPERTY(Transient)
	TSet<FActorSaveData> DestroyedActors;

	UPROPERTY(Transient)
	TMap<TWeakObjectPtr<AActor>, FGameObjectSaveData> RawObjectData;

	UPROPERTY(Transient)
	TArray<TSoftObjectPtr<AActor>> RealLoadedActors;

private:

	UPROPERTY(Transient)
	FPlayerStackArchive PlayerStackData;

	UPROPERTY(Transient)
	FPlayerArchive SavedPlayer;

/** Blueprint Library function accessors */
	
public:

	UObject* LoadRawObject(AActor* Actor, const FRawObjectSaveData& Data);
	bool SaveRawObject(AActor* Actor, const FRawObjectSaveData& Data);

	bool SavePlayerActorsCustom(AController* Controller, const FString& FileName);
	bool LoadPlayerActorsCustom(AController* Controller, const FString& FileName);
	bool DeleteCustomPlayerFile(const FString& FileName);

/** Other public Functions  */

public:

	static URPGSaveSubsystem* Get(const UObject* WorldContextObject);

	void OnAnyActorDestroyed(AActor* DestroyedActor);

	void PrepareLoadAndSaveActors(const uint32 Flags, const EAsyncCheckType FunctionType, const EPrepareType PrepareType);

	bool SavePlayerActors(APlayerController* Controller, const FString& FileName);
	void LoadPlayerActors(APlayerController* Controller);

	bool SaveLevelActors(const bool bMemoryOnly);
	void LoadLevelActors(URPGAsyncLoadGame* LoadTask);
	void LoadGameMode();
	void LoadLevelScripts();

	void FinishLoadingLevel(const bool bHasLoadedFile);

	bool TryLoadPlayerFile();
	bool TryLoadLevelFile();

	APlayerController* GetPlayerController() const;
	APawn* GetPlayerPawn(const APlayerController* PC) const;

	FTimerManager& GetTimerManager() const;

	bool HasValidGameMode() const;
	bool HasValidPlayer() const;

/** Internal Functions  */

public:

	void SaveActorToBinary(AActor* Actor, FGameObjectSaveData& OutData) const;
	void LoadActorFromBinary(AActor* Actor, const FGameObjectSaveData& InData);

	void SpawnLevelActor(const FActorSaveData& ActorArray);
	void ProcessLevelActor(AActor* Actor, const FActorSaveData& ActorArray);

	void PrepareLevelActors();

protected:

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

private:

	void CreateLevelActor(UClass* SpawnClass, const FActorSaveData& ActorArray, const FActorSpawnParameters& SpawnParams);
	void FailSpawnLevelActor(const FActorSaveData& ActorArray) const;

	bool UnpackBinaryArchive(const EDataLoadType LoadType, FMemoryReader& FromBinary, UObject* Object = nullptr) override;
	bool UnpackLevelArchive(FMemoryReader& FromBinary);
	bool UnpackPlayerArchive(FMemoryReader& FromBinary);
	bool UnpackLevel(const FLevelArchive& LevelArchive);
	void UnpackPlayer(const FPlayerArchive& PlayerArchive);

	void PrepareLoadActor(const uint32 Flags, AActor* Actor, const bool bFullReload);
	void PrepareSaveActor(const EPrepareType PrepareType, AActor* Actor);
	void PrepareFullReload(const uint32 Flags, AActor* Actor) const;
	void AddActorToList(AActor* Actor, const bool bIsLoading);
	
	FGameObjectSaveData ParseGameModeObjectForSaving(AActor* Actor) const;
	FLevelScriptSaveData ParseLevelScriptForSaving(AActor* Actor) const;
	FActorSaveData ParseLevelActorForSaving(AActor* Actor, const EActorType Type) const;

	void ExecuteActorPreSave(AActor* Actor) const;
	void ExecuteActorSaved(AActor* Actor) const;
	void ExecuteActorPreLoad(AActor* Actor) const;
	void ExecuteActorLoaded(AActor* Actor) const;

	void SerializeActorStructProperties(AActor* Actor) const;

	TArray<UActorComponent*> GetSaveComponents(AActor* Actor) const;
	void SaveActorComponents(AActor* Actor, TArray<FComponentSaveData>& OutComponents) const;
	void LoadActorComponents(AActor* Actor, const TArray<FComponentSaveData>& InComponents);

	UObject* SerializeFromRawObject(AActor* Actor, const FRawObjectSaveData& Data, const TArray<FComponentSaveData>& InputArray);
	void AppendRawObjectData(AActor* Actor, TArray<FComponentSaveData>& OutComponents) const;
	void UpdateRawObjectData(AActor* Actor, const FComponentSaveData& InputData);

	FLevelStackArchive AddMultiLevelStackData(const FLevelArchive& LevelArchive, const FLevelArchive& PersistentArchive, const FGameObjectSaveData& InGameMode, const FGameObjectSaveData& InGameState);
	FLevelArchive AddMultiLevelStreamData(const FLevelArchive& LevelArchive);
	void UpdateMultiLevelStreamData(const FLevelArchive& LevelArchive);

/** World Partition Functions  */

private:

	void OnLevelStreamingStateChanged(UWorld* InWorld, const ULevelStreaming* InStreamingLevel, ULevel* InLevelIfLoaded, ELevelStreamingState PreviousState, ELevelStreamingState NewState);
	void OnLevelBeginMakingInvisible(UWorld* InWorld, const ULevelStreaming* InStreamingLevel, ULevel* InLoadedLevel);

	void OnPreWorldInit(UWorld* World, const UWorld::InitializationValues IVS);
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	void RemoveWorldPartitionStreamDelegates();
	
	void TryInitWorldPartition();
	void PollInitWorldPartition();

	void TrySaveWorldPartition();
	void AccumulatedSaveWorldPartition();

public:

	void LoadStreamingActor(AActor* Actor, const FActorSaveData& ActorData);

/** Delegate Functions  */

public:

	inline void BroadcastOnPartitionLoaded()
	{
		OnPartitionLoaded.Broadcast(RealLoadedActors);
		RealLoadedActors.Empty();
	}

	inline void BroadcastOnLevelLoaded()
	{
		OnLevelLoaded.Broadcast(RealLoadedActors);
		RealLoadedActors.Empty();
	}

	inline void AllocateRealLoadedActors(const int32 AllocNum)
	{
		RealLoadedActors.Reserve(AllocNum);
	}

	inline bool HasActuallyLoadedActors()
	{
		return !RPGSave::ArrayEmpty(RealLoadedActors);
	}

/** Clear Data Functions  */

public:

	inline void ClearMultiLevelSave()
	{
		ClearSavedLevelActors();
		ClearStreamingData();
		ClearMultiSaveLevels();
		LevelArchiveList.Empty();
		PlayerStackData = FPlayerStackArchive();
		bLoadFromMemory = false;
	}

	inline void ClearWorldPartition()
	{
		WorldPartitionActors.Empty();
		bSavePartition = false;
		bLoadPartition = false;
		bInitWorldPartitionLoad = false;
		bLoadFromMemory = false;
		WorldPartitionInitTimer = 0.f;
	}

private:

	inline void ClearPlayerPosition()
	{
		SavedPlayer.ClearPosition();
	}

	inline void ClearActorList()
	{
		ActorList.Empty();
		ActorMap.Empty();
	}

	inline void ClearSavedLevelActors()
	{
		SavedActors.Empty();
		SavedScripts.Empty();
		SavedActorsPruned.Empty();
	}

	inline void ClearStreamingData()
	{
		MultiLevelStreamData = FMultiLevelStreamingData();
	}

	inline void ClearWorldPartitionActors()
	{
		WorldPartitionActors.Empty();
	}

	inline void ClearDestroyedActors()
	{
		DestroyedActors.Empty();
	}

	inline void ClearUserData() override
	{
		Super::ClearUserData();

		//When setting/deleting a Save User, we need to clear this
		ClearMultiLevelSave();
		ClearWorldPartition();
	}

/** Actor Helpers  */

public:

	static FName LevelScriptSaveName(const AActor* Actor);
	static EActorType GetActorType(const AActor* Actor);
	static FString GetFullActorName(const AActor* Actor);
	static bool IsLoaded(const AActor* Actor);
	static bool IsSkipSave(const AActor* Actor);
	static bool IsSkipTransform(const AActor* Actor);
	static bool IsLevelScript(const EActorType Type);
	bool HasSaveInterface(const AActor* Actor) const;
	bool HasComponentSaveInterface(const UActorComponent* Comp) const;

/** Other Helper Functions  */

public:

	inline static bool AutoDestroyActors()
	{
		return URPGSaveProjectSetting::Get()->bAutoDestroyActors;
	}

	inline static bool AdvancedSpawnCheck()
	{
		return URPGSaveProjectSetting::Get()->bAdvancedSpawnCheck;
	}

	inline bool IsValidActor(const AActor* Actor) const
	{
		return IsValid(Actor) && HasSaveInterface(Actor);
	}

	inline bool IsValidForSaving(const AActor* Actor) const
	{
		return IsValidActor(Actor) && !IsSkipSave(Actor);
	}

	inline bool IsValidForLoading(const AActor* Actor) const
	{
		return IsValidActor(Actor) && !IsLoaded(Actor);
	}

	inline FMultiLevelStreamingData GetMultiLevelStreamData() const
	{
		return MultiLevelStreamData;
	}

	inline bool HasStreamingLevels()
	{
		return FStreamHelpers::HasStreamingLevels(GetWorld());
	}

	inline bool HasLevelData() const
	{
		return !RPGSave::ArrayEmpty(SavedActors) 
			|| !RPGSave::ArrayEmpty(SavedScripts) 
			|| !RPGSave::ArrayEmpty(SavedGameMode.Data)
			|| !RPGSave::ArrayEmpty(SavedGameState.Data)
			|| MultiLevelStreamData.HasData();
	}

/** World Partition Helpers  */

public:

	inline bool AutoSaveLoadWorldPartition(UWorld* InWorld = nullptr) const
	{
		if (!InWorld)
		{
			InWorld = GetWorld();
		}

		return FStreamHelpers::AutoSaveLoadWorldPartition(InWorld);
	}

	inline bool CanProcessWorldPartition() const
	{
		return FStreamHelpers::CanProcessWorldPartition(GetWorld());
	}

	inline bool IsInitialWorldPartitionLoading() const
	{
		return !bInitWorldPartitionLoad && IsAsyncSaveOrLoadTaskActive(ESaveGameMode::MODE_Level, EAsyncCheckType::CT_Load, false);
	}

	inline bool IsLevelStreaming() const
	{
		return FStreamHelpers::IsLevelStillStreaming(GetWorld());
	}

	inline static bool SkipInitialWorldPartitionLoad()
	{
		return URPGSaveProjectSetting::Get()->WorldPartitionInit == EWorldPartitionInit::Skip;
	}

	inline bool AlwaysAutoLoadWorldPartition() const
	{
		return bInitWorldPartitionLoad && bLoadFromMemory;
	}

	inline bool InitWorldPartitionLoadComplete() const
	{
		return bInitWorldPartitionLoad && !IsLevelStreaming() && !IsInitialWorldPartitionLoading();
	}
};
