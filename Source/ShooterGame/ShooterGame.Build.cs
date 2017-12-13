// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ShooterGame : ModuleRules
{
	public ShooterGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateIncludePaths.AddRange(
			new string[] { 
				"ShooterGame/Classes/Player",
				"ShooterGame/Private",
				"ShooterGame/Private/UI",
				"ShooterGame/Private/UI/Menu",
				"ShooterGame/Private/UI/Style",
				"ShooterGame/Private/UI/Widgets",
            }
		);

        PublicDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"OnlineSubsystem",
				"OnlineSubsystemUtils",
				"AssetRegistry",
                "AIModule",
				"GameplayTasks",
			}
		);

        PrivateDependencyModuleNames.AddRange(
			new string[] {
				"InputCore",
				"Slate",
				"SlateCore",
				"ShooterGameLoadingScreen",
				"Json",
				"ApplicationCore"
			}
		);

		DynamicallyLoadedModuleNames.AddRange(
			new string[] {
				"OnlineSubsystemNull",
				"NetworkReplayStreaming",
				"NullNetworkReplayStreaming",
				"HttpNetworkReplayStreaming"
			}
		);

		PrivateIncludePathModuleNames.AddRange(
			new string[] {
				"NetworkReplayStreaming"
			}
		);
	}
}
