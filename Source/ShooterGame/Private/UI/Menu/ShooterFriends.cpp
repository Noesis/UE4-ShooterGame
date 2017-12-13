// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterFriends.h"
#include "ShooterTypes.h"
#include "ShooterStyle.h"
#include "ShooterOptionsWidgetStyle.h"
#include "Player/ShooterPersistentUser.h"
#include "ShooterGameUserSettings.h"
#include "ShooterLocalPlayer.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void FShooterFriends::Construct(ULocalPlayer* _PlayerOwner, int32 LocalUserNum_)
{
	FriendsStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterOptionsStyle>("DefaultShooterOptionsStyle");

	PlayerOwner = _PlayerOwner;
	LocalUserNum = LocalUserNum_;
	CurrFriendIndex = 0;
	MinFriendIndex = 0;
	MaxFriendIndex = 0; //initialized after the friends list is read in

	/** Friends menu root item */
	TSharedPtr<FShooterMenuItem> FriendsRoot = FShooterMenuItem::CreateRoot();

	//Populate the friends list
	FriendsItem = MenuHelper::AddMenuItem(FriendsRoot, LOCTEXT("Friends", "FRIENDS"));
	OnlineSub = IOnlineSubsystem::Get();
	OnlineFriendsPtr = OnlineSub->GetFriendsInterface();
	if ( OnlineFriendsPtr.IsValid() )
	{
		OnlineFriendsPtr->ReadFriendsList(LocalUserNum, EFriendsLists::ToString(EFriendsLists::OnlinePlayers)); //init read of the friends list with the current user
	}

	UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
}

void FShooterFriends::OnApplySettings()
{
	ApplySettings();
}

void FShooterFriends::ApplySettings()
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

void FShooterFriends::TellInputAboutKeybindings()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}
}

UShooterPersistentUser* FShooterFriends::GetPersistentUser() const
{
	UShooterLocalPlayer* const ShooterLocalPlayer = Cast<UShooterLocalPlayer>(PlayerOwner);
	return ShooterLocalPlayer ? ShooterLocalPlayer->GetPersistentUser() : nullptr;
	//// Main Menu
	//AShooterPlayerController_Menu* ShooterPCM = Cast<AShooterPlayerController_Menu>(PCOwner);
	//if(ShooterPCM)
	//{
	//	return ShooterPCM->GetPersistentUser();
	//}

	//// In-game Menu
	//AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(PCOwner);
	//if(ShooterPC)
	//{
	//	return ShooterPC->GetPersistentUser();
	//}

	//return nullptr;
}

void FShooterFriends::UpdateFriends(int32 NewOwnerIndex)
{
	if (!OnlineFriendsPtr.IsValid())
	{
		return;
	}

	LocalUserNum = NewOwnerIndex;
	OnlineFriendsPtr->ReadFriendsList(LocalUserNum, EFriendsLists::ToString(EFriendsLists::OnlinePlayers));
	MenuHelper::ClearSubMenu(FriendsItem);

	if (OnlineFriendsPtr->GetFriendsList(LocalUserNum, EFriendsLists::ToString(EFriendsLists::OnlinePlayers), Friends))
	{
		for (int32 Idx = 0; Idx < Friends.Num(); ++Idx)
		{
			const FString FriendUsername = Friends[Idx]->GetDisplayName();
			TSharedPtr<FShooterMenuItem> FriendItem = MenuHelper::AddMenuItem(FriendsItem, FText::FromString(FriendUsername));
			//FriendItem->OnControllerFacebuttonLeftPressed.BindRaw(this, &FShooterFriends::InviteSelectedFriendToGame);
			FriendItem->OnControllerDownInputPressed.BindRaw(this, &FShooterFriends::IncrementFriendsCounter);
			FriendItem->OnControllerUpInputPressed.BindRaw(this, &FShooterFriends::DecrementFriendsCounter);
			FriendItem->OnControllerFacebuttonDownPressed.BindRaw(this, &FShooterFriends::ViewSelectedFriendProfile);
		}
		MaxFriendIndex = Friends.Num() - 1;
	}
	MenuHelper::AddMenuItemSP(FriendsItem, LOCTEXT("Close", "CLOSE"), this, &FShooterFriends::OnApplySettings);
}

void FShooterFriends::IncrementFriendsCounter()
{
	if (CurrFriendIndex + 1 <= MaxFriendIndex)
	{
		++CurrFriendIndex;
	}
}
void FShooterFriends::DecrementFriendsCounter()
{
	if (CurrFriendIndex - 1 >= MinFriendIndex)
	{
		--CurrFriendIndex;
	}
}
void FShooterFriends::ViewSelectedFriendProfile()
{
	auto Identity = Online::GetIdentityInterface();
	if (Identity.IsValid() && Friends.IsValidIndex(CurrFriendIndex))
	{
		TSharedPtr<const FUniqueNetId> Requestor = Identity->GetUniquePlayerId(LocalUserNum);
		TSharedPtr<const FUniqueNetId> Requestee = Friends[CurrFriendIndex]->GetUserId();
		auto ExternalUI = Online::GetExternalUIInterface();
		if (ExternalUI.IsValid() && Requestor.IsValid() && Requestee.IsValid())
		{
			ExternalUI->ShowProfileUI(*Requestor, *Requestee, FOnProfileUIClosedDelegate());
		}
	}
}
void FShooterFriends::InviteSelectedFriendToGame()
{
	if (OnlineFriendsPtr.IsValid() && Friends.IsValidIndex(CurrFriendIndex))
	{
		OnlineFriendsPtr->SendInvite(LocalUserNum, *Friends[CurrFriendIndex]->GetUserId(), EFriendsLists::ToString(EFriendsLists::Default));
	}
}


#undef LOCTEXT_NAMESPACE
