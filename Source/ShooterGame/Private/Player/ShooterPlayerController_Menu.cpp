// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterPlayerController_Menu.h"
#include "ShooterStyle.h"


AShooterPlayerController_Menu::AShooterPlayerController_Menu(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

UShooterLocalPlayer* AShooterPlayerController_Menu::GetLocalPlayer() const
{
	return Cast<UShooterLocalPlayer>(Player);
}

void AShooterPlayerController_Menu::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	FShooterStyle::Initialize();
}
