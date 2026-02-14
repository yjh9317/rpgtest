// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class RPGSystem : ModuleRules
{
	public RPGSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject",
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"NavigationSystem",
			"EngineCameras",
			"GameplayCameras",
			"TemplateSequence",
			"MovieScene",
			"LevelSequence",
			"UMG",     
			"Slate",   
			"SlateCore",
			"Niagara",
			"RenderCore",
			"RHI", 
			"DialogueSystem"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"GameplayTags",
			"NetCore",
			"UMG",
			"Slate",
			"SlateCore",
			"AnimGraphRuntime",
			"AnimationLocomotionLibraryRuntime",
		});
		
		PublicIncludePaths.Add(ModuleDirectory + "/Public");
		PrivateIncludePaths.Add(ModuleDirectory + "/Private");

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd" 
			});
		}
	}
}
