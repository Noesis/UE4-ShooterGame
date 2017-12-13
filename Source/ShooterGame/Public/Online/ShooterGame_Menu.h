// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterGame_Menu.generated.h"

UCLASS()
class AShooterGame_Menu : public AGameModeBase
{
	GENERATED_UCLASS_BODY()

public:

	// Begin AGameModeBase interface
	/** skip it, menu doesn't require player start or pawn */
	virtual void RestartPlayer(class AController* NewPlayer) override;

	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;
	// End AGameModeBase interface

protected:

	/** Perform some final tasks before hosting/joining a session. Remove menus, set king state etc */
	void BeginSession();
	
	/** Display a loading screen */
	void ShowLoadingScreen();
};
