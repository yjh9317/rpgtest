// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"
#include "EngineUtils.h"
#include "ImageUtils.h"
#include "PlatformFeatures.h"
#include "SaveGameSystem.h"
#include "Async/Async.h"
#include "Engine/LevelScriptActor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "SaveSystem/Async/RPGAsyncLoadGame.h"
#include "SaveSystem/Async/RPGAsyncSaveGame.h"
#include "SaveSystem/Async/RPGAsyncStream.h"
#include "SaveSystem/Data/RPGSaveActors.h"
#include "SaveSystem/Interface/RPGActorSaveInterface.h"
#include "SaveSystem/Interface/RPGCompSaveInterface.h"
#include "SaveSystem/SaveGame/CustomSaveGame.h"
#include "SaveSystem/SaveGame/InfoSaveGame.h"
#include "Serialization/ArchiveLoadCompressedProxy.h"
#include "Serialization/ArchiveSaveCompressedProxy.h"
#include "Serialization/BufferArchive.h"
#include "Streaming/LevelStreamingDelegates.h"


URPGSaveSubsystem::URPGSaveSubsystem() : URPGSaveSubsystemBase()
{ 
	bInitWorldPartitionLoad = false;
	bLoadPartition = false;
	bSavePartition = false;
	WorldPartitionInitTimer = 0.f;
}

void URPGSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	FWorldDelegates::OnPreWorldInitialization.AddUObject(this, &URPGSaveSubsystem::OnPreWorldInit);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &URPGSaveSubsystem::OnWorldCleanup);
}

void URPGSaveSubsystem::Deinitialize()
{
	Super::Deinitialize();

	RemoveWorldPartitionStreamDelegates();
	FWorldDelegates::OnPreWorldInitialization.RemoveAll(this);
	FWorldDelegates::OnWorldCleanup.RemoveAll(this);
}

URPGSaveSubsystem* URPGSaveSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World)
	{
		const UGameInstance* GameInst = World->GetGameInstance();
		if (GameInst)
		{
			URPGSaveSubsystem* EMSSubSystem = GameInst->GetSubsystem<URPGSaveSubsystem>();
			if (EMSSubSystem)
			{
				return EMSSubSystem;
			}
		}
	}

	return nullptr;
}

/**
Load Level and Player Files
**/

bool URPGSaveSubsystem::TryLoadPlayerFile()
{
	//Player has no load from memory, as it is loaded once at the beginning in most cases.
	return LoadBinaryArchive(EDataLoadType::DATA_Player, PlayerSaveFile());
}

bool URPGSaveSubsystem::TryLoadLevelFile()
{
	//Check for memory load
	if (bLoadFromMemory && HasLevelData())
	{
		return true;
	}

	const bool bDiskLoadSuccess = LoadBinaryArchive(EDataLoadType::DATA_Level, ActorSaveFile());

	//We don't have data yet, but want to allow auto-saving
	if (!bDiskLoadSuccess && AutoSaveLoadWorldPartition())
	{
		bInitWorldPartitionLoad = true;
	}

	//Allow loading from memory
	bLoadFromMemory = bDiskLoadSuccess;

	return bDiskLoadSuccess;
}

/**
Unpack Archive Functions
**/

bool URPGSaveSubsystem::UnpackBinaryArchive(const EDataLoadType LoadType, FMemoryReader& FromBinary, UObject* Object)
{
	if (FromBinary.IsError() || FromBinary.IsCriticalError())
	{
		UE_LOG(LogRPGSave, Error, TEXT("Unknown archive error. Load type: %d"), uint8(LoadType));
		return false;
	}

	if (LoadType == EDataLoadType::DATA_Level)
	{
		//Per-Level data must be cleared
		ClearSavedLevelActors();
		ClearStreamingData();

		return UnpackLevelArchive(FromBinary);
	}
	else if (LoadType == EDataLoadType::DATA_Player)
	{
		return UnpackPlayerArchive(FromBinary);
	}
	else if (LoadType == EDataLoadType::DATA_Object)
	{
		//We need to get the top level archive, otherwise it would offset the data
		FBufferArchive ObjectArchive;
		FromBinary << ObjectArchive; 

		FMemoryReader MemoryReader(ObjectArchive, true);
		ReadPackageInfo(MemoryReader);

		if (Object)
		{
			FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
			Object->Serialize(Ar);
			return true;
		}
	}

	return false;
}

bool URPGSaveSubsystem::UnpackLevelArchive(FMemoryReader& FromBinary)
{
	bool bLevelLoadSuccess = false;

	//Check for multi level saving.
	if (IsStackBasedMultiLevelSave())
	{
		FLevelStackArchive LevelStack;
		FromBinary << LevelStack;

		//Copy from disk to memory.
		if (RPGSave::ArrayEmpty(LevelArchiveList))
		{
			LevelArchiveList = LevelStack.Archives;
		}

		//It will only unpack the archive for the current level.
		for (const FLevelArchive& StackedArchive : LevelStack.Archives)
		{
			if (StackedArchive.Level == GetLevelName())
			{
				if (IsFullMultiLevelSave())
				{
					UpdateMultiLevelStreamData(StackedArchive);
				}

				UnpackLevel(StackedArchive);
			}
			else if (StackedArchive.Level == RPGSave::PersistentActors)
			{
				//Persistent Actors have their own sub-archive
				UnpackLevel(StackedArchive);
			}
		}

		SavedGameMode = LevelStack.SavedGameMode;
		SavedGameState = LevelStack.SavedGameState;

		bLevelLoadSuccess = true;
	}
	else
	{
		FLevelArchive LevelArchive;
		FromBinary << LevelArchive;

		//Update stream data for current level only
		if (IsStreamMultiLevelSave() && LevelArchive.Level == GetLevelName())
		{
			UpdateMultiLevelStreamData(LevelArchive);
		}

		if (UnpackLevel(LevelArchive))
		{
			bLevelLoadSuccess = true;
		}
	}

	return bLevelLoadSuccess;
}

bool URPGSaveSubsystem::UnpackLevel(const FLevelArchive& LevelArchive)
{
	bool bLevelLoadSuccess = false;

	SavedActors.Reserve(LevelArchive.SavedActors.Num());
	for (const FActorSaveData& TempSavedActor : LevelArchive.SavedActors)
	{
		//Add Actors from the current level or persistent ones
		if (FActorHelpers::IsPersistent(EActorType(TempSavedActor.Type)) || LevelArchive.Level == GetLevelName())
		{
			SavedActors.Add(TempSavedActor);
			bLevelLoadSuccess = true;
		}
	}

	if (LevelArchive.Level == GetLevelName())
	{
		SavedScripts.Append(LevelArchive.SavedScripts);
		bLevelLoadSuccess = true;
	}

	//Basic Multi-Level saving saves mode in the level stack. It is always persistent.
	if (!IsStackBasedMultiLevelSave())
	{
		if (LevelArchive.Level == GetLevelName())
		{
			SavedGameMode = LevelArchive.SavedGameMode;
			SavedGameState = LevelArchive.SavedGameState;
			bLevelLoadSuccess = true;
		}
	}

	return bLevelLoadSuccess;
}

bool URPGSaveSubsystem::UnpackPlayerArchive(FMemoryReader& FromBinary)
{
	//Check for multi level saving.
	if (IsStackBasedMultiLevelSave())
	{
		FPlayerStackArchive PlayerStack;
		FromBinary << PlayerStack;

		//Copy from disk to memory.
		if (PlayerStackData.IsEmpty())
		{
			PlayerStackData = PlayerStack;
		}

		UnpackPlayer(PlayerStack.PlayerArchive);

		//Set transform per level
		for (auto It = PlayerStack.LevelPositions.CreateConstIterator(); It; ++It)
		{
			if (It.Key() == GetLevelName())
			{
				FPlayerPositionArchive PosArchive = It.Value();
				PosArchive.CopyToPlayerArchive(SavedPlayer);
				return true;
			}
		}

		//This prevents loading previous position for an unsaved level
		ClearPlayerPosition();

		return true;
	}
	else
	{
		FPlayerArchive PlayerArchive;
		FromBinary << PlayerArchive;

		if (PlayerArchive.Level == GetLevelName())
		{
			UnpackPlayer(PlayerArchive);
			return true;
		}
	}

	return false;
}

void URPGSaveSubsystem::UnpackPlayer(const FPlayerArchive& PlayerArchive)
{
	SavedPlayer.UnpackPlayer(PlayerArchive);
}

/**
Prepare Actors for Load and Save
**/

void URPGSaveSubsystem::PrepareLoadAndSaveActors(const uint32 Flags, const EAsyncCheckType FunctionType, const EPrepareType PrepareType)
{
	const bool bIsLoading = (FunctionType == EAsyncCheckType::CT_Load);
	const bool bFullReload = (PrepareType == EPrepareType::PT_FullReload);

	//Clear first
	ClearActorList();

	if (bIsLoading && bFullReload)
	{
		ClearLoadFromMemory();
	}

	const UWorld* World = GetWorld();
	for (FActorIterator It(World); It; ++It)
	{
		AActor* Actor = *It;

		//Skip save will fully ignore the Actor
		if (!IsValidActor(Actor) || IsSkipSave(Actor))
		{
			continue;
		}

		if (bIsLoading)
		{
			PrepareLoadActor(Flags, Actor, bFullReload);
		}
		else
		{
			PrepareSaveActor(PrepareType, Actor);
		}
	}
}

void URPGSaveSubsystem::PrepareLoadActor(const uint32 Flags, AActor* Actor, const bool bFullReload)
{
	if (bFullReload)
	{
		PrepareFullReload(Flags, Actor);
	}
	else 
	{
		//Don't consider loaded placed Actors at all. Runtime Actors require another check, otherwise they would duplicate
		if (FActorHelpers::IsPlacedActor(Actor) && IsLoaded(Actor))
		{
			return;
		}
	}

	AddActorToList(Actor, true);
}

void URPGSaveSubsystem::PrepareSaveActor(const EPrepareType PrepareType, AActor* Actor)
{
	//Automatic saving of World Partition cells can skip non-runtime Actors
	if (PrepareType == EPrepareType::PT_RuntimeOnly)
	{
		const EActorType Type = GetActorType(Actor);
		if (!FActorHelpers::IsRuntime(Type))
		{
			return;
		}
	}

	AddActorToList(Actor, false);
}

void URPGSaveSubsystem::PrepareFullReload(const uint32 Flags, AActor* Actor) const
{
	bool bIsPlayer = false;
	if (const APlayerController* PC = GetPlayerController())
	{
		const APawn* PlayerPawn = GetPlayerPawn(PC);
		bIsPlayer = (Actor == PC) || (PC->PlayerState && Actor == PC->PlayerState) || (PlayerPawn && Actor == PlayerPawn);
	}

	if (Flags & ENUM_TO_FLAG(ELoadTypeFlags::LF_Player) && bIsPlayer)
	{
		Actor->Tags.Remove(RPGSave::HasLoadedTag);
	}

	if (Flags & ENUM_TO_FLAG(ELoadTypeFlags::LF_Level) && !bIsPlayer)
	{
		Actor->Tags.Remove(RPGSave::HasLoadedTag);
	}
}

void URPGSaveSubsystem::AddActorToList(AActor* Actor, const bool bIsLoading)
{
	const EActorType Type = GetActorType(Actor);

	//For all, including player, a tick before actual save/load
	SerializeActorStructProperties(Actor);

	/*
	Load the default pawn again with data from the level file, in case player was possessing a different pawn when saving.
	Default pawn is still possessed during prepare.
	We can only load a placed Pawn Actor. If spawned at runtime from PlayerStart, the initial/default Pawn has to be removed manually.
	EMS will respawn the saved Pawn as runtime Actor, keeping any soft-refs available.
	*/
	const bool bIsLevelPlayerPawn = bIsLoading && FActorHelpers::IsPlacedActor(Actor) && Type == EActorType::AT_PlayerPawn;

	//Real level Actors and Scripts
	const bool bIsLevelActor = FActorHelpers::IsLevelActor(Type, true);

	if (bIsLevelActor || bIsLevelPlayerPawn)
	{
		//Actor map for loading
		if (bIsLoading)
		{
			const FName ActorName = IsLevelScript(Type) ? LevelScriptSaveName(Actor) : FName(GetFullActorName(Actor));
			ActorMap.Add(ActorName, Actor);
		}
		//Actor list for saving
		else
		{
			ActorList.Add(Actor);
		}
	}
}

/**
Saving Level Actors
**/

bool URPGSaveSubsystem::SaveLevelActors(const bool bMemoryOnly)
{
	TArray<FActorSaveData> InActors;
	TArray<FActorSaveData> InPersistentActors;
	TArray<FLevelScriptSaveData> InScripts;
	FGameObjectSaveData InGameMode;
	FGameObjectSaveData InGameState;

	FScopeLock Lock(&SaveActorsScope);

	for (const TWeakObjectPtr<AActor>& ActorWeakPtr : ActorList)
	{
		if (!ActorWeakPtr.IsValid(false, true))
		{
			continue;
		}

		AActor* Actor = ActorWeakPtr.Get();
		if (!IsValidForSaving(Actor))
		{
			continue;
		}

		const EActorType Type = GetActorType(Actor);

		switch (Type)
		{
		case EActorType::AT_Runtime:
		case EActorType::AT_Placed:
			{
				const FActorSaveData ActorArray = ParseLevelActorForSaving(Actor, Type);
				InActors.Add(ActorArray);
			}
			break;

		case EActorType::AT_Persistent:
			{
				const FActorSaveData ActorArray = ParseLevelActorForSaving(Actor, Type);
				InPersistentActors.Add(ActorArray);
			}
			break;

		case EActorType::AT_LevelScript:
			{
				const FLevelScriptSaveData ScriptArray = ParseLevelScriptForSaving(Actor);
				InScripts.Add(ScriptArray);
			}
			break;

		default:
			break;
		}
	}

	//World Partition Actors
	if (!RPGSave::ArrayEmpty(WorldPartitionActors))
	{
		InActors.Append(WorldPartitionActors.Array());
		ClearWorldPartitionActors();
	}

	//Destroyed Actors
	if (!RPGSave::ArrayEmpty(DestroyedActors))
	{
		InActors.Append(DestroyedActors.Array());
		ClearDestroyedActors();
	}

	//Game Mode/State Actors
	if (const UWorld* World = GetWorld())
	{
		InGameMode = ParseGameModeObjectForSaving(World->GetAuthGameMode());
		InGameState = ParseGameModeObjectForSaving(World->GetGameState());
	}

	FBufferArchive LevelData;
	WritePackageInfo(LevelData);

	FLevelArchive LevelArchive;
	{
		//Stack based only has one set of data for mode and state, so skip
		if (!IsStackBasedMultiLevelSave())
		{
			LevelArchive.SavedGameMode = InGameMode;
			LevelArchive.SavedGameState = InGameState;

			//Also make sure we add persistent Actors
			InActors.Append(InPersistentActors);
		}

		LevelArchive.SavedActors = InActors;
		LevelArchive.SavedScripts = InScripts;

		LevelArchive.Level = GetLevelName();
	}

	//MLS persistent Archive, which is like a virtual level archive
	FLevelArchive PersistentArchive;
	{
		if (IsStackBasedMultiLevelSave())
		{
			PersistentArchive.SavedActors = InPersistentActors;
			PersistentArchive.Level = RPGSave::PersistentActors;
		}
	}

	//Check for multi level saving.
	if (IsNormalMultiLevelSave())
	{
		FLevelStackArchive LevelStack = AddMultiLevelStackData(LevelArchive, PersistentArchive, InGameMode, InGameState);
		LevelData << LevelStack;
	}
	else if (IsStreamMultiLevelSave())
	{
		FLevelArchive StreamArchive = AddMultiLevelStreamData(LevelArchive);
		LevelData << StreamArchive;
	}
	else if (IsFullMultiLevelSave())
	{
		const FLevelArchive StreamArchive = AddMultiLevelStreamData(LevelArchive);
		FLevelStackArchive MultiLevelStack = AddMultiLevelStackData(StreamArchive, PersistentArchive, InGameMode, InGameState);
		LevelData << MultiLevelStack;
	}
	else
	{
		LevelData << LevelArchive;
	}

	//Fail logging level
	auto LogFailedToSave = []()
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Failed to save Level Actors"));
	};

	if (FSaveHelpers::HasSaveArchiveError(LevelData, ESaveErrorType::ER_Level))
	{
		LogFailedToSave();
		return false;
	}

	//Memory only automatic saving for World Partition cells. Much faster as it skips compression etc. 
	if (bMemoryOnly)
	{
		bLoadFromMemory = true;
		UE_LOG(LogRPGSave, Log, TEXT("Level and Game Actors stored in memory"));
		return true;
	}

	if (SaveBinaryArchive(LevelData, ActorSaveFile()))
	{
		UE_LOG(LogRPGSave, Log, TEXT("Level and Game Actors have been saved"));
		return true;
	}

	LogFailedToSave();

	return false;
}

FGameObjectSaveData URPGSaveSubsystem::ParseGameModeObjectForSaving(AActor* Actor) const
{
	FGameObjectSaveData GameObjectActorData;

	if (Actor && IsValidForSaving(Actor))
	{
		SaveActorToBinary(Actor, GameObjectActorData);
	}

	return GameObjectActorData;
}

FLevelScriptSaveData URPGSaveSubsystem::ParseLevelScriptForSaving(AActor* Actor) const
{
	FLevelScriptSaveData ScriptArray;
	ScriptArray.Name = LevelScriptSaveName(Actor);
	SaveActorToBinary(Actor, ScriptArray.SaveData);

	return ScriptArray;
}

FActorSaveData URPGSaveSubsystem::ParseLevelActorForSaving(AActor* Actor, const EActorType Type) const
{
	FActorSaveData ActorArray;

	ActorArray.Type = uint8(Type);
	ActorArray.Name = BytesFromString(GetFullActorName(Actor));
	
	//Class is saved for runtime and persistent Actors
	if (FActorHelpers::IsRuntime(Type))
	{
		ActorArray.Class = BytesFromString(Actor->GetClass()->GetPathName());
	}

	//No transform for persistent Actors or if skipped
	if (!FActorHelpers::IsPersistent(Type) && FActorHelpers::CanProcessActorTransform(Actor))
	{
		ActorArray.Transform = Actor->GetActorTransform();
	}
	else
	{
		ActorArray.Transform = FTransform::Identity;
	}

	SaveActorToBinary(Actor, ActorArray.SaveData);

	return ActorArray;
}

void URPGSaveSubsystem::OnAnyActorDestroyed(AActor* Actor)
{
	//Check for placed, but add as destroyed
	if (IsValidActor(Actor) && FActorHelpers::IsPlacedActor(Actor))
	{
		FActorSaveData ActorArray;
		ActorArray.Type = uint8(EActorType::AT_Destroyed);
		ActorArray.Name = BytesFromString(GetFullActorName(Actor));
		ActorArray.Transform = Actor->GetActorTransform();

		DestroyedActors.Add(ActorArray);
	}
}

/**
Loading Level Actors
**/

void URPGSaveSubsystem::LoadGameMode()
{
	//Game Mode Actor
	if (!RPGSave::ArrayEmpty(SavedGameMode.Data))
	{
		AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
		if (GameMode && IsValidForLoading(GameMode))
		{
			LoadActorFromBinary(GameMode, SavedGameMode);
			UE_LOG(LogRPGSave, Log, TEXT("Game Mode loaded"));
		}
	}

	//Game State Actor
	if (!RPGSave::ArrayEmpty(SavedGameState.Data))
	{
		AGameStateBase* GameState = GetWorld()->GetGameState();
		if (GameState && IsValidForLoading(GameState))
		{
			LoadActorFromBinary(GameState, SavedGameState);
			UE_LOG(LogRPGSave, Log, TEXT("Game State loaded"));
		}
	}
}

void URPGSaveSubsystem::LoadLevelScripts()
{
	if (ActorMap.IsEmpty())
	{
		return;
	}

	for (const FLevelScriptSaveData& ScriptData : SavedScripts)
	{
		const FName& ActorName = ScriptData.Name;

		if (const TWeakObjectPtr<AActor>* ActorWeakPtr = ActorMap.Find(ActorName))
		{
			if (!ActorWeakPtr->IsValid())
			{
				continue;
			}

			AActor* Actor = ActorWeakPtr->Get();
			if (!IsValidForLoading(Actor))
			{
				continue;
			}

			const EActorType Type = GetActorType(Actor);
			if (!IsLevelScript(Type))
			{
				continue;
			}

			LoadActorFromBinary(Actor, ScriptData.SaveData);
			UE_LOG(LogRPGSave, Log, TEXT("%s Level Blueprint loaded"), *ActorName.ToString());
		}
	}
}

void URPGSaveSubsystem::LoadLevelActors(URPGAsyncLoadGame* LoadTask)
{
	if (!LoadTask)
	{
		return;
	}

	if (RPGSave::ArrayEmpty(SavedActors))
	{
		LoadTask->FinishLoading();
		return;
	}

	//Start the actual loading task
	const FLoaderInitData LoaderData(false, SavedActorsPruned, ActorMap);
	LoadTask->LoadLevelActors(LoaderData);
}

void URPGSaveSubsystem::PrepareLevelActors()
{
	SavedActorsPruned = SavedActors;

	//Make sure we only parse the placed Actors that are actually visible
	if (HasStreamingLevels())
	{
		FActorHelpers::PruneSavedActors(ActorMap, SavedActorsPruned);
	}

	AllocateRealLoadedActors(SavedActors.Num());
}

void URPGSaveSubsystem::SpawnLevelActor(const FActorSaveData & ActorArray)
{
	if (RPGSave::ArrayEmpty(ActorArray.Class))
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Trying to respawn Level Actor with invalid Class"));
		FailSpawnLevelActor(ActorArray);
		return;
	}
	
	if (DestroyedActors.Contains(ActorArray))
	{
		return;
	}

	const EActorType Type = EActorType(ActorArray.Type);
	if (!FActorHelpers::IsRuntime(Type))
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Trying to respawn non-runtime Level Actor from save data"));
		FailSpawnLevelActor(ActorArray);
		return;
	}

	const FString Class = StringFromBytes(ActorArray.Class);
	UClass* SpawnClass = FSpawnHelpers::ResolveSpawnClass(Class);

	if (!SpawnClass)
	{
		const FString ActorName = StringFromBytes(ActorArray.Name);
		UE_LOG(LogRPGSave, Warning, TEXT("Spawn Class '%s' could not be loaded for Actor: %s"), *Class, *ActorName);
		return;
	}

	const FActorSpawnParameters SpawnParams = FSpawnHelpers::GetSpawnParams(this, ActorArray);
	if (SpawnParams.Name.IsNone() || !SpawnClass->ImplementsInterface(URPGActorSaveInterface::StaticClass()))
	{
		return;
	}

	//Actually spawn and process the runtime Actor
	CreateLevelActor(SpawnClass, ActorArray, SpawnParams);
}

void URPGSaveSubsystem::CreateLevelActor(UClass* SpawnClass, const FActorSaveData& ActorArray, const FActorSpawnParameters& SpawnParams)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	//Advanced Spawn checking
	if (AdvancedSpawnCheck())
	{
		if (AActor* Actor = FSpawnHelpers::CheckForExistingActor(World, ActorArray))
		{
			ProcessLevelActor(Actor, ActorArray);
			return;
		}
	}

	AActor* NewActor = World->SpawnActor(SpawnClass, &ActorArray.Transform, SpawnParams);
	if (NewActor)
	{
		ProcessLevelActor(NewActor, ActorArray);
	}
	else
	{
		FailSpawnLevelActor(ActorArray);
	}
}

void URPGSaveSubsystem::ProcessLevelActor(AActor* Actor, const FActorSaveData& ActorArray)
{
	//Destroy
	if (AutoDestroyActors())
	{
		if (ActorArray.Type == uint8(EActorType::AT_Destroyed))
		{
			Actor->Destroy();
			return;
		}
	}

	if (FActorHelpers::CanProcessActorTransform(Actor) && FActorHelpers::HasValidTransform(ActorArray.Transform))
	{
		Actor->SetActorTransform(ActorArray.Transform, false, nullptr, ETeleportType::TeleportPhysics);
	}

	LoadActorFromBinary(Actor, ActorArray.SaveData);
	RealLoadedActors.Add(Actor);
}

void URPGSaveSubsystem::FailSpawnLevelActor(const FActorSaveData& ActorArray) const
{
	const FString ActorName = StringFromBytes(ActorArray.Name);
	UE_LOG(LogRPGSave, Warning, TEXT("Failed to spawn Level Actor: %s"), *ActorName);
}

void URPGSaveSubsystem::FinishLoadingLevel(const bool bHasLoadedFile)
{
	if (!bHasLoadedFile)
	{
		return;
	}

	if (HasActuallyLoadedActors())
	{
		UE_LOG(LogRPGSave, Log, TEXT("Level loaded with a total of %d Actors"), RealLoadedActors.Num());
	}

	//Always fire the event, to make it consistent with the OnCompleted output of the Load node.
	BroadcastOnLevelLoaded();

	if (!bInitWorldPartitionLoad && AutoSaveLoadWorldPartition())
	{
		//Not relevant for automatic loading of World Partition cells
		ClearSavedLevelActors();
		bInitWorldPartitionLoad = true;
	}

	//Free some memory
	ClearActorList();
}

/**
Multi-Level Saving System Functions
**/

FLevelStackArchive URPGSaveSubsystem::AddMultiLevelStackData(const FLevelArchive& LevelArchive, const FLevelArchive& PersistentArchive, const FGameObjectSaveData& InGameMode, const FGameObjectSaveData& InGameState)
{
	//Create a new Stack.
	FLevelStackArchive LevelStack;
	{
		LevelStack.AddTo(LevelArchive);
		LevelStack.SavedGameMode = InGameMode;
		LevelStack.SavedGameState = InGameState;
	}

	//Add data from memory(aka. unloaded levels) to Stack Archive.
	for (const FLevelArchive& MemoryArchive : LevelArchiveList)
	{
		if (MemoryArchive != LevelArchive && MemoryArchive != PersistentArchive)
		{
			LevelStack.AddTo(MemoryArchive);
		}
	}

	//Update the list in memory with data from current level.
	bool bFoundInList = false;
	for (FLevelArchive& ExistingArchive : LevelArchiveList)
	{
		if (ExistingArchive == LevelArchive)
		{
			ExistingArchive.ReplaceWith(LevelArchive);
			bFoundInList = true;
			break;
		}
	}

	//Add LevelArchive to the list if it was not found.
	if (!bFoundInList)
	{
		LevelArchiveList.Add(LevelArchive);
	}

	//Persistent does not need anything in memory. So LevelArchiveList is not updated.
	LevelStack.AddTo(PersistentArchive);

	return LevelStack;
}

FLevelArchive URPGSaveSubsystem::AddMultiLevelStreamData(const FLevelArchive& LevelArchive)
{
	//Add all Actors and Scripts from StreamData to the returned archive that is added to the stack.
	FLevelArchive NewLevelArchive = LevelArchive;
	{
		UpdateMultiLevelStreamData(LevelArchive);
		MultiLevelStreamData.CopyFrom(NewLevelArchive);
	}

	/*
	Update cached actors to avoid reloading them from disk after saving.
	This step is crucial for non-World Partition levels where
	levels are unloaded and reloaded frequently. In World Partition levels,
	the actor data is fetched directly from the multi-level stream data.
	*/
	if (bLoadFromMemory && !AutoSaveLoadWorldPartition())
	{
		ClearSavedLevelActors();
		UnpackLevel(NewLevelArchive);
	}

	return NewLevelArchive;
}

void URPGSaveSubsystem::UpdateMultiLevelStreamData(const FLevelArchive& LevelArchive)
{
	//Either replace or add current Actor data from the archive
	MultiLevelStreamData.CopyTo(LevelArchive);
}

/**
Saving and Loading Player
**/

bool URPGSaveSubsystem::SavePlayerActors(APlayerController* Controller, const FString& FileName)
{
	//Always required
	if (!Controller)
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Trying to save Player with invalid Player Controller"));
		return false;
	}

	if (FileName.IsEmpty())
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Trying to save Player with invalid File Name"));
		return false;
	}

	bool bHasPlayerSaveData = false;

	//Controller
	FControllerSaveData InController;
	if (IsValidForSaving(Controller))
	{
		if (!IsSkipTransform(Controller))
		{
			InController.Rotation = Controller->GetControlRotation();
		}

		SaveActorToBinary(Controller, InController.SaveData);
		bHasPlayerSaveData = true;
	}

	//Pawn
	FPawnSaveData InPawn;
	APawn* Pawn = GetPlayerPawn(Controller);
	if (Pawn && IsValidForSaving(Pawn))
	{
		if (!IsSkipTransform(Pawn))
		{
			InPawn.Position = Pawn->GetActorLocation();
			InPawn.Rotation = Pawn->GetActorRotation();
		}

		SaveActorToBinary(Pawn, InPawn.SaveData);
		bHasPlayerSaveData = true;
	}

	//Player State
	FGameObjectSaveData InPlayerState;
	APlayerState* PlayerState = GetPlayerController()->PlayerState;
	if (PlayerState && IsValidForSaving(PlayerState))
	{
		SaveActorToBinary(PlayerState, InPlayerState);
		bHasPlayerSaveData = true;
	}

	//Without any data, we can just return here
	if (!bHasPlayerSaveData)
	{
		return true;
	}

	FPlayerArchive PlayerArchive;
	{
		PlayerArchive.Controller = InController;
		PlayerArchive.Pawn = InPawn;
		PlayerArchive.State = InPlayerState;
		PlayerArchive.Level = GetLevelName();
	}

	FBufferArchive PlayerData;
	WritePackageInfo(PlayerData);

	//Check for multi level saving.
	if (IsStackBasedMultiLevelSave())
	{
		PlayerStackData.ReplaceOrAdd(PlayerArchive);
		FPlayerStackArchive PlayerStack = PlayerStackData;
		PlayerData << PlayerStack;
	}
	else
	{
		PlayerData << PlayerArchive;
	}

	//Fail logging player
	auto LogFailedToSave = []()
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Failed to save Player"));
	};

	if (FSaveHelpers::HasSaveArchiveError(PlayerData, ESaveErrorType::ER_Player))
	{
		LogFailedToSave();
		return false;
	}

	if (SaveBinaryArchive(PlayerData, FileName))
	{
		UE_LOG(LogRPGSave, Log, TEXT("Player Actors have been saved"));
		return true;
	}

	LogFailedToSave();

	return false;
}

void URPGSaveSubsystem::LoadPlayerActors(APlayerController* Controller)
{
	if (!Controller)
	{
		UE_LOG(LogRPGSave, Warning, TEXT("Trying to load Player with invalid Player Controller"));
		return;
	}

	bool bLoadSuccess = false;

	//Controller
	if (IsValidForLoading(Controller))
	{
		const FControllerSaveData ControllerData = SavedPlayer.Controller;

		if (!IsSkipTransform(Controller) && !ControllerData.Rotation.IsNearlyZero())
		{
			Controller->SetControlRotation(ControllerData.Rotation);
		}

		LoadActorFromBinary(Controller, ControllerData.SaveData);

		UE_LOG(LogRPGSave, Log, TEXT("Player Controller loaded"));
		bLoadSuccess = true;
	}

	//Pawn
	APawn* Pawn = GetPlayerPawn(Controller);
	if (Pawn && IsValidForLoading(Pawn))
	{
		const FPawnSaveData PawnData = SavedPlayer.Pawn;

		if (!IsSkipTransform(Pawn) && !PawnData.Position.IsNearlyZero())
		{
			Pawn->SetActorLocation(PawnData.Position, false, nullptr, ETeleportType::TeleportPhysics);
			Pawn->SetActorRotation(PawnData.Rotation, ETeleportType::TeleportPhysics);
		}

		LoadActorFromBinary(Pawn, PawnData.SaveData);

		UE_LOG(LogRPGSave, Log, TEXT("Player Pawn loaded"));
		bLoadSuccess = true;
	}

	//Player State
	if (SavedPlayer.HasPlayerState())
	{
		APlayerState* PlayerState = GetPlayerController()->PlayerState;
		if (PlayerState && IsValidForLoading(PlayerState))
		{
			LoadActorFromBinary(PlayerState, SavedPlayer.State);

			UE_LOG(LogRPGSave, Log, TEXT("Player State loaded"))
			bLoadSuccess = true;
		}
	}

	if (bLoadSuccess)
	{
		OnPlayerLoaded.Broadcast(Controller);
	}
}

bool URPGSaveSubsystem::SavePlayerActorsCustom(AController* Controller, const FString& FileName)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		const FString SlotFileName = CustomSaveFile(FileName, GetCurrentSaveGameName());
		const bool bSavePlayer = SavePlayerActors(PlayerController, SlotFileName);

		if (bSavePlayer)
		{
			UE_LOG(LogRPGSave, Log, TEXT("Custom Player file was saved with name: %s"), *FileName);
		}

		return bSavePlayer;
	}

	return false;
}

bool URPGSaveSubsystem::LoadPlayerActorsCustom(AController* Controller, const FString& FileName)
{
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		const FString SlotFileName = CustomSaveFile(FileName, GetCurrentSaveGameName());
		const bool bLoadFile = LoadBinaryArchive(EDataLoadType::DATA_Player, SlotFileName);

		//Processing must be synchronous, as the data is stored globally.
		if (bLoadFile)
		{
			//It can always reload
			FActorHelpers::SetPlayerNotLoaded(PlayerController);

			UE_LOG(LogRPGSave, Log, TEXT("Custom Player file was loaded with name: %s"), *FileName);
			LoadPlayerActors(PlayerController);
			return true;
		}
	}

	return false;
}

bool URPGSaveSubsystem::DeleteCustomPlayerFile(const FString& FileName)
{
	const FString SaveFile = CustomSaveFile(FileName, GetCurrentSaveGameName());
	if (DoesFileExist(SaveFile))
	{
		ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
		if (SaveSystem->DeleteGame(false, *SaveFile, PlayerIndex))
		{
			UE_LOG(LogRPGSave, Log, TEXT("Custom Player file was removed with name: %s"), *FileName);
			return true;
		}
	}

	return false;
}

/**
Loading and Saving Components
**/

TArray<UActorComponent*> URPGSaveSubsystem::GetSaveComponents(AActor* Actor) const
{
	if (!HasSaveInterface(Actor))
	{
		return TArray<UActorComponent*>();
	}

	TArray<UActorComponent*> SourceComps;
	IRPGActorSaveInterface::Execute_ComponentsToSave(Actor, SourceComps);

	//Get Components with interface
	for (UActorComponent* Component : Actor->GetComponents())
	{
		if (HasComponentSaveInterface(Component))
		{
			if (SourceComps.Find(Component) == INDEX_NONE)
			{
				SourceComps.Add(Component);
			}
		}
	}

	return SourceComps;
}

void URPGSaveSubsystem::SaveActorComponents(AActor* Actor, TArray<FComponentSaveData>& OutComponents) const
{
	const TArray<UActorComponent*> SourceComps = GetSaveComponents(Actor);
	if (RPGSave::ArrayEmpty(SourceComps))
	{
		return;
	}

	for (UActorComponent* Component : SourceComps)
	{
		if (!Component || !Component->IsRegistered())
		{
			continue;
		}

		const FString CompName = FActorHelpers::GetComponentName(Actor, Component);

		FComponentSaveData ComponentArray;
		ComponentArray.Name = BytesFromString(CompName);

		if (const USceneComponent* SceneComp = Cast<USceneComponent>(Component))
		{
			ComponentArray.Transform = SceneComp->GetRelativeTransform();
		}

		if (const UChildActorComponent* ChildActorComp = Cast<UChildActorComponent>(Component))
		{
			if (AActor* ChildActor = ChildActorComp->GetChildActor())
			{
				if (!HasSaveInterface(ChildActor))
				{
					SerializeToBinary(ChildActor, ComponentArray.Data);
				}
				else
				{
					UE_LOG(LogRPGSave, Warning, TEXT("Child Actor has Actor Save Interface and is already saved as Level Actor, skipping: %s"), *CompName);
				}
			}
		}
		else
		{
			const bool bInterface = HasComponentSaveInterface(Component);

			if (bInterface)
			{
				IRPGCompSaveInterface::Execute_ComponentPreSave(Component);
			}

			SerializeToBinary(Component, ComponentArray.Data);

			if (bInterface)
			{
				IRPGCompSaveInterface::Execute_ComponentSaved(Component);
			}
		}

		OutComponents.Add(ComponentArray);
	}
}

void URPGSaveSubsystem::LoadActorComponents(AActor* Actor, const TArray<FComponentSaveData>& InComponents)
{
	const TArray<UActorComponent*> SourceComps = GetSaveComponents(Actor);
	if (RPGSave::ArrayEmpty(SourceComps))
	{
		return;
	}

	for (UActorComponent* Component : SourceComps)
	{
		if (!Component || !Component->IsRegistered())
		{
			continue; 
		}

		for (const FComponentSaveData& ComponentArray : InComponents)
		{
			const FString CompName = FActorHelpers::GetComponentName(Actor, Component);

			if (!CompareIdentifiers(ComponentArray.Name, CompName))
			{
				continue; 
			}
			
			if (USceneComponent* SceneComp = Cast<USceneComponent>(Component))
			{
				if (FActorHelpers::IsMovable(SceneComp))
				{
					SceneComp->SetRelativeTransform(ComponentArray.Transform, false, nullptr, ETeleportType::TeleportPhysics);
				}
			}

			if (UChildActorComponent* ChildActorComp = Cast<UChildActorComponent>(Component))
			{
				if (AActor* ChildActor = ChildActorComp->GetChildActor())
				{
					if (!HasSaveInterface(ChildActor))
					{
						SerializeFromBinary(ChildActor, ComponentArray.Data);
					}
				}
			}
			else
			{
				const bool bInterface = HasComponentSaveInterface(Component);

				if (bInterface)
				{
					IRPGCompSaveInterface::Execute_ComponentPreLoad(Component);
				}

				SerializeFromBinary(Component, ComponentArray.Data);

				if (bInterface)
				{
					IRPGCompSaveInterface::Execute_ComponentLoaded(Component);
				}
			}	
		}
	}
}

/**
Loading and Saving Raw Objects
**/

UObject* URPGSaveSubsystem::LoadRawObject(AActor* Actor, const FRawObjectSaveData& Data)
{
	if (!IsValidActor(Actor) || !Data.IsValidData())
	{
		return nullptr;
	}

	TArray<FComponentSaveData> ObjectArray;
	const EActorType Type = GetActorType(Actor);

	//Level Actors
	if (FActorHelpers::IsLevelActor(Type, false))
	{
		for (const FActorSaveData& ActorArray : SavedActorsPruned)
		{
			if (CompareIdentifiers(ActorArray.Name, GetFullActorName(Actor)))
			{
				ObjectArray.Append(ActorArray.SaveData.Components);
				return SerializeFromRawObject(Actor, Data, ObjectArray);
			}
		}

		return nullptr;
	}

	//Level Scripts
	if (FActorHelpers::IsLevelScript(Type))
	{
		for (const FLevelScriptSaveData& ScriptData : SavedScripts)
		{
			if (LevelScriptSaveName(Actor) == ScriptData.Name)
			{
				ObjectArray.Append(ScriptData.SaveData.Components);
				return SerializeFromRawObject(Actor, Data, ObjectArray);
			}
		}

		return nullptr;
	}

	//Other Actors
	switch (Type)
	{
	case EActorType::AT_PlayerPawn:
		ObjectArray.Append(SavedPlayer.GetPawnComps());
		break;

	case EActorType::AT_PlayerActor:
		ObjectArray.Append(SavedPlayer.GetStateComps());
		ObjectArray.Append(SavedPlayer.GetControllerComps());
		break;

	case EActorType::AT_GameObject:
		ObjectArray.Append(SavedGameMode.Components);
		ObjectArray.Append(SavedGameState.Components);
		break;

	default:
		return nullptr;
	}

	return SerializeFromRawObject(Actor, Data, ObjectArray);
}

UObject* URPGSaveSubsystem::SerializeFromRawObject(AActor* Actor, const FRawObjectSaveData& Data, const TArray<FComponentSaveData>& InputArray)
{
	if (!Actor || !Data.IsValidData() || RPGSave::ArrayEmpty(InputArray))
	{
		return nullptr;
	}

	const FString FullId = FSaveHelpers::GetRawObjectID(Data);

	//Check the input component array from the Actor and serialize if found
	for (const FComponentSaveData& ComponentData : InputArray)
	{
		if (CompareIdentifiers(ComponentData.Name, FullId))
		{
			FStructHelpers::SerializeStruct(Data.Object);
			SerializeFromBinary(Data.Object, ComponentData.Data);
			UpdateRawObjectData(Actor, ComponentData);
			break;
		}
	}

	return Data.Object;
}

bool URPGSaveSubsystem::SaveRawObject(AActor* Actor, const FRawObjectSaveData& Data)
{
	if (!Actor || !Data.IsValidData())
	{
		return false;
	}

	//Hijack the component struct
	FComponentSaveData ComponentData;

	const FString FullId = FSaveHelpers::GetRawObjectID(Data);
	ComponentData.Name = BytesFromString(FullId);

	FStructHelpers::SerializeStruct(Data.Object);
	SerializeToBinary(Data.Object, ComponentData.Data);
	UpdateRawObjectData(Actor, ComponentData);

	return true;
}

void URPGSaveSubsystem::AppendRawObjectData(AActor* Actor, TArray<FComponentSaveData>& OutComponents) const
{
	if (Actor && !RawObjectData.IsEmpty())
	{
		if (const FGameObjectSaveData* GameObjectDataRef = RawObjectData.Find(Actor))
		{
			OutComponents.Append(GameObjectDataRef->Components);
		}
	}
}

void URPGSaveSubsystem::UpdateRawObjectData(AActor* Actor, const FComponentSaveData& InputData)
{
	if (!Actor)
	{
		return;
	}

	//Try to update global raw objects array
	FGameObjectSaveData& GameObjectData = RawObjectData.FindOrAdd(Actor);
	bool bDataUpdated = false;

	for (FComponentSaveData& ExistingData : GameObjectData.Components)
	{
		if (ExistingData.Name == InputData.Name)
		{
			ExistingData = InputData;
			bDataUpdated = true;
			break;
		}
	}

	//Add if not found
	if (!bDataUpdated)
	{
		GameObjectData.Components.Add(InputData);
	}
}

/**
Serialize Functions
**/

void URPGSaveSubsystem::ExecuteActorPreSave(AActor* Actor) const
{
	if (!HasSaveInterface(Actor))
	{
		return;
	}
	
	//Check for pre-save on game thread, can be useful for accessing timers or rendering properties etc.
	if (!IsInGameThread() && URPGSaveProjectSetting::Get()->bPreSaveOnGameThread)
	{
		FGraphEventRef GraphEvent = FFunctionGraphTask::CreateAndDispatchWhenReady([Actor]()
		{
			IRPGActorSaveInterface::Execute_ActorPreSave(Actor);

		}, TStatId(), nullptr, ENamedThreads::GameThread);

		//Wait for the event to complete
		if (GraphEvent.IsValid())
		{
			GraphEvent->Wait();
		}
	}
	else
	{
		IRPGActorSaveInterface::Execute_ActorPreSave(Actor);
	}
}

void URPGSaveSubsystem::ExecuteActorSaved(AActor* Actor) const
{
	if (HasSaveInterface(Actor))
	{
		IRPGActorSaveInterface::Execute_ActorSaved(Actor);
	}
}

void URPGSaveSubsystem::ExecuteActorPreLoad(AActor* Actor) const
{
	if (HasSaveInterface(Actor))
	{
		IRPGActorSaveInterface::Execute_ActorPreLoad(Actor);
	}
}

void URPGSaveSubsystem::ExecuteActorLoaded(AActor* Actor) const
{
	if (HasSaveInterface(Actor))
	{
		IRPGActorSaveInterface::Execute_ActorLoaded(Actor);
	}
}

void URPGSaveSubsystem::SaveActorToBinary(AActor* Actor, FGameObjectSaveData& OutData) const
{ 
	ExecuteActorPreSave(Actor);

	SerializeToBinary(Actor, OutData.Data);

	//Save components for non Level Scripts
	const EActorType Type = GetActorType(Actor);
	if (!IsLevelScript(Type))
	{
		SaveActorComponents(Actor, OutData.Components); 
	}

	//Store raw object data in Component array. 
	AppendRawObjectData(Actor, OutData.Components);

	//This prevents re-loading without 'Full Reload' directly after saving
	Actor->Tags.Add(RPGSave::HasLoadedTag);

	ExecuteActorSaved(Actor);
}

void URPGSaveSubsystem::LoadActorFromBinary(AActor* Actor, const FGameObjectSaveData& InData)
{
	ExecuteActorPreLoad(Actor);

	const EActorType Type = GetActorType(Actor);

	//For runtime Actors we need to parse the structs separately
	if (FActorHelpers::IsRuntime(Type))
	{
		SerializeActorStructProperties(Actor);
	}

	Actor->Tags.Add(RPGSave::HasLoadedTag);

	SerializeFromBinary(Actor, InData.Data);

	//Load components for non Level Scripts
	if (!IsLevelScript(Type))
	{
		LoadActorComponents(Actor, InData.Components);
	}

	//Post Component Load
	ExecuteActorLoaded(Actor);
}

/**
Blueprint Struct Property Serialization
**/

void URPGSaveSubsystem::SerializeActorStructProperties(AActor* Actor) const
{
	if (!URPGSaveProjectSetting::Get()->bAutoSaveStructs)
	{
		return;
	}

	FStructHelpers::SerializeStruct(Actor);

	//Also for Components
	const TArray<UActorComponent*> SourceComps = GetSaveComponents(Actor);
	if (RPGSave::ArrayEmpty(SourceComps))
	{
		return;
	}

	for (UActorComponent* Component : SourceComps)
	{
		if (Component)
		{
			FStructHelpers::SerializeStruct(Component);
		}
	}
}

/**
Helper Functions
**/

APlayerController* URPGSaveSubsystem::GetPlayerController() const
{
	return UGameplayStatics::GetPlayerController(GetWorld(), PlayerIndex);
}

APawn* URPGSaveSubsystem::GetPlayerPawn(const APlayerController* PC) const
{
	if (PC)
	{
		return PC->GetPawnOrSpectator();
	}

	return nullptr;
}

bool URPGSaveSubsystem::HasValidPlayer() const
{
	//Used by async check to wait for valid player pawn
	if (const APlayerController* PC = GetPlayerController())
	{
		return IsValid(GetPlayerPawn(PC));
	}

	return false;
}

bool URPGSaveSubsystem::HasValidGameMode() const
{
	//On clients, we assume the game mode is valid
	if (GetWorld()->GetNetMode() == ENetMode::NM_Client)
	{
		return true;
	}

	const AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	return IsValid(GameMode);
}

FTimerManager& URPGSaveSubsystem::GetTimerManager() const
{
	return GetWorld()->GetTimerManager();
}

/**
Actor Helper Functions
**/

FName URPGSaveSubsystem::LevelScriptSaveName(const AActor* Actor)
{
	//Compare by level name, since the engine creates multiple script actors.
	return FName(FActorHelpers::GetActorLevelName(Actor));
}

EActorType URPGSaveSubsystem::GetActorType(const AActor* Actor)
{
	return FActorHelpers::GetActorType(Actor);
}

FString URPGSaveSubsystem::GetFullActorName(const AActor* Actor)
{
	return FActorHelpers::GetFullActorName(Actor);
}

bool URPGSaveSubsystem::IsLoaded(const AActor* Actor)
{
	return FActorHelpers::IsLoaded(Actor);
}

bool URPGSaveSubsystem::IsSkipSave(const AActor* Actor)
{
	return FActorHelpers::IsSkipSave(Actor);
}

bool URPGSaveSubsystem::IsSkipTransform(const AActor* Actor)
{
	return FActorHelpers::IsSkipTransform(Actor);
}

bool URPGSaveSubsystem::IsLevelScript(const EActorType Type)
{
	return FActorHelpers::IsLevelScript(Type);
}

bool URPGSaveSubsystem::HasSaveInterface(const AActor* Actor) const
{
	return Actor->GetClass()->ImplementsInterface(URPGActorSaveInterface::StaticClass());
}

bool URPGSaveSubsystem::HasComponentSaveInterface(const UActorComponent* Comp) const
{
	return Comp && Comp->IsRegistered() && Comp->GetClass()->ImplementsInterface(URPGCompSaveInterface::StaticClass());
}

/**
World Delegates
**/

void URPGSaveSubsystem::OnPreWorldInit(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (!World)
	{
		return;
	}

	if (AutoSaveLoadWorldPartition(World))
	{
		FLevelStreamingDelegates::OnLevelStreamingStateChanged.AddUObject(this, &URPGSaveSubsystem::OnLevelStreamingStateChanged);

		if (URPGSaveProjectSetting::Get()->WorldPartitionSaving != EWorldPartitionMethod::LoadOnly)
		{
			FLevelStreamingDelegates::OnLevelBeginMakingInvisible.AddUObject(this, &URPGSaveSubsystem::OnLevelBeginMakingInvisible);
		}
	}

	if (AutoDestroyActors())
	{
		ActorDestroyedDelegate = World->AddOnActorDestroyedHandler(FOnActorDestroyed::FDelegate::CreateUObject(this, &URPGSaveSubsystem::OnAnyActorDestroyed));
	}
}

void URPGSaveSubsystem::OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	ClearWorldPartition();
	RemoveWorldPartitionStreamDelegates();

	ClearDestroyedActors();

	// [주석 해제 및 수정] 사용하시는 엔진 버전(UE 5.5 이상 여부)에 맞춰 하나만 남기거나 if문을 사용하세요.
	if (World)
	{
		World->RemoveOnActorDestroyedHandler(ActorDestroyedDelegate);
	}

	ClearLoadFromMemory();
}

/**
World Partition
**/

void URPGSaveSubsystem::RemoveWorldPartitionStreamDelegates()
{
	FLevelStreamingDelegates::OnLevelStreamingStateChanged.RemoveAll(this);
	FLevelStreamingDelegates::OnLevelBeginMakingInvisible.RemoveAll(this);
}

void URPGSaveSubsystem::OnLevelStreamingStateChanged(UWorld* InWorld, const ULevelStreaming* InStreamingLevel, ULevel* InLevelIfLoaded, ELevelStreamingState PreviousState, ELevelStreamingState NewState)
{
	if (!InWorld || !InLevelIfLoaded)
	{
		return;
	}

	if (NewState == ELevelStreamingState::LoadedVisible && PreviousState == ELevelStreamingState::MakingVisible)
	{
		//Optimized loading of placed Actors only
		if (AlwaysAutoLoadWorldPartition())
		{	
			URPGAsyncStream::InitStreamingLoadTask(this, InLevelIfLoaded);
			return;
		}
		
		//Skip initial loading and wait until the game was manually loaded
		if (!bInitWorldPartitionLoad)
		{
			if (SkipInitialWorldPartitionLoad())
			{
				return;
			}

			//New cell streamed in, unable to process all Actors. Edge case. @TODO May actually spawn a Stream Load Task here?
			if (IsInitialWorldPartitionLoading())
			{
				UE_LOG(LogRPGSave, Warning, TEXT("Level streamed in during initial World Partition load. Unable to parse all Actors. You might lock Player movement."));
				return;
			}
		}

		//Initial load will happen in a conventional way
		if (!bLoadPartition)
		{
			TryInitWorldPartition();
			bLoadPartition = true;
		}
	}
}

void URPGSaveSubsystem::TryInitWorldPartition()
{
	//If the loop hangs for some reason, we want to cancel it.
	if (AlwaysAutoLoadWorldPartition())
	{
		return;
	}

	//Check to see if we can start the initial loading process.
	FTimerHandle TimerHandle;
	const float PollingRate = URPGSaveProjectSetting::Get()->WorldPartitionInitPollingRate;
	GetTimerManager().SetTimer(TimerHandle, this, &URPGSaveSubsystem::PollInitWorldPartition, PollingRate, false);
}

void URPGSaveSubsystem::PollInitWorldPartition()
{
	if (CanProcessWorldPartition())
	{
		const float PollingRate = URPGSaveProjectSetting::Get()->WorldPartitionInitPollingRate;
		const float CheckTime = URPGSaveProjectSetting::Get()->WorldPartitionInitThreshold;
		WorldPartitionInitTimer += PollingRate;

		if (WorldPartitionInitTimer >= CheckTime)
		{
			URPGAsyncLoadGame::AutoLoadLevelActors(this);
			bLoadPartition = false;
			return;
		}
	}
	else
	{
		WorldPartitionInitTimer = 0.f;
	}

	TryInitWorldPartition();
}

void URPGSaveSubsystem::LoadStreamingActor(AActor* Actor, const FActorSaveData& ActorData)
{
	FScopeLock Lock(&LoadActorScope);

	if (!IsInGameThread())
	{
		AsyncTask(ENamedThreads::GameThread, [this, Actor, ActorData]()
		{
			SerializeActorStructProperties(Actor);
			ProcessLevelActor(Actor, ActorData);
		});
	}
	else
	{
		SerializeActorStructProperties(Actor);
		ProcessLevelActor(Actor, ActorData);
	}
}

void URPGSaveSubsystem::OnLevelBeginMakingInvisible(UWorld* InWorld, const ULevelStreaming* InStreamingLevel, ULevel* InLoadedLevel)
{
	if (!InWorld || !InLoadedLevel || FStreamHelpers::IsWorldPartitionInit(InWorld))
	{
		//Block on init
		return;
	}

	//We need to load at least once before we can auto-save in any way
	if (!bInitWorldPartitionLoad)
	{
		//Destroy load task to prevent it getting stuck 
		if (IsInitialWorldPartitionLoading())
		{
			FAsyncSaveHelpers::DestroyAsyncLoadLevelTask();
			UE_LOG(LogRPGSave, Warning, TEXT("Level streamed out during initial World Partition load. Async operation stopped."));		
			UE_LOG(LogRPGSave, Warning, TEXT("Please load manually or stream in another cell to continue."));
		}

		return;
	}

	//Check when a auto-loading process is still ongoing, otherwise it might write the unloaded data
	if (FAsyncSaveHelpers::IsStreamAutoLoadActive(InLoadedLevel))
	{
		FAsyncSaveHelpers::DestroyStreamAutoLoadTask(InLoadedLevel);
		UE_LOG(LogRPGSave, Warning, TEXT("Skipped saving partially loaded streaming level to prevent invalid data."));
		return;
	}

	TArray<FActorSaveData> InActors;
	InActors.Reserve(InLoadedLevel->Actors.Num());

	//Collect placed Actors from level. They don't need to be valid anymore when saving, the data is parsed here
	for (AActor* Actor : InLoadedLevel->Actors)
	{
		if (IsValidForSaving(Actor) && FActorHelpers::IsPlacedActor(Actor))
		{
			SerializeActorStructProperties(Actor);

			const FActorSaveData ActorArray = ParseLevelActorForSaving(Actor, EActorType::AT_Placed);
			InActors.Add(ActorArray);

			//This will prevent false pruning of Actors, for some reason Tags are kept when the Actor is streamed out
			Actor->Tags.Remove(RPGSave::HasLoadedTag);
		}
	}

	//We need to proceed, even if we only have Destroyed Actors
	if (!RPGSave::ArrayEmpty(InActors) || !RPGSave::ArrayEmpty(DestroyedActors))
	{
		WorldPartitionActors.Append(InActors);

		if (!bSavePartition)
		{
			TrySaveWorldPartition();
			bSavePartition = true;
		}
	}
}

void URPGSaveSubsystem::TrySaveWorldPartition()
{
	GetTimerManager().SetTimerForNextTick(this, &URPGSaveSubsystem::AccumulatedSaveWorldPartition);
}

void URPGSaveSubsystem::AccumulatedSaveWorldPartition()
{
	//Otherwise this could lead to an infinite loop when switching levels
	if (!bInitWorldPartitionLoad)
	{
		return;
	}

	//Wait for streaming and loading
	if (CanProcessWorldPartition())
	{
		URPGAsyncSaveGame::AutoSaveLevelActors(this);
		bSavePartition = false;
	}
	else
	{
		TrySaveWorldPartition();
	}
}
