// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

// This module must be loaded "PreLoadingScreen" in the .uproject file, otherwise it will not hook in time!

public class ShooterGameLoadingScreen : ModuleRules
{
    public ShooterGameLoadingScreen(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateIncludePaths.Add("../../ShooterGame/Source/ShooterGameLoadingScreen/Private");

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"MoviePlayer",
				"Slate",
				"SlateCore",
				"InputCore"
			}
		);
	}
}
