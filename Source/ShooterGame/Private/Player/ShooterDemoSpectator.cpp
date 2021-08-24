// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterDemoSpectator.h"
#include "UI/Menu/ShooterDemoPlaybackMenu.h"
#include "UI/Widgets/SShooterDemoHUD.h"
#include "Engine/DemoNetDriver.h"

AShooterDemoSpectator::AShooterDemoSpectator(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bShowMouseCursor = true;
	PrimaryActorTick.bTickEvenWhenPaused = true;
	bShouldPerformFullTickWhenPaused = true;
}

void AShooterDemoSpectator::SetupInputComponent()
{
	Super::SetupInputComponent();

	// UI input
	//InputComponent->BindAction( "InGameMenu", IE_Pressed, this, &AShooterDemoSpectator::OnToggleInGameMenu );

	InputComponent->BindAction( "NextWeapon", IE_Pressed, this, &AShooterDemoSpectator::OnIncreasePlaybackSpeed );
	InputComponent->BindAction( "PrevWeapon", IE_Pressed, this, &AShooterDemoSpectator::OnDecreasePlaybackSpeed );
}

void AShooterDemoSpectator::SetPlayer( UPlayer* InPlayer )
{
	Super::SetPlayer( InPlayer );

	// Build menu only after game is initialized
	ShooterDemoPlaybackMenu = MakeShareable( new FShooterDemoPlaybackMenu() );
	ShooterDemoPlaybackMenu->Construct( Cast< ULocalPlayer >( Player ) );

	// Create HUD if this is playback
	if (GetWorld() != nullptr && GetWorld()->GetDemoNetDriver() != nullptr && !GetWorld()->GetDemoNetDriver()->IsServer())
	{
		if (GEngine != nullptr && GEngine->GameViewport != nullptr)
		{
			DemoHUD = SNew(SShooterDemoHUD)
				.PlayerOwner(this);

			GEngine->GameViewport->AddViewportWidgetContent(DemoHUD.ToSharedRef());
		}
	}

	FActorSpawnParameters SpawnInfo;

	SpawnInfo.Owner				= this;
	SpawnInfo.Instigator		= GetInstigator();
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PlaybackSpeed = 2;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(DemoHUD);

	SetInputMode(InputMode);
}

void AShooterDemoSpectator::OnToggleInGameMenu()
{
	// if no one's paused, pause
	if ( ShooterDemoPlaybackMenu.IsValid() )
	{
		ShooterDemoPlaybackMenu->ToggleGameMenu();
	}
}

static float PlaybackSpeedLUT[5] = { 0.1f, 0.5f, 1.0f, 2.0f, 4.0f };

void AShooterDemoSpectator::OnIncreasePlaybackSpeed()
{
	PlaybackSpeed = FMath::Clamp( PlaybackSpeed + 1, 0, 4 );

	GetWorldSettings()->DemoPlayTimeDilation = PlaybackSpeedLUT[ PlaybackSpeed ];
}

void AShooterDemoSpectator::OnDecreasePlaybackSpeed()
{
	PlaybackSpeed = FMath::Clamp( PlaybackSpeed - 1, 0, 4 );

	GetWorldSettings()->DemoPlayTimeDilation = PlaybackSpeedLUT[ PlaybackSpeed ];
}

void AShooterDemoSpectator::Destroyed()
{
	if (GEngine != nullptr && GEngine->GameViewport != nullptr && DemoHUD.IsValid())
	{
		// Remove HUD
		GEngine->GameViewport->RemoveViewportWidgetContent(DemoHUD.ToSharedRef());
	}

	Super::Destroyed();
}
