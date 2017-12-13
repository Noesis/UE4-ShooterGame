// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ShooterGameInstance.h"

class SShooterSplitScreenLobby : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS( SShooterSplitScreenLobby )
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)
		
	SLATE_ARGUMENT(FOnClicked, OnPlayClicked)
	SLATE_ARGUMENT(FOnClicked, OnCancelClicked)

	SLATE_END_ARGS()	

	/** says that we can support keyboard focus */
	virtual bool SupportsKeyboardFocus() const override { return true; }

	void Construct(const FArguments& InArgs);

	void Clear();

	int32 GetNumSupportedSlots() const;

	bool GetIsJoining() const { return bIsJoining; }

	void SetIsJoining( const bool _bIsJoining ) { bIsJoining =_bIsJoining; }

private:
	bool IsUniqueIdOnline( const FUniqueNetId& ControllerId ) const;

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	virtual void OnFocusLost( const FFocusEvent& InFocusEvent ) override;

	void UpdateSlots();
	void ConditionallyReadyPlayer( const int ControllerId, const bool bCanShowUI );
	void ReadyPlayer( const int ControllerId );
	void UnreadyPlayer( const int ControllerId );

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	void HandleLoginUIClosedAndReady(TSharedPtr<const FUniqueNetId> UniqueId, const int UserIndex);

	UShooterGameInstance* GetGameInstance() const;

	FText GetPlayFindText() const;
#if PLATFORM_SWITCH
	FText GetPlayAsGuestText() const;
#endif

	FReply OnOkOrCancel();
	bool ConfirmSponsorsSatisfied() const;
	void OnUserCanPlay(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	static const int MAX_POSSIBLE_SLOTS = 4;

	/** The player that owns the Lobby. */
	TWeakObjectPtr<ULocalPlayer> PlayerOwner;

	FOnClicked MasterUserBack;
	FOnClicked MasterUserPlay;
	
	TSharedPtr<SRichTextBlock> UserTextWidgets[MAX_POSSIBLE_SLOTS];
	TSharedPtr<SWidget> UserSlots[MAX_POSSIBLE_SLOTS];

	/** used for holding on to the splitscreen lobby widget so we can switch back to that UI after the LoginFailure UI pops up */
	TSharedPtr<SWidget> SplitScreenLobbyWidget;

	FText PressToPlayText;
	FText PressToFindText;
	FText PressToStartMatchText;
#if PLATFORM_SWITCH
	FText PlayAsGuestText;
#endif

	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	FDelegateHandle OnLoginCompleteDelegateHandle;

	int PendingControllerId;

	/** True if we joining a match */
	bool bIsJoining;
};
