// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Widgets/ShooterMenuItem.h"
#include "Widgets/SShooterMenuWidget.h"
#include "ShooterOptions.h"
#include "ShooterFriends.h"
#include "ShooterRecentlyMet.h"

class FShooterIngameMenu : public TSharedFromThis<FShooterIngameMenu>
{
public:
	/** sets owning player controller */
	void Construct(ULocalPlayer* PlayerOwner);

	/** toggles in game menu */
	void ToggleGameMenu();

	/** is game menu currently active? */
	bool GetIsGameMenuUp() const;

	/* updates the friends list of the current owner*/
	void UpdateFriendsList();

	/* Getter for the ShooterFriends interface/pointer*/
	TSharedPtr<class FShooterFriends> GetShooterFriends(){ return ShooterFriends; }

protected:

	/** Owning player controller */
	ULocalPlayer* PlayerOwner;

	/** game menu container widget - used for removing */
	TSharedPtr<class SWeakWidget> GameMenuContainer;

	/** root menu item pointer */
	TSharedPtr<FShooterMenuItem> RootMenuItem;

	/** main menu item pointer */
	TSharedPtr<FShooterMenuItem> MainMenuItem;

	/** HUD menu widget */
	TSharedPtr<class SShooterMenuWidget> GameMenuWidget;	

	/** if game menu is currently opened*/
	bool bIsGameMenuUp;

	/** holds cheats menu item to toggle it's visibility */
	TSharedPtr<class FShooterMenuItem> CheatsMenu;

	/** Shooter options */
	TSharedPtr<class FShooterOptions> ShooterOptions;

	/** get current user index out of PlayerOwner */
	int32 GetOwnerUserIndex() const;
	/** Shooter friends */
	TSharedPtr<class FShooterFriends> ShooterFriends;

	/** Shooter recently met users*/
	TSharedPtr<class FShooterRecentlyMet> ShooterRecentlyMet;

	/** called when going back to previous menu */
	void OnMenuGoBack(MenuPtr Menu);
	
	/** goes back in menu structure */
	void CloseSubMenu();

	/** removes widget from viewport */
	void DetachGameMenu();
	
	/** Delegate called when user cancels confirmation dialog to exit to main menu */
	void OnCancelExitToMain();

	/** Delegate called when user confirms confirmation dialog to exit to main menu */
	void OnConfirmExitToMain();		

	/** Plays sound and calls Quit */
	void OnUIQuit();

	/** Quits the game */
	void Quit();

	/** Shows the system UI to invite friends to the game */
	void OnShowInviteUI();
};
