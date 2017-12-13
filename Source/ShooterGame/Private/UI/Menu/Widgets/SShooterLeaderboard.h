// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ShooterGame.h"
#include "ShooterLeaderboards.h"

/** leaderboard row display information */
struct FLeaderboardRow
{
	/** player rank*/
	FString Rank;

	/** player name */
	FString PlayerName;

	/** player total kills to display */
	FString Kills;

	/** player total deaths to display */
	FString Deaths;

	/** Unique Id for the player at this rank */
	const TSharedPtr<const FUniqueNetId> PlayerId;

	/** Default Constructor */
	FLeaderboardRow(const FOnlineStatsRow& Row);
};

//class declare
class SShooterLeaderboard : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterLeaderboard)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer >, PlayerOwner)
	SLATE_ARGUMENT(TSharedPtr<SWidget>, OwnerWidget)

	SLATE_END_ARGS()

	/** needed for every widget */
	void Construct(const FArguments& InArgs);

	/** if we want to receive focus */
	virtual bool SupportsKeyboardFocus() const override { return true; }

	/** focus received handler - keep the ActionBindingsList focused */
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	
	/** focus lost handler - keep the ActionBindingsList focused */
	virtual void OnFocusLost( const FFocusEvent& InFocusEvent ) override;

	/** key down handler */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/** creates single item widget, called for every list item */
	TSharedRef<ITableRow> MakeListViewWidget(TSharedPtr<FLeaderboardRow> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** selection changed handler */
	void EntrySelectionChanged(TSharedPtr<FLeaderboardRow> InItem, ESelectInfo::Type SelectInfo);

	/** is there a valid selected item */
	bool IsPlayerSelectedAndValid() const;

	/** check to see if we can open the profile ui */
	EVisibility GetProfileUIVisibility() const;

	/** profile open ui handler */
	bool ProfileUIOpened() const;

	/** Starts reading leaderboards for the game */
	void ReadStats();

	/** Called on a particular leaderboard read */
	void OnStatsRead(bool bWasSuccessful);

	/** Called to login on relevant platforms first before making a leaderboard read */
	void ReadStatsLoginRequired();

	/** Delegate after login has been been completed */
	void OnLoginCompleteReadStats(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	/** selects item at current + MoveBy index */
	void MoveSelection(int32 MoveBy);

protected:

	/** Removes the bound LeaderboardReadCompleteDelegate if possible*/
	void ClearOnLeaderboardReadCompleteDelegate();

	/** Returns true if a leaderboard read request is in progress or scheduled */
	bool IsLeaderboardReadInProgress();

	/** action bindings array */
	TArray< TSharedPtr<FLeaderboardRow> > StatRows;

	/** Leaderboard read object */
	FOnlineLeaderboardReadPtr ReadObject;

	/** Indicates that a stats read operation has been initiated */
	bool bReadingStats;

	/** Delegate called when a leaderboard has been successfully read */
	FOnLeaderboardReadCompleteDelegate LeaderboardReadCompleteDelegate;

	/** action bindings list slate widget */
	TSharedPtr< SListView< TSharedPtr<FLeaderboardRow> > > RowListWidget; 

	/** currently selected list item */
	TSharedPtr<FLeaderboardRow> SelectedItem;

	/** pointer to our owner PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	/** pointer to our parent widget */
	TSharedPtr<class SWidget> OwnerWidget;

	/** Handle to the registered LeaderboardReadComplete delegate */
	FDelegateHandle LeaderboardReadCompleteDelegateHandle;
	
	/** Handle to the registered LoginComplete delegate */
	FDelegateHandle OnLoginCompleteDelegateHandle;
};


