#include "RPGSystemEditorModule.h"

// 에디터/엔진 관련 헤더
#include "LevelEditor.h"
#include "LevelEditorViewport.h"

#include "Debugger/SLocomotionDebugger.h"
#include "Debugger/SItemDebugger.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Commands/RPGEditorCommands.h"
#include "Debugger/SActionDebugger.h"
#include "Debugger/SInventoryDebugger.h"
#include "Debugger/SQuestDebugger.h"
#include "Debugger/SStatsDebugger.h"

static const FName LocomotionDebuggerTabName("LocomotionDebugger");
static const FName ItemDebuggerTabName("ItemDebugger");
static const FName ActionDebuggerTabName("ActionDebugger");
static const FName StatsDebuggerTabName("StatsDebugger");         
static const FName QuestDebuggerTabName("QuestDebugger");         
static const FName InventoryDebuggerTabName("InventoryDebugger"); 

void FRPGSystemEditorModule::StartupModule()
{
	FRPGEditorCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList);

	// 2. 액션 매핑 (함수 연결)
	PluginCommands->MapAction(
		FRPGEditorCommands::Get().SelectCenterActor,
		FExecuteAction::CreateRaw(this, &FRPGSystemEditorModule::SelectActorAtViewportCenter),
		FCanExecuteAction()
	);

	// 3. 레벨 에디터에 커맨드 추가 (전역 바인딩)
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetGlobalLevelEditorActions()->Append(PluginCommands.ToSharedRef());
	
	RegisterTabSpawners();
}

void FRPGSystemEditorModule::ShutdownModule()
{
	UnregisterTabSpawners();
	FRPGEditorCommands::Unregister();
}

void FRPGSystemEditorModule::RegisterTabSpawners()
{
	auto& TabManager = FGlobalTabmanager::Get();
	auto ToolsCategory = WorkspaceMenu::GetMenuStructure().GetToolsCategory();

	// 1. Locomotion
	TabManager->RegisterNomadTabSpawner(
		LocomotionDebuggerTabName,
		FOnSpawnTab::CreateRaw(this, &FRPGSystemEditorModule::SpawnLocomotionDebuggerTab)
	)
	.SetDisplayName(FText::FromString("Locomotion Debugger"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(ToolsCategory);

	// 2. Item
	TabManager->RegisterNomadTabSpawner(
		ItemDebuggerTabName,
		FOnSpawnTab::CreateRaw(this, &FRPGSystemEditorModule::SpawnItemDebuggerTab)
	)
	.SetDisplayName(FText::FromString("Item Inspector"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(ToolsCategory);

	// 3. Action
	TabManager->RegisterNomadTabSpawner(
		ActionDebuggerTabName,
		FOnSpawnTab::CreateRaw(this, &FRPGSystemEditorModule::SpawnActionDebuggerTab)
	)
	.SetDisplayName(FText::FromString("Action System Debugger"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(ToolsCategory);

	// 4. Stats [New]
	TabManager->RegisterNomadTabSpawner(
		StatsDebuggerTabName,
		FOnSpawnTab::CreateRaw(this, &FRPGSystemEditorModule::SpawnStatsDebuggerTab)
	)
	.SetDisplayName(FText::FromString("Stats Inspector"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(ToolsCategory);

	// 5. Quest [New]
	TabManager->RegisterNomadTabSpawner(
		QuestDebuggerTabName,
		FOnSpawnTab::CreateRaw(this, &FRPGSystemEditorModule::SpawnQuestDebuggerTab)
	)
	.SetDisplayName(FText::FromString("Quest Debugger"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(ToolsCategory);

	// 6. Inventory [New]
	TabManager->RegisterNomadTabSpawner(
		InventoryDebuggerTabName,
		FOnSpawnTab::CreateRaw(this, &FRPGSystemEditorModule::SpawnInventoryDebuggerTab)
	)
	.SetDisplayName(FText::FromString("Inventory Inspector"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetGroup(ToolsCategory);
}

void FRPGSystemEditorModule::UnregisterTabSpawners()
{
	if (FSlateApplication::IsInitialized())
	{
		auto& TabManager = FGlobalTabmanager::Get();
		TabManager->UnregisterNomadTabSpawner(LocomotionDebuggerTabName);
		TabManager->UnregisterNomadTabSpawner(ItemDebuggerTabName);
		TabManager->UnregisterNomadTabSpawner(ActionDebuggerTabName);
		TabManager->UnregisterNomadTabSpawner(StatsDebuggerTabName);     // [New]
		TabManager->UnregisterNomadTabSpawner(QuestDebuggerTabName);     // [New]
		TabManager->UnregisterNomadTabSpawner(InventoryDebuggerTabName); // [New]
	}
}

TSharedRef<SDockTab> FRPGSystemEditorModule::SpawnLocomotionDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[ SNew(SLocomotionDebugger) ];
}

TSharedRef<SDockTab> FRPGSystemEditorModule::SpawnItemDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[ SNew(SItemDebugger) ];
}

TSharedRef<SDockTab> FRPGSystemEditorModule::SpawnActionDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[ SNew(SActionDebugger) ];
}

// [New]
TSharedRef<SDockTab> FRPGSystemEditorModule::SpawnStatsDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[ SNew(SStatsDebugger) ];
}

// [New]
TSharedRef<SDockTab> FRPGSystemEditorModule::SpawnQuestDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[ SNew(SQuestDebugger) ];
}

// [New]
TSharedRef<SDockTab> FRPGSystemEditorModule::SpawnInventoryDebuggerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)[ SNew(SInventoryDebugger) ];
}

void FRPGSystemEditorModule::SelectActorAtViewportCenter()
{
	FLevelEditorViewportClient* ViewportClient = GCurrentLevelEditingViewportClient;
	if (!ViewportClient) return;

	FVector CamLoc = ViewportClient->GetViewLocation();
	FRotator CamRot = ViewportClient->GetViewRotation();
	FVector CamDir = CamRot.Vector();

	FVector Start = CamLoc;
	FVector End = CamLoc + (CamDir * 100000.0f); // 1000m

	UWorld* World = ViewportClient->GetWorld();
	if (!World) return;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true; // 정밀 검사

	if (World->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			// 기존 선택 모두 해제 (하나만 딱 선택되게)
			GEditor->SelectNone(true, true);
			// 액터 선택 수행
			GEditor->SelectActor(HitActor, true, true);
			GEditor->NoteSelectionChange();

			FString Msg = FString::Printf(TEXT("Selected: %s"), *HitActor->GetActorLabel());
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, Msg);
            
			UE_LOG(LogTemp, Log, TEXT(">> [RPGEditor] Hit & Selected: %s (Class: %s)"), 
				*HitActor->GetActorLabel(), *HitActor->GetClass()->GetName());
		}
	}
	else
	{
		// 허공을 찍었을 때
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("No Hit!"));
	}
}

IMPLEMENT_MODULE(FRPGSystemEditorModule, RPGSystemEditor);
