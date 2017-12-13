// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGame_Menu.h"
#include "ShooterMainMenu.h"
#include "ShooterWelcomeMenu.h"
#include "ShooterMessageMenu.h"
#include "ShooterPlayerController_Menu.h"
#include "Online/ShooterGameSession.h"

AShooterGame_Menu::AShooterGame_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PlayerControllerClass = AShooterPlayerController_Menu::StaticClass();
}

void AShooterGame_Menu::RestartPlayer(class AController* NewPlayer)
{
	// don't restart
}

/** Returns game session class to use */
TSubclassOf<AGameSession> AShooterGame_Menu::GetGameSessionClass() const
{
	return AShooterGameSession::StaticClass();
}
