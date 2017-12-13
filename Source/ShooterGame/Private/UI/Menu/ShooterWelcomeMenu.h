// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "SlateBasics.h"
#include "SlateExtras.h"

class FShooterWelcomeMenu : public TSharedFromThis<FShooterWelcomeMenu>
{
public:
	/** build menu */
	void Construct( TWeakObjectPtr< class UShooterGameInstance > InGameInstance );

	/** Add the menu to the gameviewport so it becomes visible */
	void AddToGameViewport();

	/** Remove from the gameviewport. */
	void RemoveFromGameViewport();

	/**
	 * The delegate function for external login UI closure when a user has signed in.
	 *
	 * @param UniqueId The unique Id of the user who just signed in.
	 * @param ControllerIndex The controller index of the player who just signed in.
	 */
	void HandleLoginUIClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex);

	/**
	 * Called when a user needs to advance from the welcome screen to the main menu.
	 *
	 * @param ControllerIndex The controller index of the player that's advancing.
	 */
	void SetControllerAndAdvanceToMainMenu(const int ControllerIndex);

	/** Lock/Unlock controls based*/
	void LockControls(bool bLock)
	{
		bControlsLocked = bLock;
	}

	bool GetControlsLocked() const
	{
		return bControlsLocked;
	}
	TWeakObjectPtr< class UShooterGameInstance > GetGameInstance() { return GameInstance; }

private:
	/** Owning game instance */
	TWeakObjectPtr< class UShooterGameInstance > GameInstance;

	/** Cache the user id that tried to advance, so we can use it again after the confirmation dialog */
	int PendingControllerIndex;

	/** "Presss A/X to play" widget */
	TSharedPtr<class SWidget> MenuWidget;

	/** Handler for continue-without-saving confirmation. */
	FReply OnContinueWithoutSavingConfirm();

	/** Generic Handler for hiding the dialog. */
	FReply OnConfirmGeneric();

	/** Handler for querying a user's privileges */
	void OnUserCanPlay(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	bool bControlsLocked;
};
