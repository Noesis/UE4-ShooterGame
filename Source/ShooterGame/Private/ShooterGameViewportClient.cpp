// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGameViewportClient.h"
#include "SShooterConfirmationDialog.h"
#include "SSafeZone.h"
#include "SThrobber.h"
#include "Player/ShooterLocalPlayer.h"

UShooterGameViewportClient::UShooterGameViewportClient(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetSuppressTransitionMessage(true);
}

void UShooterGameViewportClient::NotifyPlayerAdded(int32 PlayerIndex, ULocalPlayer* AddedPlayer)
{
	Super::NotifyPlayerAdded(PlayerIndex, AddedPlayer);

 	UShooterLocalPlayer* const ShooterLP = Cast<UShooterLocalPlayer>(AddedPlayer);
 	if (ShooterLP)
 	{
 		ShooterLP->LoadPersistentUser();
 	}
}

void UShooterGameViewportClient::AddViewportWidgetContent( TSharedRef<class SWidget> ViewportContent, const int32 ZOrder )
{
	UE_LOG( LogPlayerManagement, Log, TEXT( "UShooterGameViewportClient::AddViewportWidgetContent: %p" ), &ViewportContent.Get() );

	if ( ( DialogWidget.IsValid() || LoadingScreenWidget.IsValid() ) && ViewportContent != DialogWidget && ViewportContent != LoadingScreenWidget )
	{
		// Add to hidden list, and don't show until we hide the dialog widget
		HiddenViewportContentStack.AddUnique( ViewportContent );
		return;
	}

	if ( ViewportContentStack.Contains( ViewportContent ) )
	{
		return;
	}

	ViewportContentStack.AddUnique( ViewportContent );

	Super::AddViewportWidgetContent( ViewportContent, 0 );
}

void UShooterGameViewportClient::RemoveViewportWidgetContent( TSharedRef<class SWidget> ViewportContent )
{
	UE_LOG( LogPlayerManagement, Log, TEXT( "UShooterGameViewportClient::RemoveViewportWidgetContent: %p" ), &ViewportContent.Get()  );

	ViewportContentStack.Remove( ViewportContent );
	HiddenViewportContentStack.Remove( ViewportContent );

	Super::RemoveViewportWidgetContent( ViewportContent );
}

void UShooterGameViewportClient::HideExistingWidgets()
{
	check( HiddenViewportContentStack.Num() == 0 );

	TArray<TSharedRef<class SWidget>> CopyOfViewportContentStack = ViewportContentStack;

	for ( int32 i = ViewportContentStack.Num() - 1; i >= 0; i-- )
	{
		RemoveViewportWidgetContent( ViewportContentStack[i] );
	}

	HiddenViewportContentStack = CopyOfViewportContentStack;
}

void UShooterGameViewportClient::ShowExistingWidgets()
{
	// We shouldn't have any visible widgets at this point
	check( ViewportContentStack.Num() == 0 );

	// Unhide all of the previously hidden widgets
	for ( int32 i = 0; i < HiddenViewportContentStack.Num(); i++ )
	{
		AddViewportWidgetContent( HiddenViewportContentStack[i] );
	}

	check( ViewportContentStack.Num() == HiddenViewportContentStack.Num() );

	// Done with these
	HiddenViewportContentStack.Empty();
}

void UShooterGameViewportClient::ShowDialog(TWeakObjectPtr<ULocalPlayer> PlayerOwner, EShooterDialogType::Type DialogType, const FText& Message, const FText& Confirm, const FText& Cancel, const FOnClicked& OnConfirm, const FOnClicked& OnCancel)
{
	UE_LOG( LogPlayerManagement, Log, TEXT( "UShooterGameViewportClient::ShowDialog..." ) );

	if ( DialogWidget.IsValid() )
	{
		return;	// Already showing a dialog box
	}

	// Hide all existing widgets
	if ( !LoadingScreenWidget.IsValid() )
	{
		HideExistingWidgets();
	}

	DialogWidget = SNew( SShooterConfirmationDialog )
		.PlayerOwner(PlayerOwner)
		.DialogType(DialogType)
		.MessageText(Message)
		.ConfirmText(Confirm)
		.CancelText(Cancel)
		.OnConfirmClicked(OnConfirm)
		.OnCancelClicked(OnCancel);

	if ( LoadingScreenWidget.IsValid() )
	{
		// Can't show dialog while loading screen is visible
		HiddenViewportContentStack.Add( DialogWidget.ToSharedRef() );
	}
	else
	{
		AddViewportWidgetContent( DialogWidget.ToSharedRef() );

		// Remember what widget currently has focus
		OldFocusWidget = FSlateApplication::Get().GetKeyboardFocusedWidget();

		// Force focus to the dialog widget
		FSlateApplication::Get().SetKeyboardFocus( DialogWidget, EFocusCause::SetDirectly );
	}
}

void UShooterGameViewportClient::HideDialog()
{
	UE_LOG( LogPlayerManagement, Log, TEXT( "UShooterGameViewportClient::HideDialog. DialogWidget: %p, OldFocusWidget: %p" ), DialogWidget.Get(), OldFocusWidget.Get() );

	if ( DialogWidget.IsValid() )
	{
		const bool bRestoreOldFocus = OldFocusWidget.IsValid() && FSlateApplication::Get().GetKeyboardFocusedWidget() == DialogWidget;

		// Hide the dialog widget
		RemoveViewportWidgetContent( DialogWidget.ToSharedRef() );

		// Destroy the dialog widget
		DialogWidget = NULL;

		if ( !LoadingScreenWidget.IsValid() )
		{
			ShowExistingWidgets();
		}

		// Restore focus to last widget (but only if the dialog currently has focus still)
		if ( bRestoreOldFocus )
		{
			FSlateApplication::Get().SetKeyboardFocus( OldFocusWidget, EFocusCause::SetDirectly );
		}

		OldFocusWidget = NULL;
	}
}

void UShooterGameViewportClient::ShowLoadingScreen()
{
	if ( LoadingScreenWidget.IsValid() )
	{
		return;
	}

	if ( DialogWidget.IsValid() )
	{
		// Hide the dialog widget (loading screen takes priority)
		check( !HiddenViewportContentStack.Contains( DialogWidget.ToSharedRef() ) );
		check( ViewportContentStack.Contains( DialogWidget.ToSharedRef() ) );
		RemoveViewportWidgetContent( DialogWidget.ToSharedRef() );
		HiddenViewportContentStack.Add( DialogWidget.ToSharedRef() );
	}
	else
	{
		// Hide all existing widgets
		HideExistingWidgets();
	}

	LoadingScreenWidget = SNew( SShooterLoadingScreen );

	AddViewportWidgetContent( LoadingScreenWidget.ToSharedRef() );
}

void UShooterGameViewportClient::HideLoadingScreen()
{
	if ( !LoadingScreenWidget.IsValid() )
	{
		return;
	}

	RemoveViewportWidgetContent( LoadingScreenWidget.ToSharedRef() );

	LoadingScreenWidget = NULL;

	// Show the dialog widget if we need to
	if ( DialogWidget.IsValid() )
	{
		check( HiddenViewportContentStack.Contains( DialogWidget.ToSharedRef() ) );
		check( !ViewportContentStack.Contains( DialogWidget.ToSharedRef() ) );
		HiddenViewportContentStack.Remove( DialogWidget.ToSharedRef() );
		AddViewportWidgetContent( DialogWidget.ToSharedRef() );
	}
	else
	{
		ShowExistingWidgets();
	}
}

EShooterDialogType::Type UShooterGameViewportClient::GetDialogType() const
{
	return (DialogWidget.IsValid() ? DialogWidget->DialogType : EShooterDialogType::None);
}

TWeakObjectPtr<ULocalPlayer> UShooterGameViewportClient::GetDialogOwner() const
{
	return (DialogWidget.IsValid() ? DialogWidget->PlayerOwner : nullptr);
}

void UShooterGameViewportClient::Tick(float DeltaSeconds)
{
	if ( DialogWidget.IsValid() && !LoadingScreenWidget.IsValid() )
	{
		// Make sure the dialog widget always has focus
		if ( FSlateApplication::Get().GetKeyboardFocusedWidget() != DialogWidget )
		{
			// Remember which widget had focus before we override it
			OldFocusWidget = FSlateApplication::Get().GetKeyboardFocusedWidget();

			// Force focus back to dialog
			FSlateApplication::Get().SetKeyboardFocus( DialogWidget, EFocusCause::SetDirectly );
		}
	}
}

#if WITH_EDITOR
void UShooterGameViewportClient::DrawTransition(UCanvas* Canvas)
{
	if (GetOuterUEngine() != NULL)
	{
		TEnumAsByte<enum ETransitionType> Type = GetOuterUEngine()->TransitionType;
		switch (Type)
		{
		case TT_Connecting:
			DrawTransitionMessage(Canvas, NSLOCTEXT("GameViewportClient", "ConnectingMessage", "CONNECTING").ToString());
			break;
		case TT_WaitingToConnect:
			DrawTransitionMessage(Canvas, NSLOCTEXT("GameViewportClient", "Waitingtoconnect", "Waiting to connect...").ToString());
			break;	
		}
	}
}
#endif //WITH_EDITOR

void SShooterLoadingScreen::Construct(const FArguments& InArgs)
{
	static const FName LoadingScreenName(TEXT("/Game/UI/Menu/LoadingScreen.LoadingScreen"));

	//since we are not using game styles here, just load one image
	LoadingScreenBrush = MakeShareable( new FShooterGameLoadingScreenBrush( LoadingScreenName, FVector2D(1920,1080) ) );

	ChildSlot
	[
		SNew(SOverlay)
		+SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SImage)
			.Image(LoadingScreenBrush.Get())
		]
		+SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SSafeZone)
			.VAlign(VAlign_Bottom)
			.HAlign(HAlign_Right)
			.Padding(10.0f)
			.IsTitleSafe(true)
			[
				SNew(SThrobber)
				.Visibility(this, &SShooterLoadingScreen::GetLoadIndicatorVisibility)
			]
		]
	];
}
