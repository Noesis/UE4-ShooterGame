// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "OnlineIdentityInterface.h"
#include "ShooterGameMode.generated.h"

class AShooterAIController;
class AShooterPlayerState;
class AShooterPickup;
class FUniqueNetId;

UCLASS(config=Game)
class AShooterGameMode : public AGameMode
{
	GENERATED_UCLASS_BODY()	

	/** The bot pawn class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=GameMode)
	TSubclassOf<APawn> BotPawnClass;

	UFUNCTION(exec)
	void SetAllowBots(bool bInAllowBots, int32 InMaxBots = 8);

	virtual void PreInitializeComponents() override;

	/** Initialize the game. This is called before actors' PreInitializeComponents. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** Accept or reject a player attempting to join the server.  Fails login if you set the ErrorMessage to a non-empty string. */
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;

	/** starts match warmup */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** select best spawn point for player */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/** always pick new random spawn */
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	/** returns default pawn class for given controller */
	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;

	/** prevents friendly fire */
	virtual float ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const;

	/** notify about kills */
	virtual void Killed(AController* Killer, AController* KilledPlayer, APawn* KilledPawn, const UDamageType* DamageType);

	/** can players damage each other? */
	virtual bool CanDealDamage(AShooterPlayerState* DamageInstigator, AShooterPlayerState* DamagedPlayer) const;

	/** always create cheat manager */
	virtual bool AllowCheats(APlayerController* P) override;

	/** update remaining time */
	virtual void DefaultTimer();

	/** called before startmatch */
	virtual void HandleMatchIsWaitingToStart() override;

	/** starts new match */
	virtual void HandleMatchHasStarted() override;

	/** hides the onscreen hud and restarts the map */
	virtual void RestartGame() override;

	/** Creates AIControllers for all bots */
	void CreateBotControllers();

	/** Create a bot */
	AShooterAIController* CreateBot(int32 BotNum);	

protected:

	/** delay between first player login and starting match */
	UPROPERTY(config)
	int32 WarmupTime;

	/** match duration */
	UPROPERTY(config)
	int32 RoundTime;

	UPROPERTY(config)
	int32 TimeBetweenMatches;

	/** score for kill */
	UPROPERTY(config)
	int32 KillScore;

	/** score for death */
	UPROPERTY(config)
	int32 DeathScore;

	/** scale for self instigated damage */
	UPROPERTY(config)
	float DamageSelfScale;

	UPROPERTY(config)
	int32 MaxBots;

	UPROPERTY()
	TArray<AShooterAIController*> BotControllers;
	
	/** Handle for efficient management of DefaultTimer timer */
	FTimerHandle TimerHandle_DefaultTimer;

	bool bNeedsBotCreation;

	bool bAllowBots;		

	/** spawning all bots for this game */
	void StartBots();

	/** initialization for bot after creation */
	virtual void InitBot(AShooterAIController* AIC, int32 BotNum);

	/** check who won */
	virtual void DetermineMatchWinner();

	/** check if PlayerState is a winner */
	virtual bool IsWinner(AShooterPlayerState* PlayerState) const;

	/** check if player can use spawnpoint */
	virtual bool IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Player) const;

	/** check if player should use spawnpoint */
	virtual bool IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Player) const;

	/** Returns game session class to use */
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;	

public:	

	/** finish current match and lock players */
	UFUNCTION(exec)
	void FinishMatch();

	/*Finishes the match and bumps everyone to main menu.*/
	/*Only GameInstance should call this function */
	void RequestFinishAndExitToMainMenu();

	/** get the name of the bots count option used in server travel URL */
	static FString GetBotsCountOptionName();

	UPROPERTY()
	TArray<AShooterPickup*> LevelPickups;

};
