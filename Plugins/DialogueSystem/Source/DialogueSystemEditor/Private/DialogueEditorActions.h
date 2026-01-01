#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Styling/AppStyle.h"

class FDialogueEditorCommandsImpl : public TCommands<FDialogueEditorCommandsImpl>
{
public:

	FDialogueEditorCommandsImpl()
		: TCommands<FDialogueEditorCommandsImpl>(TEXT("DialogueEditor"), NSLOCTEXT("Contexts", "DialogueEditor", "Dialogue Editor"), NAME_None, FAppStyle::GetAppStyleSetName())
	{
	}

	virtual ~FDialogueEditorCommandsImpl()
	{
	}

	DIALOGUESYSTEMEDITOR_API virtual void RegisterCommands() override;

	TSharedPtr< FUICommandInfo > TestCommand;
};

class DIALOGUESYSTEMEDITOR_API FDialogueEditorCommands
{
public:
	static void Register();

	static const FDialogueEditorCommandsImpl& Get();

	static void Unregister();
};
