// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BumpurrCars : ModuleRules
{
	public BumpurrCars(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });

        PublicDependencyModuleNames.Add("GameplayAbilities");   // For GAS functionality
        PublicDependencyModuleNames.Add("GameplayTags");        // For tagging abilities, effects, etc.
        PublicDependencyModuleNames.Add("GameplayTasks");       // For task management within abilities
        PublicDependencyModuleNames.Add("UMG");                 // For UI components (widgets, menus, etc.)
        PublicDependencyModuleNames.Add("PhysicsCore");         // Car physics
        PublicDependencyModuleNames.Add("InputDevice");
        PublicDependencyModuleNames.Add("Niagara");

        PrivateDependencyModuleNames.AddRange(new string[] {  });

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
