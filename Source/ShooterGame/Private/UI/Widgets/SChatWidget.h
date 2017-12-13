// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ShooterHUDPCTrackerBase.h"


/** 
 * A chat widget. Contains a box with history and a text entry box.
 */
class SChatWidget : public SCompoundWidget, public ShooterHUDPCTrackerBase
{
public:
	SLATE_BEGIN_ARGS(SChatWidget)
		: _bAlwaysVisible(false)
		, _bDismissAfterSay(true)
	{}

	/** Should the chat widget be kept visible at all times */
	SLATE_ARGUMENT(bool, bAlwaysVisible)
	
	/** Should the chat widget be dismissed after 'say'.  */
	SLATE_ARGUMENT(bool, bDismissAfterSay)
		
	SLATE_END_ARGS()

	/** Needed for every widget */
	void Construct(const FArguments& InArgs, const FLocalPlayerContext& InContext);

	/** Gets the visibility of the entry widget. */
	EVisibility GetEntryVisibility() const;

	/**
	 * Sets the visibility of the chat widgets.
	 *
	 * @param	InVisibility	Required visibility.
	 */
	void SetEntryVisibility( TAttribute<EVisibility> InVisibility );

	/** 
	 * Add a new chat line.
	 *
	 * @param	ChatString		String to add.
	 * @param	SetFocus		Should the window be given focus
	 */
	void AddChatLine(const FText &ChatString, bool SetFocus);

	TSharedRef<class SWidget> AsWidget();

protected:

	// Struct to hold chat lines.
	struct FChatLine
	{
		// Source string of this chat message.
		FText ChatString;

		FChatLine(const FText& InChatString)
			: ChatString(InChatString)
		{
		}
	};

	/** Update function. Allows us to focus keyboard. */
	void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime );

	/** The UI sets up the appropriate mouse settings upon focus. */
	FReply OnFocusReceived( const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent );

	/** 
	 * Delegate called when the text is commited.
	 *
	 * @param	InText			The committed text.
	 * @Param	InCommitInfo	The type of commit (eg. Pressed enter, changed focus)
	 */
	void OnChatTextCommitted(const FText& InText, ETextCommit::Type InCommitInfo);

	/** Return the border color. */
	FSlateColor GetBorderColor() const;

	/** Return the font color. */
	FSlateColor GetChatLineColor() const;

	/** 
	 * Return the adjusted color based on whether the chatbox is visible
	 * 
	 * @param InColor	The color value to use
	 *
	 * @returns The color - with the alpha set to zero if the chatbox is not visible.
	 */
	FSlateColor GetStyleColor( const FLinearColor& InColor ) const;

	TSharedRef<ITableRow> GenerateChatRow(TSharedPtr<FChatLine> ChatLine, const TSharedRef<STableViewBase>& OwnerTable);

	/** Visibility of the entry widget previous frame. */
	EVisibility LastVisibility;
	
	/* If try we should set the focus when we change the visibility */
	uint32 bVisibiltyNeedsFocus:1;

	/** Time to show the chat lines as visible after receiving a chat message. */
	double ChatFadeTime;

	/** When we received our last chat message. */
	double LastChatLineTime;

	/** The edit text widget. */
	TSharedPtr< SEditableTextBox > ChatEditBox;

	/** The chat history list view. */
	TSharedPtr< SListView< TSharedPtr< FChatLine> > > ChatHistoryListView;

	/** The array of chat history. */
	TArray< TSharedPtr< FChatLine> > ChatHistory;

	/** Should this chatbox be kept visible. */
	uint32 bAlwaysVisible : 1;

	/** Should this chatbox be dismmised on sending a string. */
	uint32 bDismissAfterSay : 1;

	/** Style to use for this chat widget */
	const struct FShooterChatStyle *ChatStyle;

	/** Copy of the font used for chat, with the font fallback value modified */
	FSlateFontInfo ChatFont;
};



