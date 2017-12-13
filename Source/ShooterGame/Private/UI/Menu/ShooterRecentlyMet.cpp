// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterRecentlyMet.h"
#include "ShooterTypes.h"
#include "ShooterStyle.h"
#include "ShooterOptionsWidgetStyle.h"
#include "ShooterGameUserSettings.h"
#include "ShooterPersistentUser.h"
#include "Player/ShooterLocalPlayer.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void FShooterRecentlyMet::Construct(ULocalPlayer* _PlayerOwner, int32 LocalUserNum_)
{
	RecentlyMetStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterOptionsStyle>("DefaultShooterOptionsStyle");

	PlayerOwner = _PlayerOwner;
	LocalUserNum = LocalUserNum_;
	CurrRecentlyMetIndex = 0;
	MinRecentlyMetIndex = 0;
	MaxRecentlyMetIndex = 0; //initialized after the recently met list is (read in/set)

	/** Recently Met menu items */
	RecentlyMetRoot = FShooterMenuItem::CreateRoot();
	RecentlyMetItem = MenuHelper::AddMenuItem(RecentlyMetRoot, LOCTEXT("Recently Met", "RECENTLY MET"));

	/** Init online items */
	OnlineSub = IOnlineSubsystem::Get();

	UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());	
}

void FShooterRecentlyMet::OnApplySettings()
{
	ApplySettings();
}

void FShooterRecentlyMet::ApplySettings()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();

		PersistentUser->SaveIfDirty();
	}

	UserSettings->ApplySettings(false);

	OnApplyChanges.ExecuteIfBound();
}

void FShooterRecentlyMet::TellInputAboutKeybindings()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}
}

UShooterPersistentUser* FShooterRecentlyMet::GetPersistentUser() const
{
	UShooterLocalPlayer* const SLP = Cast<UShooterLocalPlayer>(PlayerOwner);
	return SLP ? SLP->GetPersistentUser() : nullptr;
}

void FShooterRecentlyMet::UpdateRecentlyMet(int32 NewOwnerIndex)
{
	LocalUserNum = NewOwnerIndex;
	
	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid())
	{
		LocalUsername = Identity->GetPlayerNickname(LocalUserNum);
	}

	MenuHelper::ClearSubMenu(RecentlyMetItem);
	MaxRecentlyMetIndex = 0;

	AShooterGameState* const MyGameState = PlayerOwner->GetWorld()->GetGameState<AShooterGameState>();
	if (MyGameState != nullptr)
	{
		PlayerArray = MyGameState->PlayerArray;

		for (int32 i = 0; i < PlayerArray.Num(); ++i)
		{
			const APlayerState* PlayerState = PlayerArray[i];
			FString Username = PlayerState->GetHumanReadableName();
			if (Username != LocalUsername && PlayerState->bIsABot == false)
			{
				TSharedPtr<FShooterMenuItem> UserItem = MenuHelper::AddMenuItem(RecentlyMetItem, FText::FromString(Username));
				UserItem->OnControllerDownInputPressed.BindRaw(this, &FShooterRecentlyMet::IncrementRecentlyMetCounter);
				UserItem->OnControllerUpInputPressed.BindRaw(this, &FShooterRecentlyMet::DecrementRecentlyMetCounter);
				UserItem->OnControllerFacebuttonDownPressed.BindRaw(this, &FShooterRecentlyMet::ViewSelectedUsersProfile);
			}
			else
			{
				PlayerArray.RemoveAt(i);
				--i; //we just deleted an item, so we need to go make sure i doesn't increment again, otherwise it would skip the player that was supposed to be looked at next
			}
		}

		MaxRecentlyMetIndex = PlayerArray.Num() - 1;
	}

	MenuHelper::AddMenuItemSP(RecentlyMetItem, LOCTEXT("Close", "CLOSE"), this, &FShooterRecentlyMet::OnApplySettings);
}

void FShooterRecentlyMet::IncrementRecentlyMetCounter()
{
	if (CurrRecentlyMetIndex + 1 <= MaxRecentlyMetIndex)
	{
		++CurrRecentlyMetIndex;
	}
}
void FShooterRecentlyMet::DecrementRecentlyMetCounter()
{
	if (CurrRecentlyMetIndex - 1 >= MinRecentlyMetIndex)
	{
		--CurrRecentlyMetIndex;
	}
}
void FShooterRecentlyMet::ViewSelectedUsersProfile()
{
	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid() && PlayerArray.IsValidIndex(CurrRecentlyMetIndex))
	{
		const APlayerState* PlayerState = PlayerArray[CurrRecentlyMetIndex];
		TSharedPtr<const FUniqueNetId> Requestor = Identity->GetUniquePlayerId(LocalUserNum);
		TSharedPtr<const FUniqueNetId> Requestee = PlayerState->UniqueId.GetUniqueNetId();
		auto ExternalUI = Online::GetExternalUIInterface();
		if (ExternalUI.IsValid() && Requestor.IsValid() && Requestee.IsValid())
		{
			ExternalUI->ShowProfileUI(*Requestor, *Requestee, FOnProfileUIClosedDelegate());
		}
	}
}


#undef LOCTEXT_NAMESPACE
