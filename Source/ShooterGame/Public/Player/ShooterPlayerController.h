// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Online.h"
#include "ShooterPlayerController.generated.h"

class AShooterHUD;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FPlayerKilledDelegate, class AShooterPlayerState*, KillerPlayerState, class AShooterPlayerState*, KilledPlayerState, const UDamageType*, KillerDamageType);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHitTakenDelegate, float, DamageTaken, FDamageEvent, DamageEvent, APawn*, PawnInstigator);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FEnemyHitDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOutOfAmmoDelegate);

UCLASS(BlueprintType, config=Game)
class AShooterPlayerController : public APlayerController
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable)
	class UShooterLocalPlayer* GetLocalPlayer() const;

	/** sets spectator location and rotation */
	UFUNCTION(reliable, client)
	void ClientSetSpectatorCamera(FVector CameraLocation, FRotator CameraRotation);

	/** notify player about started match */
	UFUNCTION(reliable, client)
	void ClientGameStarted();

	/** Starts the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientStartOnlineGame();

	/** Ends the online game using the session name in the PlayerState */
	UFUNCTION(reliable, client)
	void ClientEndOnlineGame();	

	/** notify player about finished match */
	virtual void ClientGameEnded_Implementation(class AActor* EndGameFocus, bool bIsWinner);

	/** Notifies clients to send the end-of-round event */
	UFUNCTION(reliable, client)
	void ClientSendRoundEndEvent(bool bIsWinner, int32 ExpendedTimeInSeconds);

	/** used for input simulation from blueprint (for automatic perf tests) */
	UFUNCTION(BlueprintCallable, Category="Input")
	void SimulateInputKey(FKey Key, bool bPressed = true);

	/** sends cheat message */
	UFUNCTION(reliable, server, WithValidation)
	void ServerCheat(const FString& Msg);

	/* Overriden Message implementation. */
	virtual void ClientTeamMessage_Implementation( APlayerState* SenderPlayerState, const FString& S, FName Type, float MsgLifeTime ) override;

	/* Tell the HUD to toggle the chat window. */
	void ToggleChatWindow();

	/** Local function say a string */
	UFUNCTION(exec)
	virtual void Say(const FString& Msg);

	/** RPC for clients to talk to server */
	UFUNCTION(unreliable, server, WithValidation)
	void ServerSay(const FString& Msg);	

	/** Local function run an emote */
// 	UFUNCTION(exec)
// 	virtual void Emote(const FString& Msg);

	/** notify local client about deaths */
	void OnDeathMessage(class AShooterPlayerState* KillerPlayerState, class AShooterPlayerState* KilledPlayerState, const UDamageType* KillerDamageType);

	/** */
	void OnHitTaken(float DamageTaken, struct FDamageEvent const& DamageEvent, class APawn* PawnInstigator);

	void OnEnemyHit();

	void NotifyOutOfAmmo();

	/** toggle InGameMenu handler */
	void OnToggleInGameMenu();

	/** Show the in-game menu if it's not already showing */
	void ShowInGameMenu();

	/** Hides scoreboard if currently diplayed */
	void OnConditionalCloseScoreboard();

	/** Toggles scoreboard */
	void OnToggleScoreboard();

	/** shows scoreboard */
	void OnShowScoreboard();

	/** hides scoreboard */
	void OnHideScoreboard();

	/** set infinite ammo cheat */
	UFUNCTION(BlueprintCallable)
	void SetInfiniteAmmo(bool bEnable);

	/** set infinite clip cheat */
	UFUNCTION(BlueprintCallable)
	void SetInfiniteClip(bool bEnable);

	/** set health regen cheat */
	UFUNCTION(BlueprintCallable)
	void SetHealthRegen(bool bEnable);

	/** set god mode cheat */
	UFUNCTION(exec)
	void SetGodMode(bool bEnable);

	/** sets the produce force feedback flag. */
	void SetIsVibrationEnabled(bool bEnable);

	/** get infinite ammo cheat */
	UFUNCTION(BlueprintCallable)
	bool HasInfiniteAmmo() const;

	/** get infinite clip cheat */
	UFUNCTION(BlueprintCallable)
	bool HasInfiniteClip() const;

	/** get health regen cheat */
	UFUNCTION(BlueprintCallable)
	bool HasHealthRegen() const;

	/** get gode mode cheat */
	bool HasGodMode() const;

	/** should produce force feedback? */
	bool IsVibrationEnabled() const;

	/** check if gameplay related actions (movement, weapon usage, etc) are allowed right now */
	bool IsGameInputAllowed() const;

	/** is game menu currently active? */
	bool IsGameMenuVisible() const;	

	/** Ends and/or destroys game session */
	void CleanupSessionOnReturnToMenu();

	/**
	 * Called when the read achievements request from the server is complete
	 *
	 * @param PlayerId The player id who is responsible for this delegate being fired
	 * @param bWasSuccessful true if the server responded successfully to the request
	 */
	void OnQueryAchievementsComplete(const FUniqueNetId& PlayerId, const bool bWasSuccessful );
	
	// Begin APlayerController interface

	/** handle weapon visibility */
	virtual void SetCinematicMode(bool bInCinematicMode, bool bHidePlayer, bool bAffectsHUD, bool bAffectsMovement, bool bAffectsTurning) override;

	/** Returns true if movement input is ignored. Overridden to always allow spectators to move. */
	virtual bool IsMoveInputIgnored() const override;

	/** Returns true if look input is ignored. Overridden to always allow spectators to look around. */
	virtual bool IsLookInputIgnored() const override;

	/** initialize the input system from the player settings */
	virtual void InitInputSystem() override;

	virtual bool SetPause(bool bPause, FCanUnpause CanUnpauseDelegate = FCanUnpause()) override;

	virtual FVector GetFocalLocation() const override;

	// End APlayerController interface

	// begin AShooterPlayerController-specific

	/** 
	 * Reads achievements to precache them before first use
	 */
	void QueryAchievements();

	/** 
	 * Writes a single achievement (unless another write is in progress).
	 *
	 * @param Id achievement id (string)
	 * @param Percent number 1 to 100
	 */
	void UpdateAchievementProgress( const FString& Id, float Percent );

	/** Returns a pointer to the shooter game hud. May return NULL. */
	AShooterHUD* GetShooterHUD() const;

	/** Returns the persistent user record associated with this player, or null if there is't one. */
	class UShooterPersistentUser* GetPersistentUser() const;

	/** Informs that player fragged someone */
	void OnKill();

	/** Cleans up any resources necessary to return to main menu.  Does not modify GameInstance state. */
	virtual void HandleReturnToMainMenu();

	/** Associate a new UPlayer with this PlayerController. */
	virtual void SetPlayer(UPlayer* Player);

	// end AShooterPlayerController-specific

	virtual void PreClientTravel(const FString& PendingURL, ETravelType TravelType, bool bIsSeamlessTravel) override;

protected:

	/** infinite ammo cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bInfiniteAmmo : 1;

	/** infinite clip cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bInfiniteClip : 1;

	/** health regen cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bHealthRegen : 1;

	/** god mode cheat */
	UPROPERTY(Transient, Replicated)
	uint8 bGodMode : 1;

	/** should produce force feedback? */
	uint8 bIsVibrationEnabled : 1;

	/** if set, gameplay related actions (movement, weapn usage, etc) are allowed */
	uint8 bAllowGameActions : 1;

	/** true for the first frame after the game has ended */
	uint8 bGameEndedFrame : 1;

	/** stores pawn location at last player death, used where player scores a kill after they died **/
	FVector LastDeathLocation;

	/** shooter in-game menu */
	TSharedPtr<class FShooterIngameMenu> ShooterIngameMenu;

	/** Achievements write object */
	FOnlineAchievementsWritePtr WriteObject;

	UPROPERTY(BlueprintAssignable)
	FPlayerKilledDelegate PlayerKilledDelegate;

	UPROPERTY(BlueprintAssignable)
	FHitTakenDelegate HitTakenDelegate;

	UPROPERTY(BlueprintAssignable)
	FEnemyHitDelegate EnemyHitDelegate;

	UPROPERTY(BlueprintAssignable)
	FOutOfAmmoDelegate OutOfAmmoDelegate;

	/** try to find spot for death cam */
	bool FindDeathCameraSpot(FVector& CameraLocation, FRotator& CameraRotation);

	virtual void BeginDestroy() override;

	//Begin AActor interface

	/** after all game elements are created */
	virtual void PostInitializeComponents() override;

public:
	virtual void TickActor(float DeltaTime, enum ELevelTick TickType, FActorTickFunction& ThisTickFunction) override;
	//End AActor interface

	//Begin AController interface
	
	/** transition to dead state, retries spawning later */
	virtual void FailedToSpawnPawn() override;
	
	/** update camera when pawn dies */
	virtual void PawnPendingDestroy(APawn* P) override;

	//End AController interface

	// Begin APlayerController interface

	/** respawn after dying */
	virtual void UnFreeze() override;

	/** sets up input */
	virtual void SetupInputComponent() override;

	/**
	 * Called from game info upon end of the game, used to transition to proper state. 
	 *
	 * @param EndGameFocus Actor to set as the view target on end game
	 * @param bIsWinner true if this controller is on winning team
	 */
	virtual void GameHasEnded(class AActor* EndGameFocus = NULL, bool bIsWinner = false) override;	

	/** Return the client to the main menu gracefully.  ONLY sets GI state. */
	void ClientReturnToMainMenu_Implementation(const FString& ReturnReason) override;

	/** Causes the player to commit suicide */
	UFUNCTION(exec)
	virtual void Suicide();

	/** Notifies the server that the client has suicided */
	UFUNCTION(reliable, server, WithValidation)
	void ServerSuicide();

	/** Updates achievements based on the PersistentUser stats at the end of a round */
	void UpdateAchievementsOnGameEnd();

	/** Updates leaderboard stats at the end of a round */
	void UpdateLeaderboardsOnGameEnd();

	/** Updates the save file at the end of a round */
	void UpdateSaveFileOnGameEnd(bool bIsWinner);

	// End APlayerController interface

	FName	ServerSayString;

	// Timer used for updating friends in the player tick.
	float ShooterFriendUpdateTimer;

	// For tracking whether or not to send the end event
	bool bHasSentStartEvents;

private:

	/** Handle for efficient management of ClientStartOnlineGame timer */
	FTimerHandle TimerHandle_ClientStartOnlineGame;
};

