#include "UI/Dialogue/RPGDialogueRootWidget.h"

#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "UI/Dialogue/RPGDialogueOptionEntryWidget.h"
#include "UI/DialogueViewModel.h"

void URPGDialogueRootWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindViewModel();
}

void URPGDialogueRootWidget::NativeDestruct()
{
	UnbindViewModel();
	Super::NativeDestruct();
}

void URPGDialogueRootWidget::BindViewModel()
{
	if (bBound || !ViewModel)
	{
		return;
	}

	ViewModel->OnDialogueOpened.AddDynamic(this, &URPGDialogueRootWidget::HandleDialogueOpened);
	ViewModel->OnDialogueClosed.AddDynamic(this, &URPGDialogueRootWidget::HandleDialogueClosed);
	ViewModel->OnSpeakerChanged.AddDynamic(this, &URPGDialogueRootWidget::HandleSpeakerChanged);
	ViewModel->OnRuntimeOptionsUpdated.AddDynamic(this, &URPGDialogueRootWidget::HandleOptionsUpdated);
	ViewModel->OnHistoryAdded.AddDynamic(this, &URPGDialogueRootWidget::HandleHistoryAdded);
	ViewModel->OnCinematicCueRequested.AddDynamic(this, &URPGDialogueRootWidget::HandleCinematicCue);
	bBound = true;
}

void URPGDialogueRootWidget::UnbindViewModel()
{
	if (!bBound || !ViewModel)
	{
		return;
	}

	ViewModel->OnDialogueOpened.RemoveDynamic(this, &URPGDialogueRootWidget::HandleDialogueOpened);
	ViewModel->OnDialogueClosed.RemoveDynamic(this, &URPGDialogueRootWidget::HandleDialogueClosed);
	ViewModel->OnSpeakerChanged.RemoveDynamic(this, &URPGDialogueRootWidget::HandleSpeakerChanged);
	ViewModel->OnRuntimeOptionsUpdated.RemoveDynamic(this, &URPGDialogueRootWidget::HandleOptionsUpdated);
	ViewModel->OnHistoryAdded.RemoveDynamic(this, &URPGDialogueRootWidget::HandleHistoryAdded);
	ViewModel->OnCinematicCueRequested.RemoveDynamic(this, &URPGDialogueRootWidget::HandleCinematicCue);
	bBound = false;
}

void URPGDialogueRootWidget::HandleDialogueOpened()
{
	BP_OnDialogueOpened();
}

void URPGDialogueRootWidget::HandleDialogueClosed(bool bCompleted)
{
	if (VB_Options)
	{
		VB_Options->ClearChildren();
	}

	BP_OnDialogueClosed(bCompleted);
}

void URPGDialogueRootWidget::HandleSpeakerChanged(const FDialogueSpeakerPayload& Payload)
{
	if (Txt_SpeakerName)
	{
		Txt_SpeakerName->SetText(Payload.SpeakerName);
	}

	if (RTB_CurrentDialogue)
	{
		RTB_CurrentDialogue->SetText(Payload.Node.Text);
	}

	if (Img_Portrait)
	{
		Img_Portrait->SetBrushFromTexture(Payload.Portrait);
	}

	BP_OnSpeakerChanged(Payload);
}

void URPGDialogueRootWidget::HandleOptionsUpdated(const TArray<FDialogueRuntimeOption>& Options)
{
	if (VB_Options)
	{
		VB_Options->ClearChildren();

		if (OptionEntryClass)
		{
			for (const FDialogueRuntimeOption& Option : Options)
			{
				URPGDialogueOptionEntryWidget* Entry = CreateWidget<URPGDialogueOptionEntryWidget>(GetOwningPlayer(), OptionEntryClass);
				if (Entry)
				{
					Entry->SetupOption(Option, ViewModel);
					VB_Options->AddChild(Entry);
				}
			}
		}
	}

	BP_OnOptionsUpdated(Options);
}

void URPGDialogueRootWidget::HandleHistoryAdded(const FDialogueHistoryEntryEx& Entry)
{
	BP_OnHistoryAdded(Entry);
}

void URPGDialogueRootWidget::HandleCinematicCue(const FDialogueCinematicCue& Cue)
{
	BP_OnCinematicCue(Cue);
}
