// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class ShooterGameServerTarget : TargetRules
{
	public ShooterGameServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		bUsesSteam = true;

		ExtraModuleNames.Add("ShooterGame");
	}
}
