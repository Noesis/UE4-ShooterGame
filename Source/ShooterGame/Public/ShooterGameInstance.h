// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterGame.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Engine/GameInstance.h"
#include "Engine/NetworkDelegates.h"
#include "ShooterGameInstance.generated.h"

class FVariantData;
class FShooterMainMenu;
class FShooterWelcomeMenu;
class FShooterMessageMenu;
class AShooterGameSession;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateStartedDelegate, FName, PrevState, FName, NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStateEndingDelegate, FName, CurState, FName, NextState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FServerSearchEndedDelegate, bool, bWasSuccessful, const TArray<FString>&, ServerNames);

namespace ShooterGameInstanceState
{
	extern const FName None;
	extern const FName PendingInvite;
	extern const FName WelcomeScreen;
	extern const FName MainMenu;
	extern const FName MessageMenu;
	extern const FName Playing;
}

/** This class holds the value of what message to display when we are in the "MessageMenu" state */
class FShooterPendingMessage
{
public:
	FText	DisplayString;				// This is the display message in the main message body
	FText	OKButtonString;				// This is the ok button text
	FText	CancelButtonString;			// If this is not empty, it will be the cancel button text
	FName	NextState;					// Final destination state once message is discarded

	TWeakObjectPtr< ULocalPlayer > PlayerOwner;		// Owner of dialog who will have focus (can be NULL)
};

class FShooterPendingInvite
{
public:
	FShooterPendingInvite() : ControllerId(-1), UserId(nullptr), bPrivilegesCheckedAndAllowed(false) {}

	int32							 ControllerId;
	TSharedPtr< const FUniqueNetId > UserId;
	FOnlineSessionSearchResult 		 InviteResult;
	bool							 bPrivilegesCheckedAndAllowed;
};

struct FShooterPlayTogetherInfo
{
	FShooterPlayTogetherInfo() : UserIndex(-1) {}
	FShooterPlayTogetherInfo(int32 InUserIndex, const TArray<TSharedPtr<const FUniqueNetId>>& InUserIdList)
		: UserIndex(InUserIndex)
	{
		UserIdList.Append(InUserIdList);
	}
	
	int32 UserIndex;
	TArray<TSharedPtr<const FUniqueNetId>> UserIdList;
};

class SShooterWaitDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterWaitDialog)
	{}
	SLATE_ARGUMENT(FText, MessageText)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:

	/** our curve sequence and the related handles */
	FCurveSequence WidgetAnimation;

	/** used for animating the text color. */
	FCurveHandle TextColorCurve;

	/** Gets the animated text color */
	FSlateColor GetTextColor() const;
};

UENUM()
enum class EOnlineMode : uint8
{
	Offline,
	LAN,
	Online
};


UCLASS(BlueprintType, config=Game)
class UShooterGameInstance : public UGameInstance
{
public:
	GENERATED_UCLASS_BODY()

public:

	bool Tick(float DeltaSeconds);

	AShooterGameSession* GetGameSession() const;

	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;
#if WITH_EDITOR
	virtual FGameInstancePIEResult StartPlayInEditorGameInstance(ULocalPlayer* LocalPlayer, const FGameInstancePIEParameters& Params) override;
#endif	// WITH_EDITOR

	virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;


	UFUNCTION(BlueprintCallable)
	bool HostGame(ULocalPlayer* LocalPlayer, const FString& GameType, const FString& InTravelURL);

	UFUNCTION(BlueprintCallable)
	bool JoinSession(ULocalPlayer* LocalPlayer, int32 SessionIndexInSearchResults);
	bool JoinSession(ULocalPlayer* LocalPlayer, const FOnlineSessionSearchResult& SearchResult);
	void SetPendingInvite(const FShooterPendingInvite& InPendingInvite);

	bool PlayDemo(ULocalPlayer* LocalPlayer, const FString& DemoName);
	
	/** Travel directly to the named session */
	void TravelToSession(const FName& SessionName);

	/** Begin a hosted quick match */
	void BeginHostingQuickMatch();

	/** Initiates the session searching */
	UFUNCTION(BlueprintCallable)
	bool FindSessions(ULocalPlayer* PlayerOwner, bool bIsDedicatedServer, bool bLANMatch);

	/** Sends the game to the specified state. */
	UFUNCTION(BlueprintCallable)
	void GotoState(FName NewState);

	/** Obtains the initial welcome state, which can be different based on platform */
	FName GetInitialState();

	/** Sends the game to the initial startup/frontend state  */
	void GotoInitialState();

	/**
	* Creates the message menu, clears other menus and sets the KingState to Message.
	*
	* @param	Message				Main message body
	* @param	OKButtonString		String to use for 'OK' button
	* @param	CancelButtonString	String to use for 'Cancel' button
	* @param	NewState			Final state to go to when message is discarded
	*/
	void ShowMessageThenGotoState( const FText& Message, const FText& OKButtonString, const FText& CancelButtonString, const FName& NewState, const bool OverrideExisting = true, TWeakObjectPtr< ULocalPlayer > PlayerOwner = nullptr );

	void RemoveExistingLocalPlayer(ULocalPlayer* ExistingPlayer);

	void RemoveSplitScreenPlayers();

	TSharedPtr< const FUniqueNetId > GetUniqueNetIdFromControllerId( const int ControllerId );

	/** Returns true if the game is in online mode */
	UFUNCTION(BlueprintCallable)
	EOnlineMode GetOnlineMode() const { return OnlineMode; }

	/** Sets the online mode of the game */
	UFUNCTION(BlueprintCallable)
	void SetOnlineMode(EOnlineMode InOnlineMode);

	/** Updates the status of using multiplayer features */
	void UpdateUsingMultiplayerFeatures(bool bIsUsingMultiplayerFeatures);

	/** Sets the controller to ignore for pairing changes. Useful when we are showing external UI for manual profile switching. */
	void SetIgnorePairingChangeForControllerId( const int32 ControllerId );

	/** Returns true if the passed in local player is signed in and online */
	bool IsLocalPlayerOnline(ULocalPlayer* LocalPlayer);

	/** Returns true if the passed in local player is signed in*/
	bool IsLocalPlayerSignedIn(ULocalPlayer* LocalPlayer);

	/** Returns true if owning player is online. Displays proper messaging if the user can't play */
	bool ValidatePlayerForOnlinePlay(ULocalPlayer* LocalPlayer);

	/** Returns true if owning player is signed in. Displays proper messaging if the user can't play */
	bool ValidatePlayerIsSignedIn(ULocalPlayer* LocalPlayer);

	/** Shuts down the session, and frees any net driver */
	void CleanupSessionOnReturnToMenu();

	/** Flag the local player when they quit the game */
	void LabelPlayerAsQuitter(ULocalPlayer* LocalPlayer) const;

	// Generic confirmation handling (just hide the dialog)
	FReply OnConfirmGeneric();
	bool HasLicense() const { return bIsLicensed; }

	/** Start task to get user privileges. */
	void StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate, EUserPrivileges::Type Privilege, TSharedPtr< const FUniqueNetId > UserId);

	/** Common cleanup code for any Privilege task delegate */
	void CleanupOnlinePrivilegeTask();

	/** Show approved dialogs for various privileges failures */
	void DisplayOnlinePrivilegeFailureDialogs(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	/** @return OnlineSession class to use for this player */
	TSubclassOf<class UOnlineSession> GetOnlineSessionClass() override;

	/** Called when we receive a Play Together system event on PS4 */
	void OnPlayTogetherEventReceived(const int32 UserIndex, const TArray<TSharedPtr<const FUniqueNetId>>& UserIdList);

	/** Resets Play Together PS4 system event info after it's been handled */
	void ResetPlayTogetherInfo() { PlayTogetherInfo = FShooterPlayTogetherInfo(); }

private:

	UPROPERTY(config)
	FString WelcomeScreenMap;

	UPROPERTY(config)
	FString MainMenuMap;

	UPROPERTY(BlueprintAssignable)
	FStateStartedDelegate OnStateStarted;

	UPROPERTY(BlueprintAssignable)
	FStateEndingDelegate OnStateEnding;

	UPROPERTY(BlueprintAssignable)
	FServerSearchEndedDelegate OnServerSearchEnded;


	FName CurrentState;
	FName PendingState;

	FShooterPendingMessage PendingMessage;

	FShooterPendingInvite PendingInvite;

	/** URL to travel to after pending network operations */
	FString TravelURL;

	/** Current online mode of the game (offline, LAN, or online) */
	EOnlineMode OnlineMode;

	/** If true, enable splitscreen when map starts loading */
	bool bPendingEnableSplitscreen;

	/** Whether the user has an active license to play the game */
	bool bIsLicensed;

	/** Main menu UI */
	TSharedPtr<FShooterMainMenu> MainMenuUI;

	/** Message menu (Shown in the even of errors - unable to connect etc) */
	TSharedPtr<FShooterMessageMenu> MessageMenuUI;

	/** Welcome menu UI (for consoles) */
	TSharedPtr<FShooterWelcomeMenu> WelcomeMenuUI;

	/** Dialog widget to show non-interactive waiting messages for network timeouts and such. */
	TSharedPtr<SShooterWaitDialog> WaitMessageWidget;

	/** Controller to ignore for pairing changes. -1 to skip ignore. */
	int32 IgnorePairingChangeForControllerId;

	/** Last connection status that was passed into the HandleNetworkConnectionStatusChanged hander */
	EOnlineServerConnectionStatus::Type	CurrentConnectionStatus;

	/** Delegate for callbacks to Tick */
	FTickerDelegate TickDelegate;

	/** Handle to various registered delegates */
	FDelegateHandle TickDelegateHandle;
	FDelegateHandle TravelLocalSessionFailureDelegateHandle;
	FDelegateHandle OnJoinSessionCompleteDelegateHandle;
	FDelegateHandle OnSearchSessionsCompleteDelegateHandle;
	FDelegateHandle OnStartSessionCompleteDelegateHandle;
	FDelegateHandle OnEndSessionCompleteDelegateHandle;
	FDelegateHandle OnDestroySessionCompleteDelegateHandle;
	FDelegateHandle OnCreatePresenceSessionCompleteDelegateHandle;

	/** Play Together on PS4 system event info */
	FShooterPlayTogetherInfo PlayTogetherInfo;

	/** Local player login status when the system is suspended */
	TArray<ELoginStatus::Type> LocalPlayerOnlineStatus;

	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;

	void HandleNetworkConnectionStatusChanged( const FString&, EOnlineServerConnectionStatus::Type LastConnectionStatus, EOnlineServerConnectionStatus::Type ConnectionStatus );

	void HandleSessionFailure( const FUniqueNetId& NetId, ESessionFailure::Type FailureType );
	
	void OnPreLoadMap(const FString& MapName);
	void OnPostLoadMap(UWorld*);
	void OnPostDemoPlay();

	virtual void HandleDemoPlaybackFailure( EDemoPlayFailure::Type FailureType, const FString& ErrorString ) override;

	/** Delegate function executed after checking privileges for starting quick match */
	void OnUserCanPlayInvite(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	/** Delegate function executed after checking privileges for Play Together on PS4 */
	void OnUserCanPlayTogether(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults);

	/** Delegate for ending a session */
	FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;

	void OnEndSessionComplete( FName SessionName, bool bWasSuccessful );

	void MaybeChangeState();
	void EndCurrentState(FName NextState);
	void BeginNewState(FName NewState, FName PrevState);

	void BeginPendingInviteState();
	void BeginWelcomeScreenState();
	void BeginMainMenuState();
	void BeginMessageMenuState();
	void BeginPlayingState();

	void EndPendingInviteState();
	void EndWelcomeScreenState();
	void EndMainMenuState();
	void EndMessageMenuState();
	void EndPlayingState();

	void ShowLoadingScreen();
	void AddNetworkFailureHandlers();
	void RemoveNetworkFailureHandlers();

	/** Called when there is an error trying to travel to a local session */
	void TravelLocalSessionFailure(UWorld *World, ETravelFailure::Type FailureType, const FString& ErrorString);

	/** Callback which is intended to be called upon joining session */
	void OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is intended to be called upon session creation */
	void OnCreatePresenceSessionComplete(FName SessionName, bool bWasSuccessful);

	/** Callback which is called after adding local users to a session */
	void OnRegisterLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);

	/** Called after all the local players are registered */
	void FinishSessionCreation(EOnJoinSessionCompleteResult::Type Result);

	/** Callback which is called after adding local users to a session we're joining */
	void OnRegisterJoiningLocalPlayerComplete(const FUniqueNetId& PlayerId, EOnJoinSessionCompleteResult::Type Result);

	/** Called after all the local players are registered in a session we're joining */
	void FinishJoinSession(EOnJoinSessionCompleteResult::Type Result);

	/** Send all invites for the current game session if we've created it because Play Together on PS4 was initiated*/
	void SendPlayTogetherInvites();

	/**
	* Creates the message menu, clears other menus and sets the KingState to Message.
	*
	* @param	Message				Main message body
	* @param	OKButtonString		String to use for 'OK' button
	* @param	CancelButtonString	String to use for 'Cancel' button
	*/
	void ShowMessageThenGoMain(const FText& Message, const FText& OKButtonString, const FText& CancelButtonString);

	/** Callback which is intended to be called upon finding sessions */
	void OnSearchSessionsComplete(bool bWasSuccessful);

	bool LoadFrontEndMap(const FString& MapName);

	/** Sets a rich presence string for all local players. */
	void SetPresenceForLocalPlayers(const FString& StatusStr, const FVariantData& PresenceData);

	/** Travel directly to the named session */
	void InternalTravelToSession(const FName& SessionName);

	/** Show messaging and punt to welcome screen */
	void HandleSignInChangeMessaging();

	// OSS delegates to handle
	void HandleUserLoginChanged(int32 GameUserIndex, ELoginStatus::Type PreviousLoginStatus, ELoginStatus::Type LoginStatus, const FUniqueNetId& UserId);

	// Callback to pause the game when the OS has constrained our app.
	void HandleAppWillDeactivate();

	// Callback occurs when game being suspended
	void HandleAppSuspend();

	// Callback occurs when game resuming
	void HandleAppResume();

	// Callback to process game licensing change notifications.
	void HandleAppLicenseUpdate();

	// Callback to handle safe frame size changes.
	void HandleSafeFrameChanged();

	// Callback to handle controller connection changes.
	void HandleControllerConnectionChange(bool bIsConnection, int32 Unused, int32 GameUserIndex);

	// Callback to handle controller pairing changes.
	FReply OnPairingUsePreviousProfile();

	// Callback to handle controller pairing changes.
	FReply OnPairingUseNewProfile();

	// Callback to handle controller pairing changes.
	void HandleControllerPairingChanged(int GameUserIndex, FControllerPairingChangedUserInfo PreviousUser, FControllerPairingChangedUserInfo NewUser);

	// Handle confirming the controller disconnected dialog.
	FReply OnControllerReconnectConfirm();	

protected:
	bool HandleOpenCommand(const TCHAR* Cmd, FOutputDevice& Ar, UWorld* InWorld);
};


