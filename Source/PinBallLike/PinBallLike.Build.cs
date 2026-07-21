// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PinBallLike : ModuleRules
{
	public PinBallLike(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput",
			"UMG", 
			"ModelViewViewModel",
			"StateTreeModule",
			"GameplayStateTreeModule", 
			"GameplayTags",
			"Slate",
			"SlateCore",
			"AssetRegistry",
			"DeveloperSettings",
			"GameplayMessageRuntime",
			"Niagara"
			
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"EngineCameras",
			"AsyncLoadingScreen",
			"MoviePlayer"
		});
			
		
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
