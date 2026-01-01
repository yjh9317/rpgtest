#include "DialogueSystemEditor.h"
#include "DialogueSystemEditorPrivatePCH.h"
#include "IDialogueSystemEditorModule.h"
#include "DialogueSystemEditorSettingsDetails.h"
#include "DialogueAssetTypeActions.h"
#include "DialogueEditorStyle.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"
#include "Runtime/SlateCore/Public/Rendering/SlateRenderer.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "ISettingsModule.h"
#include "LevelEditor.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "DialogueSystemEditor"

uint32 FDialogueSystemEditorModule::GameAssetCategory;

IMPLEMENT_MODULE(FDialogueSystemEditorModule, DialogueSystemEditor)

void FDialogueSystemEditorModule::StartupModule()
{	
	// registers the style (sets icons)
	FDialogueEditorStyle::Initialize(); 

	// Register asset types
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	// creates a category named Dialogue System in the advanced assets category (rmb)
	// this uint32 may then be used by factories that want their custom type to be listed in this category as well
	GameAssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("DialogueSystem")), LOCTEXT("DialogueSystemCategory", "Dialogue System"));

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout("Dialogue", FOnGetDetailCustomizationInstance::CreateStatic(&FDialogueSystemEditorSettingsDetails::MakeInstance));
	PropertyModule.NotifyCustomizationModuleChanged();

	//EAssetTypeCategories::Type AssetCategoryBit = EAssetTypeCategories::Misc;

	//TSharedRef<IAssetTypeActions> Action = MakeShareable(new FDialogueAssetTypeActions(AssetCategoryBit));
	//RegisterAssetTypeAction(AssetTools, Action);

	// register AssetTypeActions (changes their background color)
	AssetTools.RegisterAssetTypeActions(MakeShareable(new FDialogueAssetTypeActions(GameAssetCategory)));
}

void FDialogueSystemEditorModule::Initialize( TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow )
{

}

void FDialogueSystemEditorModule::AddGraphicsSwitcher( FToolBarBuilder& ToolBarBuilder )
{
	
}

void FDialogueSystemEditorModule::ShutdownModule()
{
	FDialogueEditorStyle::Shutdown();
}

bool FDialogueSystemEditorModule::AllowAutomaticGraphicsSwitching()
{
	return bAllowAutomaticGraphicsSwitching;
}

bool FDialogueSystemEditorModule::AllowMultipleGPUs()
{
	return bAllowMultiGPUs;
}

void FDialogueSystemEditorModule::RegisterAssetTypeAction(class IAssetTools& AssetTools, TSharedRef<class IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
}

#undef LOCTEXT_NAMESPACE