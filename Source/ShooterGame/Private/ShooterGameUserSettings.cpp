// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGameUserSettings.h"

UShooterGameUserSettings::UShooterGameUserSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetToDefaults();
}

void UShooterGameUserSettings::SetToDefaults()
{
	Super::SetToDefaults();

	GraphicsQuality = 1;	
	bIsLanMatch = true;
	bIsDedicatedServer = false;
}

void UShooterGameUserSettings::ApplySettings(bool bCheckForCommandLineOverrides)
{
	if (GraphicsQuality == 0)
	{
		ScalabilityQuality.SetFromSingleQualityLevel(1);
	}
	else
	{
		ScalabilityQuality.SetFromSingleQualityLevel(3);
	}

	Super::ApplySettings(bCheckForCommandLineOverrides);
}