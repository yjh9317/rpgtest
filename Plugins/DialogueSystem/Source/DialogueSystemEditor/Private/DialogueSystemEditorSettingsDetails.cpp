#include "DialogueSystemEditorSettingsDetails.h"
#include "DialogueSystemEditorPrivatePCH.h"
#include "Dialogue.h"
#include "DialogueViewportWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Editor/PropertyEditor/Public/DetailLayoutBuilder.h"
#include "Editor/PropertyEditor/Public/DetailCategoryBuilder.h"
#include "Editor/UnrealEd/Public/ScopedTransaction.h"
#include "Internationalization/Text.h"
#include "SSimpleComboButton.h"
#include "Styling/StyleColors.h"
#include "STextPropertyEditableTextBox.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SCheckBox.h"
//#include "TextCustomization.cpp"
//#include "STextPropertyEditableTextBox.h"
#include "EditableTextPropertyHandle.h"

#define LOCTEXT_NAMESPACE "DialogueSystemSettingsDetails"

TSharedRef<IDetailCustomization> FDialogueSystemEditorSettingsDetails::MakeInstance()
{
	return MakeShareable(new FDialogueSystemEditorSettingsDetails());
}

void FDialogueSystemEditorSettingsDetails::CustomizeDetails( IDetailLayoutBuilder& DetailLayout )
{
	using namespace DialogueSystemEd;

	DetailLayoutBuilder = &DetailLayout;

	const TSharedPtr<IPropertyHandle> DataProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogue, Data));
	DetailLayout.HideProperty(DataProperty);
	const TSharedPtr<IPropertyHandle> NextNodeProperty = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UDialogue, NextNodeId));
	DetailLayout.HideProperty(NextNodeProperty);

	// Create a category so this is displayed early in the properties
	IDetailCategoryBuilder& MyCategory = DetailLayout.EditCategory("Dialogue Editor");
	IDetailCategoryBuilder& CurrentNodeCategory = DetailLayout.EditCategory("Current Node", LOCTEXT("CurrentNode", "Current Node"), ECategoryPriority::Important);

	TArray<TWeakObjectPtr<UObject>> ObjectsBeingCustomized;

	DetailLayout.GetObjectsBeingCustomized(ObjectsBeingCustomized);
	TArray<UObject*> StrongObjects;
	CopyFromWeakArray(StrongObjects, ObjectsBeingCustomized);

	if (StrongObjects.Num() == 0) return;

	UDialogue* Dialogue = Cast<UDialogue>(StrongObjects[0]);
	
	if (Dialogue->CurrentNodeId != -1 && Dialogue->CurrentNodeId != 0) //display current node details:
	{
		int32 index;
		FDialogueNode CurrentNode = Dialogue->GetNodeById(Dialogue->CurrentNodeId, index);

		const TSharedPtr<IPropertyHandleArray> Array = DataProperty->AsArray();
		const TSharedPtr<IPropertyHandle> Child = Array->GetElement(index);
		const TSharedPtr<IPropertyHandle> AltText = Child->GetChildHandle("Text");
		AltText->GetMetaDataProperty()->SetMetaData("MultiLine", "true");
		
		TSharedRef<FEditableTextPropertyHandle> EditableTextSharedRef = MakeShareable(new FEditableTextPropertyHandle(AltText.ToSharedRef(), DetailLayout.GetPropertyUtilities()));
		//const TSharedPtr<FEditableTextPropertyHandle> EditableTextSharedPtr(EditableTextSharedRef);
		
		CurrentNodeCategory.AddCustomRow(LOCTEXT("Text", "Text"))
		.WholeRowContent()
		.VAlign(VAlign_Center)
		[
			SNew(SHorizontalBox) 
			+ SHorizontalBox::Slot().HAlign(HAlign_Left)
			// Just a row that says "Text (node id: ...)"
			[
				SNew(STextBlock).Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(FText::Format(LOCTEXT("TextRowWithId", "Text (node id: {0})"), Dialogue->CurrentNodeId)) // displays current Node Id in inspector
			]		
			+ SHorizontalBox::Slot().HAlign(HAlign_Left)
			[
				SNew(SBox)				
				// An invisible slot with an STextPropertyEditableTextBox
				// The reason we don't use it as is, is there's no way to make it 100 pixels high
				// But we need a reference to it to build an alternative widget that looks like what we want
				// So we build one, but make it invisible... hacky, but works alright
				.Visibility(EVisibility::Collapsed)
				[
					SNew(STextPropertyEditableTextBox, EditableTextSharedRef)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
					.AutoWrapText(true)
				]
			]
		];

		// Now we build a multiline textbox and the difficult part is, we add a localization button that we build from scratch
		const TSharedRef<FLinkedBoxManager> LinkedBoxManager = MakeShared<FLinkedBoxManager>();
		const FSlateFontInfo PropertyNormalFont = FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont");
		CurrentNodeCategory.AddCustomRow(LOCTEXT("TextValue", "TextValue"))
		.WholeRowContent()
		[
			// this is the text box, it's not complicated:
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.HeightOverride(100)
				[
					SNew(SMultiLineEditableTextBox).Text(CurrentNode.Text)
					.AutoWrapText(true)
					.OnTextCommitted(this, &FDialogueSystemEditorSettingsDetails::TextCommited, Dialogue, Dialogue->CurrentNodeId)
					.ModiferKeyForNewLine(EModifierKey::Shift)
					.Font(FAppStyle::GetFontStyle("PropertyWindow.NormalFont"))
				]
			]
			// this is the localization button, it's a lot of code copied from STextPropertyEditableTextBox.cpp from line 807:
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SSimpleComboButton)
				.Icon(EditableTextSharedRef, &FEditableTextPropertyHandle::GetAdvancedTextSettingsComboImage)
				.MenuContent()
				[
					SNew(SBox)
					.WidthOverride(340)
					.Padding(1)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						[
							SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
							.Text(LOCTEXT("TextLocalizableLabel", "Localize"))
							.ToolTipText(LOCTEXT("TextLocalizableCheckBoxToolTip", "Whether to assign this text a key and allow it to be gathered for localization.\nIf set to false, marks this text as 'culture invariant' to prevent it being gathered for localization."))
							.ContentHAlign(HAlign_Left)
							[
								SNew(SCheckBox)
								/* The next three properties come from STextPropertyEditableTextBox::Construct (lines 824-826) */
								.IsEnabled(EditableTextSharedRef, &FEditableTextPropertyHandle::IsCultureInvariantFlagEnabled)
								.IsChecked(EditableTextSharedRef, &FEditableTextPropertyHandle::GetLocalizableCheckState)
								.OnCheckStateChanged(EditableTextSharedRef, &FEditableTextPropertyHandle::HandleLocalizableCheckStateChanged)
								//.IsChecked(TAttribute<ECheckBoxState>::Create([EditableTextSharedPtr]() -> ECheckBoxState { ...								
								//.OnCheckStateChanged_Lambda([EditableTextSharedPtr](ECheckBoxState InCheckBoxState) { ...
							]
						]
						+ SVerticalBox::Slot()
						[
							SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
							.IsHeader(true)
							.Text(LOCTEXT("TextReferencedTextLabel", "Referenced Text"))
						]
						+ SVerticalBox::Slot()
						[
							SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
							.Text(LOCTEXT("TextStringTableLabel", "String Table"))
							.IsEnabled(EditableTextSharedRef, &FEditableTextPropertyHandle::IsTextLocalizable)
							[
								SNew(STextPropertyEditableStringTableReference, EditableTextSharedRef)
								.AllowUnlink(true)
								.Font(PropertyNormalFont)
								.IsEnabled(EditableTextSharedRef, &FEditableTextPropertyHandle::CanEdit)
							]
						]
						+ SVerticalBox::Slot()
						[
							SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
							.IsHeader(true)
							.Text(LOCTEXT("TextInlineTextLabel", "Inline Text"))
						]
#if USE_STABLE_LOCALIZATION_KEYS
						+ SVerticalBox::Slot()
						[
							SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
							.Text(LOCTEXT("TextPackageLabel", "Package"))
							.IsEnabled(EditableTextSharedRef, &FEditableTextPropertyHandle::IsTextLocalizable)
							[
								SNew(SEditableTextBox)
								.Text(EditableTextSharedRef, &FEditableTextPropertyHandle::GetPackageValue)
								.Font(PropertyNormalFont)
								.IsReadOnly(true)
							]
						]
#endif // USE_STABLE_LOCALIZATION_KEYS
						+ SVerticalBox::Slot()
						[
							SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
							.Text(LOCTEXT("TextNamespaceLabel", "Namespace"))
							.IsEnabled(EditableTextSharedRef, &FEditableTextPropertyHandle::IsTextLocalizable)
							[
								SAssignNew(EditableTextSharedRef->NamespaceEditableTextBox, SEditableTextBox)
								.Text(EditableTextSharedRef, &FEditableTextPropertyHandle::GetNamespaceValue)
								.Font(PropertyNormalFont)
								.SelectAllTextWhenFocused(true)
								.ClearKeyboardFocusOnCommit(false)
								.OnTextChanged(EditableTextSharedRef, &FEditableTextPropertyHandle::OnNamespaceChanged)
								.OnTextCommitted(EditableTextSharedRef, &FEditableTextPropertyHandle::OnNamespaceCommitted)
								.SelectAllTextOnCommit(true)
								.IsReadOnly(EditableTextSharedRef, &FEditableTextPropertyHandle::IsIdentityReadOnly)
							]
						]
						+ SVerticalBox::Slot()
						[
							SNew(STextPropertyEditableOptionRow, LinkedBoxManager)
							.Text(LOCTEXT("TextKeyLabel", "Key"))
							.IsEnabled(EditableTextSharedRef, &FEditableTextPropertyHandle::IsTextLocalizable)
							[
								SAssignNew(EditableTextSharedRef->KeyEditableTextBox, SEditableTextBox)
								.Text(EditableTextSharedRef, &FEditableTextPropertyHandle::GetKeyValue)
								.Font(PropertyNormalFont)
#if USE_STABLE_LOCALIZATION_KEYS
								.SelectAllTextWhenFocused(true)
								.ClearKeyboardFocusOnCommit(false)
								.OnTextChanged(EditableTextSharedRef, &FEditableTextPropertyHandle::OnKeyChanged)
								.OnTextCommitted(EditableTextSharedRef, &FEditableTextPropertyHandle::OnKeyCommitted)
								.SelectAllTextOnCommit(true)
								.IsReadOnly(EditableTextSharedRef, &FEditableTextPropertyHandle::IsIdentityReadOnly)
#else	// USE_STABLE_LOCALIZATION_KEYS
								.IsReadOnly(true)
#endif	// USE_STABLE_LOCALIZATION_KEYS
							]
						]
					]
				]
			]
		];

		const TSharedPtr<IPropertyHandle> IsPlayerField = Child->GetChildHandle("isPlayer");
		const TSharedPtr<IPropertyHandle> DrawCommentBubble = Child->GetChildHandle("bDrawBubbleComment");
		const TSharedPtr<IPropertyHandle> Comment = Child->GetChildHandle("BubbleComment");
		const TSharedPtr<IPropertyHandle> EventsField = Child->GetChildHandle("Events");
		const TSharedPtr<IPropertyHandle> ConditionsField = Child->GetChildHandle("Conditions");
		const TSharedPtr<IPropertyHandle> SoundField = Child->GetChildHandle("Sound");
		const TSharedPtr<IPropertyHandle> DialogueWaveField = Child->GetChildHandle("DialogueWave");
		
		CurrentNodeCategory.AddProperty(IsPlayerField);
		CurrentNodeCategory.AddProperty(DrawCommentBubble);
		CurrentNodeCategory.AddProperty(Comment);
		CurrentNodeCategory.AddProperty(EventsField);
		CurrentNodeCategory.AddProperty(ConditionsField);		
		CurrentNodeCategory.AddProperty(SoundField);
		CurrentNodeCategory.AddProperty(DialogueWaveField);		
	}	
	
}

void FDialogueSystemEditorSettingsDetails::TextCommited(const FText& NewText, ETextCommit::Type CommitInfo, UDialogue* Dialogue, int32 id)
{
	int32 index;
	FDialogueNode CurrentNode = Dialogue->GetNodeById(id, index);

	// we don't commit text if it hasn't changed
	if (Dialogue->Data[index].Text.EqualTo(NewText))
	{
		return;
	}
	
	const FScopedTransaction Transaction(LOCTEXT("TextCommited", "Edited Node Text"));
	Dialogue->Modify();
	
	TOptional<FString> keyOpt = FTextInspector::GetKey(Dialogue->Data[index].Text);
	TOptional<FString> nsOpt = FTextInspector::GetNamespace(Dialogue->Data[index].Text);
	Dialogue->Data[index].Text = FText::ChangeKey(
		FTextKey(nsOpt.IsSet() ? nsOpt.GetValue() : ""),
		FTextKey(keyOpt.IsSet() ? keyOpt.GetValue() : ""),
		NewText
	);
}

#undef LOCTEXT_NAMESPACE
