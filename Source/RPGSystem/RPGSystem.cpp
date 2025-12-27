// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGSystem.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "ProjectSettings/RPGSaveProjectSetting.h"

// FDefaultGameModuleImpl 대신 FDefaultModuleImpl을 사용할 수도 있습니다.
class FRPGSystemModule : public FDefaultGameModuleImpl
{
public:
    // 모듈 시작 시 호출
    virtual void StartupModule() override
    {
        RegisterSettings();
    }

    // 모듈 종료 시 호출
    virtual void ShutdownModule() override
    {
        UnregisterSettings();
    }

private:
    void RegisterSettings()
    {
        // Settings 모듈이 로드 가능한지 확인
        if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
        {
            // 등록할 세팅 오브젝트를 가져옵니다.
            URPGSaveProjectSetting* GameSettings = GetMutableDefault<URPGSaveProjectSetting>();

            // 프로젝트 세팅에 등록합니다.
            SettingsModule->RegisterSettings("Project", "Game", "SaveLoad Settings",
                FText::FromString("SaveLoad Settings"), // 표시될 이름
                FText::FromString("About SaveLoad Variables"), // 설명
                GameSettings
            );
        }
    }

    void UnregisterSettings()
    {
        // Settings 모듈이 로드 가능한지 확인
        if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
        {
            // 등록했던 세팅을 해제합니다.
            SettingsModule->UnregisterSettings("Project", "Game", "My Custom Settings");
        }
    }
};

// IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, RPGSystem, "RPGSystem" );

IMPLEMENT_PRIMARY_GAME_MODULE( FRPGSystemModule, RPGSystem, "RPGSystem" );
 