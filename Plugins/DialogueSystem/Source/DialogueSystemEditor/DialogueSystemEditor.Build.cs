// Source/DialogueSystemEditor/DialogueSystemEditor.Build.cs

using UnrealBuildTool;

public class DialogueSystemEditor : ModuleRules
{
	public DialogueSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"DialogueSystem",
				
				// --- 에디터 UI 및 기능에 필요한 모듈들 ---
				"UnrealEd",
				"PropertyEditor",
				"EditorStyle",
				"AssetTools",
				"GraphEditor",
				"Json",
				"JsonUtilities",
				"WorkspaceMenuStructure",
				"Slate",
				"SlateCore",
				"InputCore",
				"ToolWidgets",
				"EditorWidgets", 
				"KismetWidgets",
				"Projects",
				"ApplicationCore" 
			}
		);
	}
}