// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterDemoSpectator.generated.h"

class SShooterDemoHUD;

UCLASS(config=Game)
class AShooterDemoSpectator : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:
	/** shooter in-game menu */
	TSharedPtr<class FShooterDemoPlaybackMenu> ShooterDemoPlaybackMenu;

	virtual void SetupInputComponent() override;
	virtual void SetPlayer( UPlayer* Player ) override;
	virtual void Destroyed() override;

	void OnToggleInGameMenu();
	void OnIncreasePlaybackSpeed();
	void OnDecreasePlaybackSpeed();

	int32 PlaybackSpeed;

private:
	TSharedPtr<SShooterDemoHUD> DemoHUD;
};

