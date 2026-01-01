#include "DialogueSystemModule.h"
#include "DialoguePluginPrivatePCH.h"
#include "Runtime/Core/Public/Features/IModularFeatures.h"
#include "Runtime/SlateCore/Public/Rendering/SlateRenderer.h"
#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "DialogueSystem"

class FDialogueSystemModule : public IDialogueSystemModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:

	void Initialize( TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow );
	void AddGraphicsSwitcher( FToolBarBuilder& ToolBarBuilder );
	TSharedPtr< FExtender > NotificationBarExtender;
	bool bAllowAutomaticGraphicsSwitching;
	bool bAllowMultiGPUs;
};

IMPLEMENT_MODULE(FDialogueSystemModule, DialogueSystem)

void FDialogueSystemModule::StartupModule()
{	
	
}

void FDialogueSystemModule::Initialize( TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow )
{

}

void FDialogueSystemModule::AddGraphicsSwitcher( FToolBarBuilder& ToolBarBuilder )
{
	
}

void FDialogueSystemModule::ShutdownModule()
{

}

#undef LOCTEXT_NAMESPACE
