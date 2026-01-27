// Fill out your copyright notice in the Description page of Project Settings.

// RPGSystem/Private/Input/RPGInputFunctionLibrary.cpp

#include "Input/RPGInputFunctionLibrary.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

bool URPGInputFunctionLibrary::RebindKey(const UObject* WorldContextObject, UInputMappingContext* Context, const UInputAction* Action, FKey OldKey, FKey NewKey)
{
	if (!Context || !Action) return false;

	FEnhancedActionKeyMapping OldMappingData;
	bool bFound = false;
	
	for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
	{
		if (Mapping.Action == Action && Mapping.Key == OldKey)
		{
			OldMappingData = Mapping;
			bFound = true;
			break;
		}
	}

	if (bFound)
	{
		// 기존 매핑 삭제 (Unmap)
		Context->UnmapKey(Action, OldKey);

		// 새로운 키로 매핑 추가 (MapKey)
		FEnhancedActionKeyMapping& NewMapping = Context->MapKey(Action, NewKey);

		// 아까 복사해둔 설정(Modifiers, Triggers)을 새 매핑에 덮어씌웁니다.
		NewMapping.Modifiers = OldMappingData.Modifiers;
		NewMapping.Triggers = OldMappingData.Triggers;
		// 필요한 경우 SettingBehavior 등 다른 속성도 복사
        
		// 5. 서브시스템 갱신 (즉시 반영)
		// 변경사항을 플레이어에게 적용하기 위해 Context를 뺐다가 다시 넣거나 Rebuild
		if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
		{
			if (APlayerController* PC = World->GetFirstPlayerController())
			{
				if (ULocalPlayer* LP = PC->GetLocalPlayer())
				{
					if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
					{
						if (Subsystem->HasMappingContext(Context))
						{
							// 가장 확실한 갱신 방법: Remove -> Add
							// (옵션으로 FModifyContextOptions::ForceRebuild를 쓸 수도 있지만, 버전마다 다를 수 있음)
							Subsystem->RemoveMappingContext(Context);
							Subsystem->AddMappingContext(Context, 0); // 우선순위는 기존 로직에 맞춰 설정
						}
					}
				}
			}
		}
        
		return true;
	}

	return false;
}

FKey URPGInputFunctionLibrary::GetKeyForAction(const UInputMappingContext* Context, const UInputAction* Action)
{
	if (!Context || !Action) return EKeys::Invalid;

	for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
	{
		if (Mapping.Action == Action)
		{
			return Mapping.Key;
		}
	}

	return EKeys::Invalid;
}

TArray<FKey> URPGInputFunctionLibrary::GetAllKeysForAction(const UInputMappingContext* Context, const UInputAction* Action)
{
	TArray<FKey> ResultKeys;
	if (!Context || !Action) return ResultKeys;

	for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
	{
		if (Mapping.Action == Action)
		{
			ResultKeys.Add(Mapping.Key);
		}
	}

	return ResultKeys;
}

bool URPGInputFunctionLibrary::IsKeyMapped(const UInputMappingContext* Context, FKey Key)
{
	if (!Context) return false;

	// 수정 4: GetMappings() 사용
	for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
	{
		if (Mapping.Key == Key)
		{
			return true;
		}
	}
	return false;
}

void URPGInputFunctionLibrary::UnbindAction(const UObject* WorldContextObject, UInputMappingContext* Context, const UInputAction* Action, FKey KeyToRemove)
{
	if (!Context || !Action) return;

	if (KeyToRemove.IsValid())
	{
		// 특정 키만 삭제 (예: Move에서 'W'만 삭제)
		Context->UnmapKey(Action, KeyToRemove);
	}
	else
	{
		// 해당 액션의 모든 키 삭제 (예: Jump에 연결된 Space, GamepadFaceBottom 모두 삭제)
		Context->UnmapAction(Action);
	}

	// 변경 사항 적용 (서브시스템 갱신)
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (APlayerController* PC = World->GetFirstPlayerController())
		{
			if (ULocalPlayer* LP = PC->GetLocalPlayer())
			{
				if (UEnhancedInputLocalPlayerSubsystem* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
				{
					if (Sub->HasMappingContext(Context))
					{
						Sub->RemoveMappingContext(Context);
						Sub->AddMappingContext(Context, 0);
					}
				}
			}
		}
	}
}