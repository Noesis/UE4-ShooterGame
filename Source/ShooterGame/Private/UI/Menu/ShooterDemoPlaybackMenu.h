// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Widgets/ShooterMenuItem.h"
#include "Widgets/SShooterMenuWidget.h"

class FShooterDemoPlaybackMenu : public TSharedFromThis<FShooterDemoPlaybackMenu>
{
public:
	/** sets owning player controller */
	void Construct( ULocalPlayer* PlayerOwner );

	/** toggles in game menu */
	void ToggleGameMenu();

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

	/** if game menu is currently added to the viewport */
	bool bIsAddedToViewport;

	/** get current user index out of PlayerOwner */
	int32 GetOwnerUserIndex() const;

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
};
