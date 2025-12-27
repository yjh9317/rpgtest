#include "Commands/RPGEditorCommands.h" // 경로 변경 반영

#define LOCTEXT_NAMESPACE "FRPGEditorCommands"

void FRPGEditorCommands::RegisterCommands()
{
	UI_COMMAND(SelectCenterActor, "Select Center Actor", "Select the actor at the center of the viewport", EUserInterfaceActionType::Button, 
			FInputChord(EKeys::One, EModifierKey::Control | EModifierKey::Shift));
}

#undef LOCTEXT_NAMESPACE