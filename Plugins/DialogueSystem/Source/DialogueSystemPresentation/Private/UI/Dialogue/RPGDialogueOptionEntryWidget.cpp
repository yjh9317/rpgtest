#include "UI/Dialogue/RPGDialogueOptionEntryWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "UI/DialogueViewModel.h"

void URPGDialogueOptionEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Select)
	{
		Btn_Select->OnClicked.AddDynamic(this, &URPGDialogueOptionEntryWidget::OnClicked);
		Btn_Select->OnHovered.AddDynamic(this, &URPGDialogueOptionEntryWidget::OnHovered);
		Btn_Select->OnUnhovered.AddDynamic(this, &URPGDialogueOptionEntryWidget::OnUnhovered);
	}
}

void URPGDialogueOptionEntryWidget::SetupOption(const FDialogueRuntimeOption& InOption, UDialogueViewModel* InViewModel)
{
	RuntimeOption = InOption;
	ViewModel = InViewModel;

	if (Txt_Index)
	{
		Txt_Index->SetText(FText::AsNumber(RuntimeOption.OptionIndex));
	}

	if (RTB_OptionText)
	{
		const FText FinalText = RuntimeOption.DecoratedPrefix.IsEmpty()
			? RuntimeOption.DisplayText
			: FText::Format(
				NSLOCTEXT("Dialogue", "OptionFormat", "{0}{1}"),
				RuntimeOption.DecoratedPrefix,
				RuntimeOption.DisplayText);

		RTB_OptionText->SetText(FinalText);
	}

	if (Txt_UnavailableReason)
	{
		Txt_UnavailableReason->SetText(RuntimeOption.UnavailableReason);
		Txt_UnavailableReason->SetVisibility(RuntimeOption.bAvailable ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	if (Img_Locked)
	{
		Img_Locked->SetVisibility(RuntimeOption.bAvailable ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}

	if (Img_SkillCheck)
	{
		Img_SkillCheck->SetVisibility(RuntimeOption.Meta.bHasSkillCheck ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (Btn_Select)
	{
		Btn_Select->SetIsEnabled(RuntimeOption.bSelectable);
	}

	BP_ApplyOptionVisual(RuntimeOption, false);
}

void URPGDialogueOptionEntryWidget::OnClicked()
{
	if (ViewModel && RuntimeOption.bSelectable)
	{
		ViewModel->SelectOption(RuntimeOption.Node.id);
	}
}

void URPGDialogueOptionEntryWidget::OnHovered()
{
	BP_ApplyOptionVisual(RuntimeOption, true);
}

void URPGDialogueOptionEntryWidget::OnUnhovered()
{
	BP_ApplyOptionVisual(RuntimeOption, false);
}
