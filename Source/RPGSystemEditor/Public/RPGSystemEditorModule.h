// Source/RPGSystemEditor/Public/RPGSystemEditorModule.h
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * RPGSystem 에디터 전용 모듈
 * 에디터 툴, 커스터마이징, 검증 기능 제공
 */
class FRPGSystemEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface 구현 */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterTabSpawners();
	void UnregisterTabSpawners();
	/** 탭 스포너 콜백 */
	TSharedRef<class SDockTab> SpawnLocomotionDebuggerTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnItemDebuggerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<SDockTab> SpawnActionDebuggerTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedRef<class SDockTab> SpawnStatsDebuggerTab(const class FSpawnTabArgs& SpawnTabArgs);     
	TSharedRef<class SDockTab> SpawnQuestDebuggerTab(const class FSpawnTabArgs& SpawnTabArgs);     
	TSharedRef<class SDockTab> SpawnInventoryDebuggerTab(const class FSpawnTabArgs& SpawnTabArgs); 
private:
	void SelectActorAtViewportCenter();
	TSharedPtr<FUICommandList> PluginCommands;
};