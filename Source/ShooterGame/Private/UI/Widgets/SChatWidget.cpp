// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "SChatWidget.h"
#include "ShooterStyle.h"
#include "ShooterChatWidgetStyle.h"

#define CHAT_BOX_WIDTH 576.0f
#define CHAT_BOX_HEIGHT 192.0f
#define CHAT_BOX_PADDING 20.0f

void SChatWidget::Construct(const FArguments& InArgs, const FLocalPlayerContext& InContext)
{
	ShooterHUDPCTrackerBase::Init(InContext);

	ChatStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterChatStyle>("DefaultShooterChatStyle");

	bAlwaysVisible = InArgs._bAlwaysVisible;
	bDismissAfterSay = InArgs._bDismissAfterSay;

	ChatFadeTime = 10.0;
	LastChatLineTime = -1.0;
	bVisibiltyNeedsFocus = true;

	//some constant values
	const int32 PaddingValue = 2;

	// Copy the font we'll be using for chat, and limit the font fallback to localized only, for performance reasons
	ChatFont = FShooterStyle::Get().GetFontStyle("ShooterGame.ChatFont");

	ChatFont.FontFallback = EFontFallback::FF_LocalizedFallback;


	// Initialize Menu
	ChildSlot
	.HAlign(HAlign_Left)
	.VAlign(VAlign_Center)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		// The main background
		.AutoHeight()
		[			
			SNew(SBorder)
			.BorderImage(&ChatStyle->BackingBrush)
			.Padding(FMargin(CHAT_BOX_PADDING, 16.0f, CHAT_BOX_PADDING, 24.0f))
			.BorderBackgroundColor(this, &SChatWidget::GetBorderColor)
			[
				SNew(SBox)
				.HeightOverride(CHAT_BOX_HEIGHT)
				.WidthOverride(CHAT_BOX_WIDTH)
				[
					SAssignNew(ChatHistoryListView, SListView< TSharedPtr<FChatLine> >)
					.SelectionMode(ESelectionMode::None)
					.ListItemsSource(&ChatHistory)
					.OnGenerateRow(this, &SChatWidget::GenerateChatRow)
				]
			]
		]
		// Chat input
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.WidthOverride(CHAT_BOX_WIDTH)
			.Padding(FMargin(11.0f, 0.0f))
			[
				SAssignNew(ChatEditBox, SEditableTextBox)
				.OnTextCommitted(this, &SChatWidget::OnChatTextCommitted)
				.MinDesiredWidth(CHAT_BOX_WIDTH)
				.ClearKeyboardFocusOnCommit(false)
				.HintText(NSLOCTEXT("ChatWidget", "SaySomething", "Say Something..."))
				.Font(ChatFont)
				.Style(&ChatStyle->TextEntryStyle)
			]
		]
	];
	// Setup visibilty
	LastVisibility = bAlwaysVisible ? EVisibility::Visible : EVisibility::Hidden;
	SetEntryVisibility( LastVisibility );	
}


FSlateColor SChatWidget::GetBorderColor() const
{
	return GetStyleColor(ChatStyle->BoxBorderColor.GetSpecifiedColor());
}

FSlateColor SChatWidget::GetChatLineColor() const
{
	return GetStyleColor(ChatStyle->TextColor.GetSpecifiedColor());
}

FSlateColor SChatWidget::GetStyleColor( const FLinearColor& InColor ) const
{
	const double EndTime =  LastChatLineTime + ChatFadeTime;
	const double CurrentTime = FSlateApplication::Get().GetCurrentTime();

	// Get the requested color.on
	FLinearColor ReturnColor = InColor;

	// Set the alpha to zero if we are not visible. (We could also fade out here if the time has ALMOST expired).
	if (bAlwaysVisible == false)
	{
		if ((ChatEditBox->GetVisibility() == EVisibility::Hidden) || (CurrentTime > EndTime))
		{
			ReturnColor.A = 0.0f;
		}
	}

	return FSlateColor( ReturnColor );
}

void SChatWidget::AddChatLine(const FText& ChatString, bool SetFocus)
{
	ChatHistory.Add(MakeShareable(new FChatLine(ChatString)));

	if(ChatHistoryListView.IsValid())
	{
		FChatLine* LastLine  = ChatHistory[ChatHistory.Num() - 1].Get();
		UE_LOG(LogOnline, Warning, TEXT("request scroll last=%s"), *LastLine->ChatString.ToString());
		ChatHistoryListView->RequestScrollIntoView(ChatHistory[ChatHistory.Num() - 1]);
	}
	
	FSlateApplication::Get().PlaySound(ChatStyle->RxMessgeSound);
	SetEntryVisibility( EVisibility::Visible );
	bVisibiltyNeedsFocus = SetFocus;
}

EVisibility SChatWidget::GetEntryVisibility() const
{
	return ChatEditBox->GetVisibility();
}

void SChatWidget::SetEntryVisibility( TAttribute<EVisibility> InVisibility )
{
	// If we are making it visible reset the 'start' time
	if( InVisibility.Get() == EVisibility::Visible )
	{
		LastChatLineTime = FSlateApplication::Get().GetCurrentTime();
	}

	ChatEditBox->SetVisibility(InVisibility);
	ChatHistoryListView->SetVisibility(InVisibility);
	SetVisibility(InVisibility);
}

void SChatWidget::OnChatTextCommitted(const FText& InText, ETextCommit::Type InCommitInfo)
{
	if (InCommitInfo == ETextCommit::OnEnter)
	{
		if (GetPlayerController().IsValid() && !InText.IsEmpty())
		{
			// broadcast chat to other players
			GetPlayerController()->Say(InText.ToString());

			if(ChatEditBox.IsValid())
			{
				// Add the string so we see it too (we will ignore our own strings in the receive function)
				AddChatLine( InText, true );

				// Clear the text
				ChatEditBox->SetText(FText());

				// Audible indication we sent a message
				FSlateApplication::Get().PlaySound(ChatStyle->TxMessgeSound);
			}
		}
	}

 	// If we want to dismiss chat after say, and we are not always visible, hide it now.
	if ((bAlwaysVisible == false) && ( bDismissAfterSay == true ) )
 	{
 		SetEntryVisibility(EVisibility::Hidden);
 	}
}

void SChatWidget::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	// Always tick the super.
	SCompoundWidget::Tick( AllottedGeometry, InCurrentTime, InDeltaTime );

	// If we have not got the keep visible flag set, and the fade time has expired hide the widget
	const double CurrentTime = FSlateApplication::Get().GetCurrentTime();
	if( ( bAlwaysVisible == false ) && ( CurrentTime > ( LastChatLineTime + ChatFadeTime ) ) )
	{
		if (LastVisibility != EVisibility::Hidden)
		{
			SetEntryVisibility(EVisibility::Hidden);
		}
		LastVisibility = GetEntryVisibility();
	}

	// Update the visibility.
	if (GetEntryVisibility() != LastVisibility)
	{
		LastVisibility = GetEntryVisibility();
		if (LastVisibility == EVisibility::Visible)
		{
			// Enter UI mode
			FSlateApplication::Get().SetKeyboardFocus( SharedThis(this) );

			if (ChatEditBox.IsValid())
			{
				FWidgetPath WidgetToFocusPath;
				
				bool bFoundPath = FSlateApplication::Get().FindPathToWidget(ChatEditBox.ToSharedRef(), WidgetToFocusPath);
				if (bFoundPath && WidgetToFocusPath.IsValid() && bVisibiltyNeedsFocus == true )
				{
					FSlateApplication::Get().SetKeyboardFocus(WidgetToFocusPath, EFocusCause::SetDirectly);
				}
			}
		}
		else
		{
			// Exit UI mode
 			FSlateApplication::Get().SetAllUserFocusToGameViewport();
		}
	}
	FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetType(EMouseCursor::None);
}


FReply SChatWidget::OnFocusReceived( const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent )
{
	return FReply::Handled().ReleaseMouseCapture().LockMouseToWidget( SharedThis( this ) );
}

TSharedRef<ITableRow> SChatWidget::GenerateChatRow(TSharedPtr<FChatLine> ChatLine, const TSharedRef<STableViewBase>& OwnerTable)
{
	return
		SNew(STableRow< TSharedPtr< FChatLine> >, OwnerTable )
		[
			SNew(STextBlock)
			.Text(ChatLine->ChatString)
			.Font(ChatFont)
			.ColorAndOpacity(this, &SChatWidget::GetChatLineColor)
			.WrapTextAt(CHAT_BOX_WIDTH - CHAT_BOX_PADDING)
		];
}


TSharedRef<SWidget> SChatWidget::AsWidget()
{
	return SharedThis(this);
}

