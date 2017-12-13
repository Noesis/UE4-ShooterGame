// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterOnlineSessionClient.h"
#include "ShooterGameInstance.h"

UShooterOnlineSessionClient::UShooterOnlineSessionClient()
{
}

void UShooterOnlineSessionClient::OnSessionUserInviteAccepted(
	const bool							bWasSuccess,
	const int32							ControllerId,
	TSharedPtr< const FUniqueNetId > 	UserId,
	const FOnlineSessionSearchResult &	InviteResult
)
{
	UE_LOG(LogOnline, Verbose, TEXT("HandleSessionUserInviteAccepted: bSuccess: %d, ControllerId: %d, User: %s"), bWasSuccess, ControllerId, UserId.IsValid() ? *UserId->ToString() : TEXT("NULL"));

	if (!bWasSuccess)
	{
		return;
	}

	if (!InviteResult.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Invite accept returned no search result."));
		return;
	}

	if (!UserId.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Invite accept returned no user."));
		return;
	}

	UShooterGameInstance* ShooterGameInstance = Cast<UShooterGameInstance>(GetGameInstance());

	if (ShooterGameInstance)
	{
		FShooterPendingInvite PendingInvite;

		// Set the pending invite, and then go to the initial screen, which is where we will process it
		PendingInvite.ControllerId = ControllerId;
		PendingInvite.UserId = UserId;
		PendingInvite.InviteResult = InviteResult;
		PendingInvite.bPrivilegesCheckedAndAllowed = false;

		ShooterGameInstance->SetPendingInvite(PendingInvite);
		ShooterGameInstance->GotoState(ShooterGameInstanceState::PendingInvite);
	}
}

void UShooterOnlineSessionClient::OnPlayTogetherEventReceived(int32 UserIndex, TArray<TSharedPtr<const FUniqueNetId>> UserIdList)
{	
	if (UShooterGameInstance* const ShooterGameInstance = Cast<UShooterGameInstance>(GetGameInstance()))
	{
		ShooterGameInstance->OnPlayTogetherEventReceived(UserIndex, UserIdList);
	}
}