// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Player/ShooterCheatManager.h"
#include "Online/ShooterPlayerState.h"
#include "Bots/ShooterAIController.h"

UShooterCheatManager::UShooterCheatManager(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UShooterCheatManager::ToggleInfiniteAmmo()
{
	AShooterPlayerController* MyPC = GetOuterAShooterPlayerController();

	MyPC->SetInfiniteAmmo(!MyPC->HasInfiniteAmmo());
	MyPC->ClientMessage(FString::Printf(TEXT("Infinite ammo: %s"), MyPC->HasInfiniteAmmo() ? TEXT("ENABLED") : TEXT("off")));
}

void UShooterCheatManager::ToggleInfiniteClip()
{
	AShooterPlayerController* MyPC = GetOuterAShooterPlayerController();

	MyPC->SetInfiniteClip(!MyPC->HasInfiniteClip());
	MyPC->ClientMessage(FString::Printf(TEXT("Infinite clip: %s"), MyPC->HasInfiniteClip() ? TEXT("ENABLED") : TEXT("off")));
}

void UShooterCheatManager::ToggleMatchTimer()
{
	AShooterPlayerController* MyPC = GetOuterAShooterPlayerController();

	AShooterGameState* const MyGameState = MyPC->GetWorld()->GetGameState<AShooterGameState>();
	if (MyGameState && MyGameState->Role == ROLE_Authority)
	{
		MyGameState->bTimerPaused = !MyGameState->bTimerPaused;
		MyPC->ClientMessage(FString::Printf(TEXT("Match timer: %s"), MyGameState->bTimerPaused ? TEXT("PAUSED") : TEXT("running")));
	}
}

void UShooterCheatManager::ForceMatchStart()
{
	AShooterPlayerController* const MyPC = GetOuterAShooterPlayerController();

	AShooterGameMode* const MyGame = MyPC->GetWorld()->GetAuthGameMode<AShooterGameMode>();
	if (MyGame && MyGame->GetMatchState() == MatchState::WaitingToStart)
	{
		MyGame->StartMatch();
	}
}

void UShooterCheatManager::ChangeTeam(int32 NewTeamNumber)
{
	AShooterPlayerController* MyPC = GetOuterAShooterPlayerController();

	AShooterPlayerState* MyPlayerState = Cast<AShooterPlayerState>(MyPC->PlayerState);
	if (MyPlayerState && MyPlayerState->Role == ROLE_Authority)
	{
		MyPlayerState->SetTeamNum(NewTeamNumber);
		MyPC->ClientMessage(FString::Printf(TEXT("Team changed to: %d"), MyPlayerState->GetTeamNum()));
	}
}

void UShooterCheatManager::Cheat(const FString& Msg)
{
	GetOuterAShooterPlayerController()->ServerCheat(Msg.Left(128));
}

void UShooterCheatManager::SpawnBot()
{
	AShooterPlayerController* const MyPC = GetOuterAShooterPlayerController();
	APawn* const MyPawn = MyPC->GetPawn();
	AShooterGameMode* const MyGame = MyPC->GetWorld()->GetAuthGameMode<AShooterGameMode>();
	UWorld* World = MyPC->GetWorld();
	if (MyPawn && MyGame && World)
	{
		static int32 CheatBotNum = 50;
		AShooterAIController* ShooterAIController = MyGame->CreateBot(CheatBotNum++);
		MyGame->RestartPlayer(ShooterAIController);		
	}
}