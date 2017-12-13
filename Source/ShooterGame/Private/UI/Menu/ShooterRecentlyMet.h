// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Widgets/ShooterMenuItem.h"
#include "Widgets/SShooterMenuWidget.h"

class UShooterGameUserSettings;
class UShooterPersistentUser;

/** delegate called when changes are applied */
DECLARE_DELEGATE(FOnApplyChanges);

/** delegate called when a menu item is confirmed */
DECLARE_DELEGATE(FOnConfirmMenuItem);

/** delegate called down on the gamepad is pressed*/
DECLARE_DELEGATE(FOnControllerDownInputPressed);

/** delegate called up on the gamepad is pressed*/
DECLARE_DELEGATE(FOnControllerUpInputPressed);

/** delegate called when facebutton_down is pressed */
DECLARE_DELEGATE(FOnOnControllerFacebuttonDownPressed);

class FShooterRecentlyMet : public TSharedFromThis<FShooterRecentlyMet>
{
public:
	/** sets owning player controller */
	void Construct(ULocalPlayer* _PlayerOwner, int32 LocalUserNum);

	/** get current Friends values for display */
	void UpdateRecentlyMet(int32 NewOwnerIndex);

	/** UI callback for applying settings, plays sound */
	void OnApplySettings();

	/** applies changes in game settings */
	void ApplySettings();

	/** needed because we can recreate the subsystem that stores it */
	void TellInputAboutKeybindings();

	/** increment the counter keeping track of which user we're looking at */
	void IncrementRecentlyMetCounter();

	/** decrement the counter keeping track of which user we're looking at */
	void DecrementRecentlyMetCounter();

	/** send friend request to selected user */
	void ViewSelectedUsersProfile();

	/** holds Recently Met sub-menu */
	TSharedPtr<FShooterMenuItem> RecentlyMetItem;

	/** holds Recently Met menu item */
	TSharedPtr<FShooterMenuItem> RecentlyMetRoot;

	/** called when changes were applied - can be used to close submenu */
	FOnApplyChanges OnApplyChanges;

	/** delegate, which is executed by SShooterMenuWidget if user confirms this menu item */
	FOnConfirmMenuItem OnConfirmMenuItem;

	/** delegate, which is executed by SShooterMenuWidget if down input is pressed */
	FOnControllerDownInputPressed OnControllerDownInputPressed;

	/** delegate, which is executed by SShooterMenuWidget if up input is pressed */
	FOnControllerUpInputPressed OnControllerUpInputPressed;

	/** delegate, which is executed by SShooterMenuWidget if facebutton_down is pressed */
	FOnOnControllerFacebuttonDownPressed OnControllerFacebuttonDownPressed;

	int32 LocalUserNum;
	int32 CurrRecentlyMetIndex;
	int32 MinRecentlyMetIndex;
	int32 MaxRecentlyMetIndex;

	FString LocalUsername;

	TArray<class APlayerState*> PlayerArray;

	IOnlineSubsystem* OnlineSub;

protected:
	/** User settings pointer */
	UShooterGameUserSettings* UserSettings;

	/** Get the persistence user associated with PCOwner*/
	UShooterPersistentUser* GetPersistentUser() const;

	/** Owning player controller */
	ULocalPlayer* PlayerOwner;

	/** style used for the shooter Friends */
	const struct FShooterOptionsStyle *RecentlyMetStyle;
};