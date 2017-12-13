// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ShooterGame.h"
#include "SShooterMenuWidget.h"
#include "NetworkReplayStreaming.h"
#include "Misc/NetworkVersion.h"

struct FDemoEntry;

//class declare
class SShooterDemoList : public SShooterMenuWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterDemoList)
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)
	SLATE_ARGUMENT(TSharedPtr<SWidget>, OwnerWidget)

	SLATE_END_ARGS()

	/** needed for every widget */
	void Construct(const FArguments& InArgs);

	/** if we want to receive focus */
	virtual bool SupportsKeyboardFocus() const override { return true; }

	/** focus received handler - keep the ActionBindingsList focused */
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	
	/** focus lost handler - keep the ActionBindingsList focused */
	virtual void OnFocusLost(const FFocusEvent& InFocusEvent) override;

	/** key down handler */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyboardEvent) override;

	/** SListView item double clicked */
	void OnListItemDoubleClicked(TSharedPtr<FDemoEntry> InItem);

	/** creates single item widget, called for every list item */
	TSharedRef<ITableRow> MakeListViewWidget(TSharedPtr<FDemoEntry> Item, const TSharedRef<STableViewBase>& OwnerTable);

	/** selection changed handler */
	void EntrySelectionChanged(TSharedPtr<FDemoEntry> InItem, ESelectInfo::Type SelectInfo);

	/** Updates the list until it's completely populated */
	void UpdateBuildDemoListStatus();

	/** Populates the demo list */
	void BuildDemoList();

	/** Called when demo list building finished */
	void OnBuildDemoListFinished();

	/** Called when we get results from the replay streaming interface */
	void OnEnumerateStreamsComplete(const TArray<FNetworkReplayStreamInfo>& Streams);

	/** Play chosen demo */
	void PlayDemo();

	/** Delete chosen demo */
	void DeleteDemo();

	/** Delete chosen demo (really) */
	FReply OnDemoDeleteConfirm();

	/** Cancel delete chosen demo */
	FReply OnDemoDeleteCancel();

	/** Called by delegate when the replay streaming interface has finished deleting */
	void OnDeleteFinishedStreamComplete(bool bWasSuccessful);

	/** selects item at current + MoveBy index */
	void MoveSelection(int32 MoveBy);

	/**
	 * Ticks this widget.  Override in derived classes, but always call the parent implementation.
	 *
	 * @param  AllottedGeometry The space allotted for this widget
	 * @param  InCurrentTime  Current absolute real time
	 * @param  InDeltaTime  Real time passed since last tick
	 */
	void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime );

private:
	/** Callback for "show all replay versions" checkbox */
	ECheckBoxState IsShowAllReplaysChecked() const;

	/** Callback fired when "show all replay versions" checkbox is changed */
	void OnShowAllReplaysChecked(ECheckBoxState NewCheckedState);

	/** Version used for enumerating replays. This is manipulated depending on whether we want to show all versions or not. */
	FNetworkReplayVersion EnumerateStreamsVersion;

protected:

	/** Whether we're building the demo list or not */
	bool bUpdatingDemoList;

	/** action bindings array */
	TArray< TSharedPtr<FDemoEntry> > DemoList;

	/** action bindings list slate widget */
	TSharedPtr< SListView< TSharedPtr<FDemoEntry> > > DemoListWidget; 

	/** currently selected list item */
	TSharedPtr<FDemoEntry> SelectedItem;

	/** get current status text */
	FText GetBottomText() const;

	/** current status text */
	FText StatusText;

	/** pointer to our owner PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	/** pointer to our parent widget */
	TSharedPtr<class SWidget> OwnerWidget;

	/** Network replay streaming interface */
	TSharedPtr<INetworkReplayStreamer> ReplayStreamer;
};


