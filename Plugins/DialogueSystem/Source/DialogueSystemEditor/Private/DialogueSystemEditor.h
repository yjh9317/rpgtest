#pragma once
#include "CoreMinimal.h"
#include "IDialogueSystemEditorModule.h"
#include "Runtime/SlateCore/Public/Styling/SlateStyleRegistry.h"

#define LOCTEXT_NAMESPACE "DialogueSystemEditor"

class FDialogueSystemEditorModule : public IDialogueSystemEditorModule
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** IDialogueSystemEditorModule implementation */
	virtual bool AllowAutomaticGraphicsSwitching() override;
	virtual bool AllowMultipleGPUs() override;

	static uint32 GameAssetCategory;
	
private:
	void Initialize( TSharedPtr<class SWindow> InRootWindow, bool bIsNewProjectWindow );
	void AddGraphicsSwitcher( class FToolBarBuilder& ToolBarBuilder );
	
private:
	void RegisterAssetTypeAction(class IAssetTools& AssetTools, TSharedRef<class IAssetTypeActions> Action);
	TSharedPtr< class FExtender > NotificationBarExtender;
	bool bAllowAutomaticGraphicsSwitching;
	bool bAllowMultiGPUs;
};

#undef LOCTEXT_NAMESPACE