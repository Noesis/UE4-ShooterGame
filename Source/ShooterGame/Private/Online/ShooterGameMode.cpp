// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGameInstance.h"
#include "UI/ShooterHUD.h"
#include "Player/ShooterSpectatorPawn.h"
#include "Player/ShooterDemoSpectator.h"
#include "Online/ShooterGameMode.h"
#include "Online/ShooterPlayerState.h"
#include "Online/ShooterGameSession.h"
#include "Bots/ShooterAIController.h"
#include "ShooterTeamStart.h"


AShooterGameMode::AShooterGameMode(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnOb(TEXT("/Game/Blueprints/Pawns/PlayerPawn"));
	DefaultPawnClass = PlayerPawnOb.Class;
	
	static ConstructorHelpers::FClassFinder<APawn> BotPawnOb(TEXT("/Game/Blueprints/Pawns/BotPawn"));
	BotPawnClass = BotPawnOb.Class;

	HUDClass = AShooterHUD::StaticClass();
	PlayerControllerClass = AShooterPlayerController::StaticClass();
	PlayerStateClass = AShooterPlayerState::StaticClass();
	SpectatorClass = AShooterSpectatorPawn::StaticClass();
	GameStateClass = AShooterGameState::StaticClass();
	ReplaySpectatorPlayerControllerClass = AShooterDemoSpectator::StaticClass();

	MinRespawnDelay = 5.0f;

	bAllowBots = true;	
	bNeedsBotCreation = true;
	bUseSeamlessTravel = true;	
}

FString AShooterGameMode::GetBotsCountOptionName()
{
	return FString(TEXT("Bots"));
}

void AShooterGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	const int32 BotsCountOptionValue = UGameplayStatics::GetIntOption(Options, GetBotsCountOptionName(), 0);
	SetAllowBots(BotsCountOptionValue > 0 ? true : false, BotsCountOptionValue);	
	Super::InitGame(MapName, Options, ErrorMessage);

	const UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance && Cast<UShooterGameInstance>(GameInstance)->GetOnlineMode() != EOnlineMode::Offline)
	{
		bPauseable = false;
	}
}

void AShooterGameMode::SetAllowBots(bool bInAllowBots, int32 InMaxBots)
{
	bAllowBots = bInAllowBots;
	MaxBots = InMaxBots;
}

/** Returns game session class to use */
TSubclassOf<AGameSession> AShooterGameMode::GetGameSessionClass() const
{
	return AShooterGameSession::StaticClass();
}

void AShooterGameMode::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	GetWorldTimerManager().SetTimer(TimerHandle_DefaultTimer, this, &AShooterGameMode::DefaultTimer, GetWorldSettings()->GetEffectiveTimeDilation(), true);
}

void AShooterGameMode::DefaultTimer()
{
	// don't update timers for Play In Editor mode, it's not real match
	if (GetWorld()->IsPlayInEditor())
	{
		// start match if necessary.
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
		return;
	}

	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
	if (MyGameState && MyGameState->RemainingTime > 0 && !MyGameState->bTimerPaused)
	{
		MyGameState->RemainingTime--;
		
		if (MyGameState->RemainingTime <= 0)
		{
			if (GetMatchState() == MatchState::WaitingPostMatch)
			{
				RestartGame();
			}
			else if (GetMatchState() == MatchState::InProgress)
			{
				FinishMatch();

				// Send end round events
				for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
				{
					AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(*It);
					
					if (PlayerController && MyGameState)
					{
						AShooterPlayerState* PlayerState = Cast<AShooterPlayerState>((*It)->PlayerState);
						const bool bIsWinner = IsWinner(PlayerState);
					
						PlayerController->ClientSendRoundEndEvent(bIsWinner, MyGameState->ElapsedTime);
					}
				}
			}
			else if (GetMatchState() == MatchState::WaitingToStart)
			{
				StartMatch();
			}
		}
	}
}

void AShooterGameMode::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();

	if (bNeedsBotCreation)
	{
		CreateBotControllers();
		bNeedsBotCreation = false;
	}

	if (bDelayedStart)
	{
		// start warmup if needed
		AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
		if (MyGameState && MyGameState->RemainingTime == 0)
		{
			const bool bWantsMatchWarmup = !GetWorld()->IsPlayInEditor();
			if (bWantsMatchWarmup && WarmupTime > 0)
			{
				MyGameState->RemainingTime = WarmupTime;
			}
			else
			{
				MyGameState->RemainingTime = 0.0f;
			}
		}
	}
}

void AShooterGameMode::HandleMatchHasStarted()
{
	bNeedsBotCreation = true;
	Super::HandleMatchHasStarted();

	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
	MyGameState->RemainingTime = RoundTime;	
	StartBots();	

	// notify players
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AShooterPlayerController* PC = Cast<AShooterPlayerController>(*It);
		if (PC)
		{
			PC->ClientGameStarted();
		}
	}
}

void AShooterGameMode::FinishMatch()
{
	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
	if (IsMatchInProgress())
	{
		EndMatch();
		DetermineMatchWinner();		

		// notify players
		for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
		{
			AShooterPlayerState* PlayerState = Cast<AShooterPlayerState>((*It)->PlayerState);
			const bool bIsWinner = IsWinner(PlayerState);

			(*It)->GameHasEnded(NULL, bIsWinner);
		}

		// lock all pawns
		// pawns are not marked as keep for seamless travel, so we will create new pawns on the next match rather than
		// turning these back on.
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			(*It)->TurnOff();
		}

		// set up to restart the match
		MyGameState->RemainingTime = TimeBetweenMatches;
	}
}

void AShooterGameMode::RequestFinishAndExitToMainMenu()
{
	FinishMatch();

	UShooterGameInstance* const GameInstance = Cast<UShooterGameInstance>(GetGameInstance());
	if (GameInstance)
	{
		GameInstance->RemoveSplitScreenPlayers();
	}

	AShooterPlayerController* LocalPrimaryController = nullptr;
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		AShooterPlayerController* Controller = Cast<AShooterPlayerController>(*Iterator);

		if (Controller == NULL)
		{
			continue;
		}

		if (!Controller->IsLocalController())
		{
			const FString RemoteReturnReason = NSLOCTEXT("NetworkErrors", "HostHasLeft", "Host has left the game.").ToString();
			Controller->ClientReturnToMainMenu(RemoteReturnReason);
		}
		else
		{
			LocalPrimaryController = Controller;
		}
	}

	// GameInstance should be calling this from an EndState.  So call the PC function that performs cleanup, not the one that sets GI state.
	if (LocalPrimaryController != NULL)
	{
		LocalPrimaryController->HandleReturnToMainMenu();
	}
}

void AShooterGameMode::DetermineMatchWinner()
{
	// nothing to do here
}

bool AShooterGameMode::IsWinner(class AShooterPlayerState* PlayerState) const
{
	return false;
}

void AShooterGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	AShooterGameState* const MyGameState = Cast<AShooterGameState>(GameState);
	const bool bMatchIsOver = MyGameState && MyGameState->HasMatchEnded();
	if( bMatchIsOver )
	{
		ErrorMessage = TEXT("Match is over!");
	}
	else
	{
		// GameSession can be NULL if the match is over
		Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
	}
}


void AShooterGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// update spectator location for client
	AShooterPlayerController* NewPC = Cast<AShooterPlayerController>(NewPlayer);
	if (NewPC && NewPC->GetPawn() == NULL)
	{
		NewPC->ClientSetSpectatorCamera(NewPC->GetSpawnLocation(), NewPC->GetControlRotation());
	}

	// notify new player if match is already in progress
	if (NewPC && IsMatchInProgress())
	{
		NewPC->ClientGameStarted();
		NewPC->ClientStartOnlineGame();
	}
}

void AShooterGameMode::Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType)
{
	AShooterPlayerState* KillerPlayerState = Killer ? Cast<AShooterPlayerState>(Killer->PlayerState) : NULL;
	AShooterPlayerState* VictimPlayerState = KilledPlayer ? Cast<AShooterPlayerState>(KilledPlayer->PlayerState) : NULL;

	if (KillerPlayerState && KillerPlayerState != VictimPlayerState)
	{
		KillerPlayerState->ScoreKill(VictimPlayerState, KillScore);
		KillerPlayerState->InformAboutKill(KillerPlayerState, DamageType, VictimPlayerState);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->ScoreDeath(KillerPlayerState, DeathScore);
		VictimPlayerState->BroadcastDeath(KillerPlayerState, DamageType, VictimPlayerState);
	}
}

float AShooterGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	AShooterCharacter* DamagedPawn = Cast<AShooterCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		AShooterPlayerState* DamagedPlayerState = Cast<AShooterPlayerState>(DamagedPawn->PlayerState);
		AShooterPlayerState* InstigatorPlayerState = Cast<AShooterPlayerState>(EventInstigator->PlayerState);

		// disable friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.0f;
		}

		// scale self instigated damage
		if (InstigatorPlayerState == DamagedPlayerState)
		{
			ActualDamage *= DamageSelfScale;
		}
	}

	return ActualDamage;
}

bool AShooterGameMode::CanDealDamage(class AShooterPlayerState* DamageInstigator, class AShooterPlayerState* DamagedPlayer) const
{
	return true;
}

bool AShooterGameMode::AllowCheats(APlayerController* P)
{
	return true;
}

bool AShooterGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

UClass* AShooterGameMode::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	if (InController->IsA<AShooterAIController>())
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

AActor* AShooterGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	APlayerStart* BestStart = NULL;
	for (TActorIterator<APlayerStart> It(GetWorld()); It; ++It)
	{
		APlayerStart* TestSpawn = *It;
		if (TestSpawn->IsA<APlayerStartPIE>())
		{
			// Always prefer the first "Play from Here" PlayerStart, if we find one while in PIE mode
			BestStart = TestSpawn;
			break;
		}
		else
		{
			if (IsSpawnpointAllowed(TestSpawn, Player))
			{
				if (IsSpawnpointPreferred(TestSpawn, Player))
				{
					PreferredSpawns.Add(TestSpawn);
				}
				else
				{
					FallbackSpawns.Add(TestSpawn);
				}
			}
		}
	}

	
	if (BestStart == NULL)
	{
		if (PreferredSpawns.Num() > 0)
		{
			BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
		}
		else if (FallbackSpawns.Num() > 0)
		{
			BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
		}
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}

bool AShooterGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const
{
	AShooterTeamStart* ShooterSpawnPoint = Cast<AShooterTeamStart>(SpawnPoint);
	if (ShooterSpawnPoint)
	{
		AShooterAIController* AIController = Cast<AShooterAIController>(Player);
		if (ShooterSpawnPoint->bNotForBots && AIController)
		{
			return false;
		}

		if (ShooterSpawnPoint->bNotForPlayers && AIController == NULL)
		{
			return false;
		}
		return true;
	}

	return false;
}

bool AShooterGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const
{
	ACharacter* MyPawn = Cast<ACharacter>((*DefaultPawnClass)->GetDefaultObject<ACharacter>());	
	AShooterAIController* AIController = Cast<AShooterAIController>(Player);
	if( AIController != nullptr )
	{
		MyPawn = Cast<ACharacter>(BotPawnClass->GetDefaultObject<ACharacter>());
	}
	
	if (MyPawn)
	{
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn && OtherPawn != MyPawn)
			{
				const float CombinedHeight = (MyPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedRadius = MyPawn->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// check if player start overlaps this pawn
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedRadius)
				{
					return false;
				}
			}
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

void AShooterGameMode::CreateBotControllers()
{
	UWorld* World = GetWorld();
	int32 ExistingBots = 0;
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{		
		AShooterAIController* AIC = Cast<AShooterAIController>(*It);
		if (AIC)
		{
			++ExistingBots;
		}
	}

	// Create any necessary AIControllers.  Hold off on Pawn creation until pawns are actually necessary or need recreating.	
	int32 BotNum = ExistingBots;
	for (int32 i = 0; i < MaxBots - ExistingBots; ++i)
	{
		CreateBot(BotNum + i);
	}
}

AShooterAIController* AShooterGameMode::CreateBot(int32 BotNum)
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.Instigator = nullptr;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.OverrideLevel = nullptr;

	UWorld* World = GetWorld();
	AShooterAIController* AIC = World->SpawnActor<AShooterAIController>(SpawnInfo);
	InitBot(AIC, BotNum);

	return AIC;
}

void AShooterGameMode::StartBots()
{
	// checking number of existing human player.
	UWorld* World = GetWorld();
	for (FConstControllerIterator It = World->GetControllerIterator(); It; ++It)
	{		
		AShooterAIController* AIC = Cast<AShooterAIController>(*It);
		if (AIC)
		{
			RestartPlayer(AIC);
		}
	}	
}

void AShooterGameMode::InitBot(AShooterAIController* AIController, int32 BotNum)
{	
	if (AIController)
	{
		if (AIController->PlayerState)
		{
			FString BotName = FString::Printf(TEXT("Bot %d"), BotNum);
			AIController->PlayerState->PlayerName = BotName;
		}		
	}
}

void AShooterGameMode::RestartGame()
{
	// Hide the scoreboard too !
	for (FConstControllerIterator It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(*It);
		if (PlayerController != nullptr)
		{
			AShooterHUD* ShooterHUD = Cast<AShooterHUD>(PlayerController->GetHUD());
			if (ShooterHUD != nullptr)
			{
				// Passing true to bFocus here ensures that focus is returned to the game viewport.
				ShooterHUD->ShowScoreboard(false, true);
			}
		}
	}

	Super::RestartGame();
}

