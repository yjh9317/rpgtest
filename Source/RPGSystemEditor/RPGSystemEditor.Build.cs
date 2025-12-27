// Source/RPGSystemEditor/RPGSystemEditor.Build.cs
using UnrealBuildTool;

public class RPGSystemEditor : ModuleRules
{
	public RPGSystemEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"RPGSystem",
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"Slate",
			"SlateCore",
			"InputCore",
			"UnrealEd",              // 에디터 프레임워크
			"LevelEditor",           // 레벨 에디터
			"PropertyEditor",        // 프로퍼티 커스터마이징
			"AssetTools",            // 에셋 생성/관리
			"ContentBrowser",        // 콘텐츠 브라우저
			"EditorStyle",           // 에디터 스타일
			"EditorWidgets",         // 에디터 위젯
			"WorkspaceMenuStructure",// 메뉴 구조
			"ToolMenus",             // 툴바/메뉴
			"DataValidation",
		});
		
		PublicIncludePaths.Add(ModuleDirectory + "/Public");
		PrivateIncludePaths.Add(ModuleDirectory + "/Private");
	}
}