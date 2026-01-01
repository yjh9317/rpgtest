#include "DialogueFactory.h"
#include "DialogueSystemEditor.h"
#include "Dialogue.h"


#define LOCTEXT_NAMESPACE "DialogueSystemEditor" 

UDialogueFactory::UDialogueFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bCreateNew = true;
	bEditAfterNew = true;
	SupportedClass = UDialogue::StaticClass();
}

UObject* UDialogueFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UDialogue* NewObjectAsset = NewObject<UDialogue>(InParent, Class, Name, Flags | RF_Transactional);
	return NewObjectAsset;
}

uint32 UDialogueFactory::GetMenuCategories() const
{
	return FDialogueSystemEditorModule::GameAssetCategory;
}

FText UDialogueFactory::GetDisplayName() const
{
	return LOCTEXT("DialogueText", "Dialogue");
}

FString UDialogueFactory::GetDefaultNewAssetName() const
{
	return FString(TEXT("NewDialogue"));
}

#undef LOCTEXT_NAMESPACE