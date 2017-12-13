// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterStyle.h"
#include "SShooterConfirmationDialog.h"
#include "ShooterMessageMenu.h"
#include "ShooterGameViewportClient.h"
#include "ShooterGameInstance.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void FShooterMessageMenu::Construct(TWeakObjectPtr<UShooterGameInstance> InGameInstance, TWeakObjectPtr<ULocalPlayer> InPlayerOwner, const FText& Message, const FText& OKButtonText, const FText& CancelButtonText, const FName& InPendingNextState)
{
	GameInstance			= InGameInstance;
	PlayerOwner				= InPlayerOwner;
	PendingNextState		= InPendingNextState;

	if ( ensure( GameInstance.IsValid() ) )
	{
		UShooterGameViewportClient* ShooterViewport = Cast<UShooterGameViewportClient>( GameInstance->GetGameViewportClient() );

		if ( ShooterViewport )
		{
			// Hide the previous dialog
			ShooterViewport->HideDialog();

			// Show the new one
			ShooterViewport->ShowDialog( 
				PlayerOwner,
				EShooterDialogType::Generic,
				Message, 
				OKButtonText, 
				CancelButtonText, 
				FOnClicked::CreateRaw(this, &FShooterMessageMenu::OnClickedOK),
				FOnClicked::CreateRaw(this, &FShooterMessageMenu::OnClickedCancel)
			);
		}
	}
}

void FShooterMessageMenu::RemoveFromGameViewport()
{
	if ( ensure( GameInstance.IsValid() ) )
	{
		UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>( GameInstance->GetGameViewportClient() );

		if ( ShooterViewport )
		{
			// Hide the previous dialog
			ShooterViewport->HideDialog();
		}
	}
}

void FShooterMessageMenu::HideDialogAndGotoNextState()
{
	RemoveFromGameViewport();

	if ( ensure( GameInstance.IsValid() ) )
	{
		GameInstance->GotoState( PendingNextState );
	}
};

FReply FShooterMessageMenu::OnClickedOK()
{
	OKButtonDelegate.ExecuteIfBound();
	HideDialogAndGotoNextState();
	return FReply::Handled();
}

FReply FShooterMessageMenu::OnClickedCancel()
{
	CancelButtonDelegate.ExecuteIfBound();
	HideDialogAndGotoNextState();
	return FReply::Handled();
}

void FShooterMessageMenu::SetOKClickedDelegate(FMessageMenuButtonClicked InButtonDelegate)
{
	OKButtonDelegate = InButtonDelegate;
}

void FShooterMessageMenu::SetCancelClickedDelegate(FMessageMenuButtonClicked InButtonDelegate)
{
	CancelButtonDelegate = InButtonDelegate;
}


#undef LOCTEXT_NAMESPACE
