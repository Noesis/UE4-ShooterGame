// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterIngameMenu.h"
#include "ShooterStyle.h"
#include "ShooterMenuSoundsWidgetStyle.h"
#include "Online.h"
#include "OnlineExternalUIInterface.h"
#include "ShooterGameInstance.h"
#include "UI/ShooterHUD.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

#if PLATFORM_SWITCH
#	define FRIENDS_SUPPORTED 0
#else
#	define FRIENDS_SUPPORTED 1
#endif


void FShooterIngameMenu::Construct(ULocalPlayer* _PlayerOwner)
{
	PlayerOwner = _PlayerOwner;
	bIsGameMenuUp = false;

	if (!GEngine || !GEngine->GameViewport)
	{
		return;
	}
	
	//todo:  don't create ingame menus for remote players.
	const UShooterGameInstance* GameInstance = nullptr;
	if (PlayerOwner)
	{
		GameInstance = Cast<UShooterGameInstance>(PlayerOwner->GetGameInstance());
	}

	if (!GameMenuWidget.IsValid())
	{
		SAssignNew(GameMenuWidget, SShooterMenuWidget)
			.PlayerOwner(TWeakObjectPtr<ULocalPlayer>(PlayerOwner))
			.Cursor(EMouseCursor::Default)
			.IsGameMenu(true);			


		int32 const OwnerUserIndex = GetOwnerUserIndex();

		// setup the exit to main menu submenu.  We wanted a confirmation to avoid a potential TRC violation.
		// fixes TTP: 322267
		TSharedPtr<FShooterMenuItem> MainMenuRoot = FShooterMenuItem::CreateRoot();
		MainMenuItem = MenuHelper::AddMenuItem(MainMenuRoot,LOCTEXT("Main Menu", "MAIN MENU"));
		MenuHelper::AddMenuItemSP(MainMenuItem,LOCTEXT("No", "NO"), this, &FShooterIngameMenu::OnCancelExitToMain);
		MenuHelper::AddMenuItemSP(MainMenuItem,LOCTEXT("Yes", "YES"), this, &FShooterIngameMenu::OnConfirmExitToMain);		

		ShooterOptions = MakeShareable(new FShooterOptions());
		ShooterOptions->Construct(PlayerOwner);
		ShooterOptions->TellInputAboutKeybindings();
		ShooterOptions->OnApplyChanges.BindSP(this, &FShooterIngameMenu::CloseSubMenu);

		MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterOptions->CheatsItem.ToSharedRef());
		MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterOptions->OptionsItem.ToSharedRef());

#if FRIENDS_SUPPORTED
		if (GameInstance && GameInstance->GetOnlineMode() == EOnlineMode::Online)
		{
#if !PLATFORM_XBOXONE
			ShooterFriends = MakeShareable(new FShooterFriends());
			ShooterFriends->Construct(PlayerOwner, OwnerUserIndex);
			ShooterFriends->TellInputAboutKeybindings();
			ShooterFriends->OnApplyChanges.BindSP(this, &FShooterIngameMenu::CloseSubMenu);

			MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterFriends->FriendsItem.ToSharedRef());

			ShooterRecentlyMet = MakeShareable(new FShooterRecentlyMet());
			ShooterRecentlyMet->Construct(PlayerOwner, OwnerUserIndex);
			ShooterRecentlyMet->TellInputAboutKeybindings();
			ShooterRecentlyMet->OnApplyChanges.BindSP(this, &FShooterIngameMenu::CloseSubMenu);

			MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterRecentlyMet->RecentlyMetItem.ToSharedRef());
#endif		

#if SHOOTER_CONSOLE_UI			
			TSharedPtr<FShooterMenuItem> ShowInvitesItem = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Invite Players", "INVITE PLAYERS"));
			ShowInvitesItem->OnConfirmMenuItem.BindRaw(this, &FShooterIngameMenu::OnShowInviteUI);		
#endif
		}
#endif

		if (FSlateApplication::Get().SupportsSystemHelp())
		{
			TSharedPtr<FShooterMenuItem> HelpSubMenu = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Help", "HELP"));
			HelpSubMenu->OnConfirmMenuItem.BindStatic([](){ FSlateApplication::Get().ShowSystemHelp(); });
		}

		MenuHelper::AddExistingMenuItem(RootMenuItem, MainMenuItem.ToSharedRef());
				
#if !SHOOTER_CONSOLE_UI
		MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Quit", "QUIT"), this, &FShooterIngameMenu::OnUIQuit);
#endif

		GameMenuWidget->MainMenu = GameMenuWidget->CurrentMenu = RootMenuItem->SubMenu;
		GameMenuWidget->OnMenuHidden.BindSP(this,&FShooterIngameMenu::DetachGameMenu);
		GameMenuWidget->OnToggleMenu.BindSP(this,&FShooterIngameMenu::ToggleGameMenu);
		GameMenuWidget->OnGoBack.BindSP(this, &FShooterIngameMenu::OnMenuGoBack);
	}
}

void FShooterIngameMenu::CloseSubMenu()
{
	GameMenuWidget->MenuGoBack();
}

void FShooterIngameMenu::OnMenuGoBack(MenuPtr Menu)
{
	// if we are going back from options menu
	if (ShooterOptions.IsValid() && ShooterOptions->OptionsItem->SubMenu == Menu)
	{
		ShooterOptions->RevertChanges();
	}
}

bool FShooterIngameMenu::GetIsGameMenuUp() const
{
	return bIsGameMenuUp;
}

void FShooterIngameMenu::UpdateFriendsList()
{
	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (OnlineSub)
	{
		IOnlineFriendsPtr OnlineFriendsPtr = OnlineSub->GetFriendsInterface();
		if (OnlineFriendsPtr.IsValid())
		{
			OnlineFriendsPtr->ReadFriendsList(GetOwnerUserIndex(), EFriendsLists::ToString(EFriendsLists::OnlinePlayers));
		}
	}
}

void FShooterIngameMenu::DetachGameMenu()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(GameMenuContainer.ToSharedRef());
	}
	bIsGameMenuUp = false;

	AShooterPlayerController* const PCOwner = PlayerOwner ? Cast<AShooterPlayerController>(PlayerOwner->PlayerController) : nullptr;
	if (PCOwner)
	{
		PCOwner->SetPause(false);

		// If the game is over enable the scoreboard
		AShooterHUD* const ShooterHUD = PCOwner->GetShooterHUD();
		if( ( ShooterHUD != NULL ) && ( ShooterHUD->IsMatchOver() == true ) && ( PCOwner->IsPrimaryPlayer() == true ) )
		{
			ShooterHUD->ShowScoreboard( true, true );
		}
	}
}

void FShooterIngameMenu::ToggleGameMenu()
{
	//Update the owner in case the menu was opened by another controller
	//UpdateMenuOwner();

	if (!GameMenuWidget.IsValid())
	{
		return;
	}

	// check for a valid user index.  could be invalid if the user signed out, in which case the 'please connect your control' ui should be up anyway.
	// in-game menu needs a valid userindex for many OSS calls.
	if (GetOwnerUserIndex() == -1)
	{
		UE_LOG(LogShooter, Log, TEXT("Trying to toggle in-game menu for invalid userid"));
		return;
	}

	if (bIsGameMenuUp && GameMenuWidget->CurrentMenu != RootMenuItem->SubMenu)
	{
		GameMenuWidget->MenuGoBack();
		return;
	}
	
	AShooterPlayerController* const PCOwner = PlayerOwner ? Cast<AShooterPlayerController>(PlayerOwner->PlayerController) : nullptr;
	if (!bIsGameMenuUp)
	{
		// Hide the scoreboard
		if (PCOwner)
		{
			AShooterHUD* const ShooterHUD = PCOwner->GetShooterHUD();
			if( ShooterHUD != NULL )
			{
				ShooterHUD->ShowScoreboard( false );
			}
		}

		GEngine->GameViewport->AddViewportWidgetContent(
			SAssignNew(GameMenuContainer,SWeakWidget)
			.PossiblyNullContent(GameMenuWidget.ToSharedRef())
			);

		int32 const OwnerUserIndex = GetOwnerUserIndex();
		if(ShooterOptions.IsValid())
		{
			ShooterOptions->UpdateOptions();
		}
		if(ShooterRecentlyMet.IsValid())
		{
			ShooterRecentlyMet->UpdateRecentlyMet(OwnerUserIndex);
		}
		GameMenuWidget->BuildAndShowMenu();
		bIsGameMenuUp = true;

		if (PCOwner)
		{
			// Disable controls while paused
			PCOwner->SetCinematicMode(true, false, false, true, true);

			PCOwner->SetPause(true);
		}
	} 
	else
	{
		//Start hiding animation
		GameMenuWidget->HideMenu();
		if (PCOwner)
		{
			// Make sure viewport has focus
			FSlateApplication::Get().SetAllUserFocusToGameViewport();

			// Don't renable controls if the match is over
			AShooterHUD* const ShooterHUD = PCOwner->GetShooterHUD();
			if( ( ShooterHUD != NULL ) && ( ShooterHUD->IsMatchOver() == false ) )
			{
				PCOwner->SetCinematicMode(false,false,false,true,true);
			}
		}
	}
}

void FShooterIngameMenu::OnCancelExitToMain()
{
	CloseSubMenu();
}

void FShooterIngameMenu::OnConfirmExitToMain()
{
	UShooterGameInstance* const GameInstance = Cast<UShooterGameInstance>(PlayerOwner->GetGameInstance());
	if (GameInstance)
	{
		GameInstance->LabelPlayerAsQuitter(PlayerOwner);

		// tell game instance to go back to main menu state
		GameInstance->GotoState(ShooterGameInstanceState::MainMenu);
	}
}

void FShooterIngameMenu::OnUIQuit()
{
	UShooterGameInstance* const GI = Cast<UShooterGameInstance>(PlayerOwner->GetGameInstance());
	if (GI)
	{
		GI->LabelPlayerAsQuitter(PlayerOwner);
	}

	GameMenuWidget->LockControls(true);
	GameMenuWidget->HideMenu();

	UWorld* const World = PlayerOwner ? PlayerOwner->GetWorld() : nullptr;
	if (World)
	{
		const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle<FShooterMenuSoundsStyle>("DefaultShooterMenuSoundsStyle");
		MenuHelper::PlaySoundAndCall(World, MenuSounds.ExitGameSound, GetOwnerUserIndex(), this, &FShooterIngameMenu::Quit);
	}
}

void FShooterIngameMenu::Quit()
{
	APlayerController* const PCOwner = PlayerOwner ? PlayerOwner->PlayerController : nullptr;
	if (PCOwner)
	{
		PCOwner->ConsoleCommand("quit");
	}
}

void FShooterIngameMenu::OnShowInviteUI()
{
	const auto ExternalUI = Online::GetExternalUIInterface();

	if(!ExternalUI.IsValid())
	{
		UE_LOG(LogShooter, Warning, TEXT("OnShowInviteUI: External UI interface is not supported on this platform."));
		return;
	}

	ExternalUI->ShowInviteUI(GetOwnerUserIndex());
}

int32 FShooterIngameMenu::GetOwnerUserIndex() const
{
	return PlayerOwner ? PlayerOwner->GetControllerId() : 0;
}


#undef LOCTEXT_NAMESPACE
