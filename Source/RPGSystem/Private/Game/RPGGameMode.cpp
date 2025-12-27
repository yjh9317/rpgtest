// RPGSystem/Private/Game/RPGGameMode.cpp

#include "Game/RPGGameMode.h"
#include "SaveSystem/Subsystem/RPGSaveSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "SaveSystem/RPGSaveFunctionLibrary.h"
#include "SaveSystem/Async/RPGAsyncLoadGame.h"

void ARPGGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorld()->GetTimerManager().SetTimer(TestTimerHandle,this,&ARPGGameMode::TestFunc,0.1f,false);
	
}

void ARPGGameMode::TestFunc()
{
	if (URPGSaveFunctionLibrary::IsWorldPartition(this))
	{
		int32 SaveFlags = (1 << (int32)ESaveTypeFlags::SF_Player);
		URPGAsyncLoadGame::AsyncLoadActors(this,SaveFlags,false);
		LoadCustoms();
	}
	else
	{
		int32 SaveFlags = (1 << (int32)ESaveTypeFlags::SF_Level) | (1 << (int32)ESaveTypeFlags::SF_Player);
		URPGAsyncLoadGame::AsyncLoadActors(this,SaveFlags,false);
		LoadCustoms();
	}
}

