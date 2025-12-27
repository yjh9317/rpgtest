#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "EditorStyleSet.h"

class FRPGEditorCommands : public TCommands<FRPGEditorCommands>
{
public:
	FRPGEditorCommands()
		: TCommands<FRPGEditorCommands>(
			TEXT("RPGSystemEditor"), 
			NSLOCTEXT("Contexts", "RPGSystemEditor", "RPG System Editor"), 
			NAME_None, 
			FAppStyle::GetAppStyleSetName()
		)
	{}

	// 커맨드 등록
	virtual void RegisterCommands() override;

public:
	// 화면 중앙 액터 선택 커맨드
	TSharedPtr<FUICommandInfo> SelectCenterActor;
};