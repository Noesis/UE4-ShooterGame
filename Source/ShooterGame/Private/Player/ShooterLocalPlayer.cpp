// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterLocalPlayer.h"
#include "OnlineSubsystemUtilsClasses.h"
#include "ShooterGameInstance.h"

UShooterLocalPlayer::UShooterLocalPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UShooterPersistentUser* UShooterLocalPlayer::GetPersistentUser() const
{
	// if persistent data isn't loaded yet, load it
	if (PersistentUser == nullptr)
	{
		UShooterLocalPlayer* const MutableThis = const_cast<UShooterLocalPlayer*>(this);
		// casting away constness to enable caching implementation behavior
		MutableThis->LoadPersistentUser();
	}
	return PersistentUser;
}

void UShooterLocalPlayer::LoadPersistentUser()
{
	FString SaveGameName = GetNickname();

#if PLATFORM_SWITCH
	// on Switch, the displayable nickname can change, so we can't use it as a save ID (explicitly stated in docs, so changing for pre-cert)
	FPlatformMisc::GetUniqueStringNameForControllerId(GetControllerId(), SaveGameName);
#endif

	// if we changed controllerid / user, then we need to load the appropriate persistent user.
	if (PersistentUser != nullptr && ( GetControllerId() != PersistentUser->GetUserIndex() || SaveGameName != PersistentUser->GetName() ) )
	{
		PersistentUser->SaveIfDirty();
		PersistentUser = nullptr;
	}

	if (PersistentUser == NULL)
	{
		// Use the platform id here to be resilient in the face of controller swapping and similar situations.
		FPlatformUserId PlatformId = GetControllerId();

		auto Identity = Online::GetIdentityInterface();
		if (Identity.IsValid() && GetPreferredUniqueNetId().IsValid())
		{
			PlatformId = Identity->GetPlatformUserIdFromUniqueNetId(*GetPreferredUniqueNetId());
		}

		PersistentUser = UShooterPersistentUser::LoadPersistentUser(SaveGameName, PlatformId );
	}
}

void UShooterLocalPlayer::SetControllerId(int32 NewControllerId)
{
	FString SaveGameName = GetNickname();

#if PLATFORM_SWITCH
	// on Switch, the displayable nickname can change, so we can't use it as a save ID (explicitly stated in docs, so changing for pre-cert)
	FPlatformMisc::GetUniqueStringNameForControllerId(GetControllerId(), SaveGameName);
#endif

	ULocalPlayer::SetControllerId(NewControllerId);

	// if we changed controllerid / user, then we need to load the appropriate persistent user.
	if (PersistentUser != nullptr && ( GetControllerId() != PersistentUser->GetUserIndex() || SaveGameName != PersistentUser->GetName() ) )
	{
		PersistentUser->SaveIfDirty();
		PersistentUser = nullptr;
	}

	if (!PersistentUser)
	{
		LoadPersistentUser();
	}
}

FString UShooterLocalPlayer::GetNickname() const
{
	FString UserNickName = Super::GetNickname();

	if ( UserNickName.Len() > MAX_PLAYER_NAME_LENGTH )
	{
		UserNickName = UserNickName.Left( MAX_PLAYER_NAME_LENGTH ) + "...";
	}

	bool bReplace = (UserNickName.Len() == 0);

	// Check for duplicate nicknames...and prevent reentry
	static bool bReentry = false;
	if(!bReentry)
	{
		bReentry = true;
		UShooterGameInstance* GameInstance = GetWorld() != NULL ? Cast<UShooterGameInstance>(GetWorld()->GetGameInstance()) : NULL;
		if(GameInstance)
		{
			// Check all the names that occur before ours that are the same
			const TArray<ULocalPlayer*>& LocalPlayers = GameInstance->GetLocalPlayers();
			for (int i = 0; i < LocalPlayers.Num(); ++i)
			{
				const ULocalPlayer* LocalPlayer = LocalPlayers[i];
				if( this == LocalPlayer)
				{
					break;
				}

				if( UserNickName == LocalPlayer->GetNickname())
				{
					bReplace = true;
					break;
				}
			}
		}
		bReentry = false;
	}

	if ( bReplace )
	{
		UserNickName = FString::Printf( TEXT( "Player%i" ), GetControllerId() + 1 );
	}	

	return UserNickName;
}
