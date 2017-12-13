// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterWelcomeMenu.h"
#include "ShooterStyle.h"
#include "SShooterConfirmationDialog.h"
#include "ShooterGameViewportClient.h"
#include "ShooterGameInstance.h"


#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

class SShooterWelcomeMenuWidget : public SCompoundWidget
{
	/** The menu that owns this widget. */
	FShooterWelcomeMenu* MenuOwner;

	/** Animate the text so that the screen isn't static, for console cert requirements. */
	FCurveSequence TextAnimation;

	/** The actual curve that animates the text. */
	FCurveHandle TextColorCurve;

	SLATE_BEGIN_ARGS( SShooterWelcomeMenuWidget )
	{}

	SLATE_ARGUMENT(FShooterWelcomeMenu*, MenuOwner)

	SLATE_END_ARGS()

	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

	/**
	 * Gets the text color based on the current state of the animation.
	 *
	 * @return The text color based on the current state of the animation.
	 */
	FSlateColor GetTextColor() const
	{
		return FSlateColor(FMath::Lerp(FLinearColor(0.0f,0.0f,0.0f,1.0f), FLinearColor(0.5f,0.5f,0.5f,1.0f), TextColorCurve.GetLerp()));
	}

	void Construct( const FArguments& InArgs )
	{
		MenuOwner = InArgs._MenuOwner;
		
		TextAnimation = FCurveSequence();
		const float AnimDuration = 1.5f;
		TextColorCurve = TextAnimation.AddCurve(0, AnimDuration, ECurveEaseFunction::QuadInOut);

		ChildSlot
		[
			SNew(SBorder)
			.Padding(30.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[ 
				SNew( SRichTextBlock )
#if PLATFORM_PS4
				.Text( LOCTEXT("PressStartPS4", "PRESS CROSS BUTTON TO PLAY" ) )
#elif PLATFORM_SWITCH
				.Text(LOCTEXT("PressStartSwitch", "PRESS <img src=\"ShooterGame.Switch.Right\"/> TO PLAY"))
#else
				.Text( LOCTEXT("PressStartXboxOne", "PRESS A TO PLAY" ) )
#endif
//				.ColorAndOpacity(this, &SShooterWelcomeMenuWidget::GetTextColor)
				.TextStyle( FShooterStyle::Get(), "ShooterGame.WelcomeScreen.WelcomeTextStyle" )
				.DecoratorStyleSet(&FShooterStyle::Get())
				+ SRichTextBlock::ImageDecorator()
			]
		];
	}

	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override
	{
		if(!TextAnimation.IsPlaying())
		{
			if(TextAnimation.IsAtEnd())
			{
				TextAnimation.PlayReverse(this->AsShared());
			}
			else
			{
				TextAnimation.Play(this->AsShared());
			}
		}
	}

	virtual FReply OnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override
	{
		return FReply::Handled();
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		const FKey Key = InKeyEvent.GetKey();
		if (Key == EKeys::Enter)
		{
			MenuOwner->HandleLoginUIClosed(TSharedPtr<const FUniqueNetId>(), 0);
		}
		else if (!MenuOwner->GetControlsLocked() && Key == EKeys::Virtual_Accept)
		{
			bool bSkipToMainMenu = true;

			{
				const auto OnlineSub = IOnlineSubsystem::Get();
				if (OnlineSub)
				{
					const auto IdentityInterface = OnlineSub->GetIdentityInterface();
					if (IdentityInterface.IsValid())
					{
						TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
						const bool bIsLicensed = GenericApplication->ApplicationLicenseValid();

						const auto LoginStatus = IdentityInterface->GetLoginStatus(InKeyEvent.GetUserIndex());
						if (LoginStatus == ELoginStatus::NotLoggedIn || !bIsLicensed)
						{
							// Show the account picker.
							const auto ExternalUI = OnlineSub->GetExternalUIInterface();
							if (ExternalUI.IsValid())
							{
								ExternalUI->ShowLoginUI(InKeyEvent.GetUserIndex(), false, true, FOnLoginUIClosedDelegate::CreateSP(MenuOwner, &FShooterWelcomeMenu::HandleLoginUIClosed));
								bSkipToMainMenu = false;
							}
						}
					}
				}
			}

			if (bSkipToMainMenu)
			{
				const auto OnlineSub = IOnlineSubsystem::Get();
				if (OnlineSub)
				{
					const auto IdentityInterface = OnlineSub->GetIdentityInterface();
					if (IdentityInterface.IsValid())
					{
						TSharedPtr<const FUniqueNetId> UserId = IdentityInterface->GetUniquePlayerId(InKeyEvent.GetUserIndex());
						// If we couldn't show the external login UI for any reason, or if the user is
						// already logged in, just advance to the main menu immediately.
						MenuOwner->HandleLoginUIClosed(UserId, InKeyEvent.GetUserIndex());
					}
				}
			}

			return FReply::Handled();
		}

		return FReply::Unhandled();
	}

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override
	{
		return FReply::Handled().ReleaseMouseCapture().SetUserFocus(SharedThis( this ), EFocusCause::SetDirectly, true);
	}
};

void FShooterWelcomeMenu::Construct( TWeakObjectPtr< UShooterGameInstance > InGameInstance )
{
	bControlsLocked = false;
	GameInstance = InGameInstance;
	PendingControllerIndex = -1;

	MenuWidget = SNew( SShooterWelcomeMenuWidget )
		.MenuOwner(this);	
}

void FShooterWelcomeMenu::AddToGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(MenuWidget.ToSharedRef());
		FSlateApplication::Get().SetKeyboardFocus(MenuWidget);
	}
}

void FShooterWelcomeMenu::RemoveFromGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidget.ToSharedRef());
	}
}

void FShooterWelcomeMenu::HandleLoginUIClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex)
{
	if ( !ensure( GameInstance.IsValid() ) )
	{
		return;
	}

	UShooterGameViewportClient* ShooterViewport = Cast<UShooterGameViewportClient>( GameInstance->GetGameViewportClient() );

	TSharedPtr<GenericApplication> GenericApplication = FSlateApplication::Get().GetPlatformApplication();
	const bool bIsLicensed = GenericApplication->ApplicationLicenseValid();

	// If they don't currently have a license, let them know, but don't let them proceed
	if (!bIsLicensed && ShooterViewport != NULL)
	{
		const FText StopReason	= NSLOCTEXT( "ProfileMessages", "NeedLicense", "The signed in users do not have a license for this game. Please purchase ShooterGame from the Xbox Marketplace or sign in a user with a valid license." );
		const FText OKButton	= NSLOCTEXT( "DialogButtons", "OKAY", "OK" );

		ShooterViewport->ShowDialog( 
			nullptr,
			EShooterDialogType::Generic,
			StopReason,
			OKButton,
			FText::GetEmpty(),
			FOnClicked::CreateRaw(this, &FShooterWelcomeMenu::OnConfirmGeneric),
			FOnClicked::CreateRaw(this, &FShooterWelcomeMenu::OnConfirmGeneric)
			);
		return;
	}

	PendingControllerIndex = ControllerIndex;

	if(UniqueId.IsValid())
	{
		// Next step, check privileges
		const auto OnlineSub = IOnlineSubsystem::Get();
		if (OnlineSub)
		{
			const auto IdentityInterface = OnlineSub->GetIdentityInterface();
			if (IdentityInterface.IsValid())
			{
				IdentityInterface->GetUserPrivilege(*UniqueId, EUserPrivileges::CanPlay, IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateSP(this, &FShooterWelcomeMenu::OnUserCanPlay));
			}
		}
	}
	else
	{
		// Show a warning that your progress won't be saved if you continue without logging in. 
		if (ShooterViewport != NULL)
		{
			ShooterViewport->ShowDialog( 
				nullptr,
				EShooterDialogType::Generic,
				NSLOCTEXT("ProfileMessages", "ProgressWillNotBeSaved", "If you continue without signing in, your progress will not be saved."),
				NSLOCTEXT("DialogButtons", "AContinue", "A - Continue"),
				NSLOCTEXT("DialogButtons", "BBack", "B - Back"),
				FOnClicked::CreateRaw(this, &FShooterWelcomeMenu::OnContinueWithoutSavingConfirm),
				FOnClicked::CreateRaw(this, &FShooterWelcomeMenu::OnConfirmGeneric)
			);
		}
	}
}

void FShooterWelcomeMenu::SetControllerAndAdvanceToMainMenu(const int ControllerIndex)
{
	if ( !ensure( GameInstance.IsValid() ) )
	{
		return;
	}

	ULocalPlayer * NewPlayerOwner = GameInstance->GetFirstGamePlayer();

	if ( NewPlayerOwner != nullptr && ControllerIndex != -1 )
	{
		NewPlayerOwner->SetControllerId(ControllerIndex);
		NewPlayerOwner->SetCachedUniqueNetId(NewPlayerOwner->GetUniqueNetIdFromCachedControllerId());

		// tell gameinstance to transition to main menu
		GameInstance->GotoState(ShooterGameInstanceState::MainMenu);
	}	
}

FReply FShooterWelcomeMenu::OnContinueWithoutSavingConfirm()
{
	if ( !ensure( GameInstance.IsValid() ) )
	{
		return FReply::Handled();
	}

	UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>( GameInstance->GetGameViewportClient() );

	if (ShooterViewport != NULL)
	{
		ShooterViewport->HideDialog();
	}

	SetControllerAndAdvanceToMainMenu(PendingControllerIndex);
	return FReply::Handled();
}

FReply FShooterWelcomeMenu::OnConfirmGeneric()
{
	if ( !ensure( GameInstance.IsValid() ) )
	{
		return FReply::Handled();
	}

	UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>( GameInstance->GetGameViewportClient() );

	if (ShooterViewport != NULL)
	{
		ShooterViewport->HideDialog();
	}

	return FReply::Handled();
}

void FShooterWelcomeMenu::OnUserCanPlay(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		SetControllerAndAdvanceToMainMenu(PendingControllerIndex);
	}
	else
	{
		UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>( GameInstance->GetGameViewportClient() );

		if ( ShooterViewport != NULL )
		{
			const FText ReturnReason = NSLOCTEXT("PrivilegeFailures", "CannotPlayAgeRestriction", "You cannot play this game due to age restrictions.");
			const FText OKButton = NSLOCTEXT("DialogButtons", "OKAY", "OK");

			ShooterViewport->ShowDialog( 
				nullptr,
				EShooterDialogType::Generic,
				ReturnReason,
				OKButton,
				FText::GetEmpty(),
				FOnClicked::CreateRaw(this, &FShooterWelcomeMenu::OnConfirmGeneric),
				FOnClicked::CreateRaw(this, &FShooterWelcomeMenu::OnConfirmGeneric)
			);
		}
	}
}

#undef LOCTEXT_NAMESPACE
