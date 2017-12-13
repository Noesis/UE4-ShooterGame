// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterDemoPlaybackMenu.h"
#include "ShooterStyle.h"
#include "ShooterMenuSoundsWidgetStyle.h"
#include "ShooterGameInstance.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void FShooterDemoPlaybackMenu::Construct( ULocalPlayer* _PlayerOwner )
{
	PlayerOwner = _PlayerOwner;
	bIsAddedToViewport = false;

	if ( !GEngine || !GEngine->GameViewport )
	{
		return;
	}
	
	if ( !GameMenuWidget.IsValid() )
	{
		SAssignNew( GameMenuWidget, SShooterMenuWidget )
			.PlayerOwner( TWeakObjectPtr<ULocalPlayer>( PlayerOwner ) )
			.Cursor( EMouseCursor::Default )
			.IsGameMenu( true );			

		TSharedPtr<FShooterMenuItem> MainMenuRoot = FShooterMenuItem::CreateRoot();
		MainMenuItem = MenuHelper::AddMenuItem(MainMenuRoot,LOCTEXT( "Main Menu", "MAIN MENU" ) );

		MenuHelper::AddMenuItemSP( MainMenuItem, LOCTEXT( "No", "NO" ), this, &FShooterDemoPlaybackMenu::OnCancelExitToMain );
		MenuHelper::AddMenuItemSP( MainMenuItem, LOCTEXT( "Yes", "YES" ), this, &FShooterDemoPlaybackMenu::OnConfirmExitToMain );

		MenuHelper::AddExistingMenuItem( RootMenuItem, MainMenuItem.ToSharedRef() );
				
#if !SHOOTER_CONSOLE_UI
		MenuHelper::AddMenuItemSP( RootMenuItem, LOCTEXT("Quit", "QUIT"), this, &FShooterDemoPlaybackMenu::OnUIQuit );
#endif

		GameMenuWidget->MainMenu = GameMenuWidget->CurrentMenu = RootMenuItem->SubMenu;
		GameMenuWidget->OnMenuHidden.BindSP( this, &FShooterDemoPlaybackMenu::DetachGameMenu );
		GameMenuWidget->OnToggleMenu.BindSP( this, &FShooterDemoPlaybackMenu::ToggleGameMenu );
		GameMenuWidget->OnGoBack.BindSP( this, &FShooterDemoPlaybackMenu::OnMenuGoBack );
	}
}

void FShooterDemoPlaybackMenu::CloseSubMenu()
{
	GameMenuWidget->MenuGoBack();
}

void FShooterDemoPlaybackMenu::OnMenuGoBack(MenuPtr Menu)
{
}

void FShooterDemoPlaybackMenu::DetachGameMenu()
{
	if ( GEngine && GEngine->GameViewport )
	{
		GEngine->GameViewport->RemoveViewportWidgetContent( GameMenuContainer.ToSharedRef() );
	}

	bIsAddedToViewport = false;
}

void FShooterDemoPlaybackMenu::ToggleGameMenu()
{
	if ( !GameMenuWidget.IsValid( ))
	{
		return;
	}

	if ( bIsAddedToViewport && GameMenuWidget->CurrentMenu != RootMenuItem->SubMenu )
	{
		GameMenuWidget->MenuGoBack();
		return;
	}
	
	if ( !bIsAddedToViewport )
	{
		GEngine->GameViewport->AddViewportWidgetContent( SAssignNew( GameMenuContainer, SWeakWidget ).PossiblyNullContent( GameMenuWidget.ToSharedRef() ) );

		GameMenuWidget->BuildAndShowMenu();

		bIsAddedToViewport = true;
	} 
	else
	{
		// Start hiding animation
		GameMenuWidget->HideMenu();

		AShooterPlayerController* const PCOwner = PlayerOwner ? Cast<AShooterPlayerController>(PlayerOwner->PlayerController) : nullptr;

		if ( PCOwner )
		{
			// Make sure viewport has focus
			FSlateApplication::Get().SetAllUserFocusToGameViewport();
		}
	}
}

void FShooterDemoPlaybackMenu::OnCancelExitToMain()
{
	CloseSubMenu();
}

void FShooterDemoPlaybackMenu::OnConfirmExitToMain()
{
	UShooterGameInstance* const GameInstance = Cast<UShooterGameInstance>( PlayerOwner->GetGameInstance() );

	if ( GameInstance )
	{
		// tell game instance to go back to main menu state
		GameInstance->GotoState( ShooterGameInstanceState::MainMenu );
	}
}

void FShooterDemoPlaybackMenu::OnUIQuit()
{
	UShooterGameInstance* const GameInstance = Cast<UShooterGameInstance>( PlayerOwner->GetGameInstance() );

	GameMenuWidget->LockControls( true );
	GameMenuWidget->HideMenu();

	UWorld* const World = PlayerOwner ? PlayerOwner->GetWorld() : nullptr;
	if ( World )
	{
		const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle< FShooterMenuSoundsStyle >( "DefaultShooterMenuSoundsStyle" );
		MenuHelper::PlaySoundAndCall( World, MenuSounds.ExitGameSound, GetOwnerUserIndex(), this, &FShooterDemoPlaybackMenu::Quit );
	}
}

void FShooterDemoPlaybackMenu::Quit()
{
	APlayerController* const PCOwner = PlayerOwner ? PlayerOwner->PlayerController : nullptr;

	if ( PCOwner )
	{
		PCOwner->ConsoleCommand( "quit" );
	}
}

int32 FShooterDemoPlaybackMenu::GetOwnerUserIndex() const
{
	return PlayerOwner ? PlayerOwner->GetControllerId() : 0;
}
#undef LOCTEXT_NAMESPACE
