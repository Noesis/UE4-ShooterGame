// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "SShooterDemoList.h"
#include "SHeaderRow.h"
#include "ShooterStyle.h"
#include "CoreStyle.h"
#include "ShooterGameLoadingScreen.h"
#include "ShooterGameInstance.h"
#include "NetworkReplayStreaming.h"
#include "ShooterGameViewportClient.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

struct FDemoEntry
{
	FNetworkReplayStreamInfo StreamInfo;
	FString		Date;
	FString		Size;
	int32		ResultsIndex;
};

void SShooterDemoList::Construct(const FArguments& InArgs)
{
	PlayerOwner			= InArgs._PlayerOwner;
	OwnerWidget			= InArgs._OwnerWidget;
	bUpdatingDemoList	= false;
	StatusText			= FText::GetEmpty();
	
	EnumerateStreamsVersion = FNetworkVersion::GetReplayVersion();

	const int32 NameWidth		= 280;
	const int32 ViewersWidth	= 64;
	const int32 DateWidth		= 210;
	const int32 LengthWidth		= 64;

	ChildSlot
	.VAlign(VAlign_Fill)
	.HAlign(HAlign_Fill)
	[
		SNew(SVerticalBox)
		+SVerticalBox::Slot()
		.HAlign(HAlign_Left)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
		[
			SNew(SCheckBox)
			.IsChecked(this, &SShooterDemoList::IsShowAllReplaysChecked)
			.OnCheckStateChanged(this, &SShooterDemoList::OnShowAllReplaysChecked)
			.Style(FCoreStyle::Get(), "Checkbox")
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ShowAllDemos", "Show demos from all versions"))
				.TextStyle(FShooterStyle::Get(), "ShooterGame.DemoListCheckboxTextStyle")
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)  
			.WidthOverride(700)
			.HeightOverride(300)
			[
				SAssignNew(DemoListWidget, SListView<TSharedPtr<FDemoEntry>>)
				.ItemHeight(20)
				.ListItemsSource(&DemoList)
				.SelectionMode(ESelectionMode::Single)
				.OnGenerateRow(this, &SShooterDemoList::MakeListViewWidget)
				.OnSelectionChanged(this, &SShooterDemoList::EntrySelectionChanged)
				.OnMouseButtonDoubleClick(this,&SShooterDemoList::OnListItemDoubleClicked)
				.HeaderRow(
					SNew(SHeaderRow)
					+ SHeaderRow::Column("DemoName").FixedWidth(NameWidth).DefaultLabel(NSLOCTEXT("DemoList", "DemoNameColumn", "Demo Name"))
					+ SHeaderRow::Column("Viewers").FixedWidth(ViewersWidth).DefaultLabel(NSLOCTEXT("Viewers", "ViewersColumn", "Viewers"))
					+ SHeaderRow::Column("Date").FixedWidth(DateWidth).DefaultLabel(NSLOCTEXT("DemoList", "DateColumn", "Date"))
					+ SHeaderRow::Column("Length").FixedWidth(LengthWidth).DefaultLabel(NSLOCTEXT("Length", "LengthColumn", "Length"))
					+ SHeaderRow::Column("Size").HAlignHeader(HAlign_Left).HAlignCell(HAlign_Right).DefaultLabel(NSLOCTEXT("DemoList", "SizeColumn", "Size")))
			]
		]
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SOverlay)
			+SOverlay::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SShooterDemoList::GetBottomText)
				.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuServerListTextStyle")
			]
		]
	];

	ReplayStreamer = FNetworkReplayStreaming::Get().GetFactory().CreateReplayStreamer();

	BuildDemoList();
}

void SShooterDemoList::OnEnumerateStreamsComplete(const TArray<FNetworkReplayStreamInfo>& Streams)
{
	check(bUpdatingDemoList); // should not be called otherwise

	bool bFinished = true;

	for ( const auto& StreamInfo : Streams )
	{
		float SizeInKilobytes = StreamInfo.SizeInBytes / 1024.0f;

		TSharedPtr<FDemoEntry> NewDemoEntry = MakeShareable( new FDemoEntry() );

		NewDemoEntry->StreamInfo	= StreamInfo;
		NewDemoEntry->Date			= StreamInfo.Timestamp.ToString( TEXT( "%m/%d/%Y %h:%M %A" ) );	// UTC time
		NewDemoEntry->Size			= SizeInKilobytes >= 1024.0f ? FString::Printf( TEXT("%2.2f MB" ), SizeInKilobytes / 1024.0f ) : FString::Printf( TEXT("%i KB" ), (int)SizeInKilobytes );

		DemoList.Add( NewDemoEntry );
	}

	// Sort demo names by date
	struct FCompareDateTime
	{
		FORCEINLINE bool operator()( const TSharedPtr<FDemoEntry> & A, const TSharedPtr<FDemoEntry> & B ) const
		{
			return A->StreamInfo.Timestamp.GetTicks() > B->StreamInfo.Timestamp.GetTicks();
		}
	};

	Sort( DemoList.GetData(), DemoList.Num(), FCompareDateTime() );

	//StatusText = "";
	StatusText = LOCTEXT("DemoSelectionInfo","Press ENTER to Play. Press DEL to delete.");

	if ( bFinished )
	{		
		OnBuildDemoListFinished();
	}
}

FText SShooterDemoList::GetBottomText() const
{
	 return StatusText;
}

/**
 * Ticks this widget.  Override in derived classes, but always call the parent implementation.
 *
 * @param  AllottedGeometry The space allotted for this widget
 * @param  InCurrentTime  Current absolute real time
 * @param  InDeltaTime  Real time passed since last tick
 */
void SShooterDemoList::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
}

ECheckBoxState SShooterDemoList::IsShowAllReplaysChecked() const
{
	if (EnumerateStreamsVersion.NetworkVersion == 0 && EnumerateStreamsVersion.Changelist == 0)
	{
		return ECheckBoxState::Checked;
	}

	return ECheckBoxState::Unchecked;
}

void SShooterDemoList::OnShowAllReplaysChecked(ECheckBoxState NewCheckedState)
{
	EnumerateStreamsVersion = FNetworkVersion::GetReplayVersion();

	// Always set CL to 0, we only want to ever check NetworkVersion (now that we have backwards compat)
	EnumerateStreamsVersion.Changelist = 0;

	if (NewCheckedState == ECheckBoxState::Checked)
	{
		EnumerateStreamsVersion.NetworkVersion = 0;
	}

	BuildDemoList();
}

/** Populates the demo list */
void SShooterDemoList::BuildDemoList()
{
	bUpdatingDemoList = true;
	DemoList.Empty();

	if ( ReplayStreamer.IsValid() )
	{
		ReplayStreamer->EnumerateStreams(EnumerateStreamsVersion, FString(), FString(), FOnEnumerateStreamsComplete::CreateSP(this, &SShooterDemoList::OnEnumerateStreamsComplete));
	}
}

/** Called when demo list building is finished */
void SShooterDemoList::OnBuildDemoListFinished()
{
	bUpdatingDemoList = false;

	int32 SelectedItemIndex = DemoList.IndexOfByKey(SelectedItem);

	DemoListWidget->RequestListRefresh();
	if (DemoList.Num() > 0)
	{
		DemoListWidget->UpdateSelectionSet();
		DemoListWidget->SetSelection(DemoList[SelectedItemIndex > -1 ? SelectedItemIndex : 0],ESelectInfo::OnNavigation);
	}
}

void SShooterDemoList::PlayDemo()
{
	if (bUpdatingDemoList)
	{
		// We're still building the list
		return;
	}

	if (SelectedItem.IsValid())
	{
		UShooterGameInstance* const GI = Cast<UShooterGameInstance>(PlayerOwner->GetGameInstance());

		if ( GI != NULL )
		{
			FString DemoName = SelectedItem->StreamInfo.Name;

			// Play the demo
			GI->PlayDemo( PlayerOwner.Get(), DemoName );
		}
	}
}

void SShooterDemoList::DeleteDemo()
{
	if (bUpdatingDemoList)
	{
		// We're still building the list
		return;
	}

	if (SelectedItem.IsValid())
	{
		UShooterGameInstance* const GI = Cast<UShooterGameInstance>(PlayerOwner->GetGameInstance());

		if ( GI != NULL )
		{
			UShooterGameViewportClient* ShooterViewport = Cast<UShooterGameViewportClient>( GI->GetGameViewportClient() );

			if ( ShooterViewport )
			{
				ShooterViewport->ShowDialog( 
					PlayerOwner,
					EShooterDialogType::Generic,
					FText::Format(LOCTEXT("DeleteDemoFmt", "Delete {0}?"), FText::FromString(SelectedItem->StreamInfo.FriendlyName)),
					LOCTEXT("EnterYes", "ENTER - YES"),
					LOCTEXT("EscapeNo", "ESC - NO"),
					FOnClicked::CreateRaw(this, &SShooterDemoList::OnDemoDeleteConfirm),
					FOnClicked::CreateRaw(this, &SShooterDemoList::OnDemoDeleteCancel)
				);
			}
		}
	}
}

FReply SShooterDemoList::OnDemoDeleteConfirm()
{
	if (SelectedItem.IsValid() && ReplayStreamer.IsValid())
	{
		bUpdatingDemoList = true;
		DemoList.Empty();

		ReplayStreamer->DeleteFinishedStream(SelectedItem->StreamInfo.Name, FOnDeleteFinishedStreamComplete::CreateSP(this, &SShooterDemoList::OnDeleteFinishedStreamComplete));
	}

	UShooterGameInstance* const GI = Cast<UShooterGameInstance>(PlayerOwner->GetGameInstance());

	if ( GI != NULL )
	{
		UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>( GI->GetGameViewportClient() );

		if ( ShooterViewport )
		{
			ShooterViewport->HideDialog();
		}
	}

	return FReply::Handled();
}

FReply SShooterDemoList::OnDemoDeleteCancel()
{
	UShooterGameInstance* const GI = Cast<UShooterGameInstance>(PlayerOwner->GetGameInstance());

	if ( GI != NULL )
	{
		UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>( GI->GetGameViewportClient() );

		if ( ShooterViewport )
		{
			ShooterViewport->HideDialog();
		}
	}

	return FReply::Handled();
}

void SShooterDemoList::OnDeleteFinishedStreamComplete(bool bWasSuccessful)
{
	BuildDemoList();
}

void SShooterDemoList::OnFocusLost(const FFocusEvent& InFocusEvent)
{
	if (InFocusEvent.GetCause() != EFocusCause::SetDirectly)
	{
		FSlateApplication::Get().SetKeyboardFocus(SharedThis( this ));
	}
}

FReply SShooterDemoList::OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent)
{
	return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly, true);
}

void SShooterDemoList::EntrySelectionChanged(TSharedPtr<FDemoEntry> InItem, ESelectInfo::Type SelectInfo)
{
	SelectedItem = InItem;
}

void SShooterDemoList::OnListItemDoubleClicked(TSharedPtr<FDemoEntry> InItem)
{
	SelectedItem = InItem;
	PlayDemo();
	FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
}

void SShooterDemoList::MoveSelection(int32 MoveBy)
{
	const int32 SelectedItemIndex = DemoList.IndexOfByKey(SelectedItem);

	if (SelectedItemIndex+MoveBy > -1 && SelectedItemIndex+MoveBy < DemoList.Num())
	{
		DemoListWidget->SetSelection(DemoList[SelectedItemIndex+MoveBy]);
	}
}

FReply SShooterDemoList::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyboardEvent) 
{
	if (bUpdatingDemoList) // lock input
	{
		return FReply::Handled();
	}

	FReply Result = FReply::Unhandled();
	const FKey Key = InKeyboardEvent.GetKey();
	if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
	{
		PlayDemo();
		Result = FReply::Handled();
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	}
	else if (Key == EKeys::Delete)
	{
		DeleteDemo();
		Result = FReply::Handled();
		FSlateApplication::Get().SetKeyboardFocus(SharedThis(this));
	}
	//hit space bar or left gamepad face button to search for demos again / refresh the list, only when not searching already
	else if (Key == EKeys::SpaceBar || Key == EKeys::Gamepad_FaceButton_Left)
	{
		// Refresh demo list
		BuildDemoList();
	}
	else if (Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_LeftStick_Up)
	{
		MoveSelection(-1);
		Result = FReply::Handled();
	}
	else if (Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_LeftStick_Down)
	{
		MoveSelection(1);
		Result = FReply::Handled();
	}
	return Result;
}

TSharedRef<ITableRow> SShooterDemoList::MakeListViewWidget(TSharedPtr<FDemoEntry> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	class SDemoEntryWidget : public SMultiColumnTableRow< TSharedPtr<FDemoEntry> >
	{
	public:
		SLATE_BEGIN_ARGS(SDemoEntryWidget){}
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTable, TSharedPtr<FDemoEntry> InItem)
		{
			Item = InItem;
			SMultiColumnTableRow< TSharedPtr<FDemoEntry> >::Construct(FSuperRowType::FArguments(), InOwnerTable);
		}

		TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName)
		{
			FText ItemText = FText::GetEmpty();

			if (ColumnName == "DemoName")
			{
				FString NameString = Item->StreamInfo.FriendlyName.IsEmpty() ? Item->StreamInfo.Name : Item->StreamInfo.FriendlyName;

				const int MAX_DEMO_NAME_DISPLAY_LEN = 18;
				if ( NameString.Len() > MAX_DEMO_NAME_DISPLAY_LEN )
				{
					NameString = NameString.Left( MAX_DEMO_NAME_DISPLAY_LEN ) + TEXT( "..." );
				}

				if (Item->StreamInfo.bIsLive)
				{
					NameString += " (Live)";
				}

				ItemText = FText::FromString(NameString);
			}
			else if (ColumnName == "Viewers")
			{
				ItemText = FText::FromString( FString::Printf( TEXT( "%i" ), Item->StreamInfo.NumViewers ) );
			}
			else if (ColumnName == "Date")
			{
				ItemText = FText::FromString(Item->Date);
			}
			else if (ColumnName == "Length")
			{
				const int32 Minutes = Item->StreamInfo.LengthInMS / ( 1000 * 60 );
				const int32 Seconds = ( Item->StreamInfo.LengthInMS / 1000 ) % 60;

				ItemText = FText::FromString( FString::Printf( TEXT( "%02i:%02i" ), Minutes, Seconds ) );
			}
			else if (ColumnName == "Size")
			{
				ItemText = FText::FromString(Item->Size);
			}

			return SNew(STextBlock)
				.Text(ItemText)
				.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuServerListTextStyle");
		}
		TSharedPtr<FDemoEntry> Item;
	};
	return SNew(SDemoEntryWidget, OwnerTable, Item);
}

#undef LOCTEXT_NAMESPACE