// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterMainMenu.h"
#include "ShooterGameLoadingScreen.h"
#include "ShooterStyle.h"
#include "ShooterMenuSoundsWidgetStyle.h"
#include "ShooterGameInstance.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "GenericPlatformChunkInstall.h"
#include "Online/ShooterOnlineGameSettings.h"
#include "OnlineSubsystemSessionSettings.h"
#include "SShooterConfirmationDialog.h"
#include "ShooterMenuItemWidgetStyle.h"
#include "ShooterGameUserSettings.h"
#include "ShooterGameViewportClient.h"
#include "ShooterPersistentUser.h"
#include "Player/ShooterLocalPlayer.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

#define MAX_BOT_COUNT 8

static const FString MapNames[] = { TEXT("Sanctuary"), TEXT("Highrise") };
static const FString JoinMapNames[] = { TEXT("Any"), TEXT("Sanctuary"), TEXT("Highrise") };
static const FName PackageNames[] = { TEXT("Sanctuary.umap"), TEXT("Highrise.umap") };
static const int DefaultTDMMap = 1;
static const int DefaultFFAMap = 0; 
static const float QuickmatchUIAnimationTimeDuration = 30.f;

//use an EMap index, get back the ChunkIndex that map should be part of.
//Instead of this mapping we should really use the AssetRegistry to query for chunk mappings, but maps aren't members of the AssetRegistry yet.
static const int ChunkMapping[] = { 1, 2 };

#if PLATFORM_SWITCH
#	define LOGIN_REQUIRED_FOR_ONLINE_PLAY 1
#else
#	define LOGIN_REQUIRED_FOR_ONLINE_PLAY 0
#endif

#if PLATFORM_SWITCH
#	define CONSOLE_LAN_SUPPORTED 1
#else
#	define CONSOLE_LAN_SUPPORTED 0
#endif

FShooterMainMenu::~FShooterMainMenu()
{
	auto Sessions = Online::GetSessionInterface();
	CleanupOnlinePrivilegeTask();
}

void FShooterMainMenu::Construct(TWeakObjectPtr<UShooterGameInstance> _GameInstance, TWeakObjectPtr<ULocalPlayer> _PlayerOwner)
{
	bShowingDownloadPct = false;
	bAnimateQuickmatchSearchingUI = false;
	bUsedInputToCancelQuickmatchSearch = false;
	bQuickmatchSearchRequestCanceled = false;
	bIncQuickMAlpha = false;
	PlayerOwner = _PlayerOwner;
	MatchType = EMatchType::Custom;

	check(_GameInstance.IsValid());
	auto Sessions = Online::GetSessionInterface();

	GameInstance = _GameInstance;
	PlayerOwner = _PlayerOwner;

	OnCancelMatchmakingCompleteDelegate = FOnCancelMatchmakingCompleteDelegate::CreateSP(this, &FShooterMainMenu::OnCancelMatchmakingComplete);
	OnMatchmakingCompleteDelegate = FOnMatchmakingCompleteDelegate::CreateSP(this, &FShooterMainMenu::OnMatchmakingComplete);
	
	// read user settings
#if SHOOTER_CONSOLE_UI
	bIsLanMatch = FParse::Param(FCommandLine::Get(), TEXT("forcelan"));
#else
	UShooterGameUserSettings* const UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
	bIsLanMatch = UserSettings->IsLanMatch();
	bIsDedicatedServer = UserSettings->IsDedicatedServer();
#endif

	BotsCountOpt = 1;
	bIsRecordingDemo = false;

	if(GetPersistentUser())
	{
		BotsCountOpt = GetPersistentUser()->GetBotsCount();
		bIsRecordingDemo = GetPersistentUser()->IsRecordingDemos();
	}		

	// number entries 0 up to MAX_BOX_COUNT
	TArray<FText> BotsCountList;
	for (int32 i = 0; i <= MAX_BOT_COUNT; i++)
	{
		BotsCountList.Add(FText::AsNumber(i));
	}
	
	TArray<FText> MapList;
	for (int32 i = 0; i < ARRAY_COUNT(MapNames); ++i)
	{
		MapList.Add(FText::FromString(MapNames[i]));		
	}

	TArray<FText> JoinMapList;
	for (int32 i = 0; i < ARRAY_COUNT(JoinMapNames); ++i)
	{
		JoinMapList.Add(FText::FromString(JoinMapNames[i]));
	}

	TArray<FText> OnOffList;
	OnOffList.Add( LOCTEXT("Off","OFF") );
	OnOffList.Add( LOCTEXT("On","ON") );

	ShooterOptions = MakeShareable(new FShooterOptions()); 
	ShooterOptions->Construct(GetPlayerOwner());
	ShooterOptions->TellInputAboutKeybindings();
	ShooterOptions->OnApplyChanges.BindSP(this, &FShooterMainMenu::CloseSubMenu);

	//Now that we are here, build our menu 
	MenuWidget.Reset();
	MenuWidgetContainer.Reset();

	TArray<FString> Keys;
	GConfig->GetSingleLineArray(TEXT("/Script/SwitchRuntimeSettings.SwitchRuntimeSettings"), TEXT("LeaderboardMap"), Keys, GEngineIni);

	if (GEngine && GEngine->GameViewport)
	{		
		SAssignNew(MenuWidget, SShooterMenuWidget)
			.Cursor(EMouseCursor::Default)
			.PlayerOwner(GetPlayerOwner())
			.IsGameMenu(false);

		SAssignNew(MenuWidgetContainer, SWeakWidget)
			.PossiblyNullContent(MenuWidget);		

		TSharedPtr<FShooterMenuItem> RootMenuItem;

				
		SAssignNew(SplitScreenLobbyWidget, SShooterSplitScreenLobby)
			.PlayerOwner(GetPlayerOwner())
			.OnCancelClicked(FOnClicked::CreateSP(this, &FShooterMainMenu::OnSplitScreenBackedOut)) 
			.OnPlayClicked(FOnClicked::CreateSP(this, &FShooterMainMenu::OnSplitScreenPlay));

		FText Msg = LOCTEXT("No matches could be found", "No matches could be found");
		FText OKButtonString = NSLOCTEXT("DialogButtons", "OKAY", "OK");
		QuickMatchFailureWidget = SNew(SShooterConfirmationDialog).PlayerOwner(PlayerOwner)			
			.MessageText(Msg)
			.ConfirmText(OKButtonString)
			.CancelText(FText())
			.OnConfirmClicked(FOnClicked::CreateRaw(this, &FShooterMainMenu::OnQuickMatchFailureUICancel))
			.OnCancelClicked(FOnClicked::CreateRaw(this, &FShooterMainMenu::OnQuickMatchFailureUICancel));

		Msg = LOCTEXT("Searching for Match...", "SEARCHING FOR MATCH...");
		OKButtonString = LOCTEXT("Stop", "STOP");
		QuickMatchSearchingWidget = SNew(SShooterConfirmationDialog).PlayerOwner(PlayerOwner)			
			.MessageText(Msg)
			.ConfirmText(OKButtonString)
			.CancelText(FText())
			.OnConfirmClicked(FOnClicked::CreateRaw(this, &FShooterMainMenu::OnQuickMatchSearchingUICancel))
			.OnCancelClicked(FOnClicked::CreateRaw(this, &FShooterMainMenu::OnQuickMatchSearchingUICancel));

		SAssignNew(SplitScreenLobbyWidgetContainer, SWeakWidget)
			.PossiblyNullContent(SplitScreenLobbyWidget);		

		SAssignNew(QuickMatchFailureWidgetContainer, SWeakWidget)
			.PossiblyNullContent(QuickMatchFailureWidget);	

		SAssignNew(QuickMatchSearchingWidgetContainer, SWeakWidget)
			.PossiblyNullContent(QuickMatchSearchingWidget);

		FText StoppingOKButtonString = LOCTEXT("Stopping", "STOPPING...");
		QuickMatchStoppingWidget = SNew(SShooterConfirmationDialog).PlayerOwner(PlayerOwner)			
			.MessageText(Msg)
			.ConfirmText(StoppingOKButtonString)
			.CancelText(FText())
			.OnConfirmClicked(FOnClicked())
			.OnCancelClicked(FOnClicked());

		SAssignNew(QuickMatchStoppingWidgetContainer, SWeakWidget)
			.PossiblyNullContent(QuickMatchStoppingWidget);

#if PLATFORM_XBOXONE
		TSharedPtr<FShooterMenuItem> MenuItem;

		// HOST ONLINE menu option
		{
			MenuItem = MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("HostCustom", "HOST CUSTOM"), this, &FShooterMainMenu::OnHostOnlineSelected);

			// submenu under "HOST ONLINE"
			MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("TDMLong", "TEAM DEATHMATCH"), this, &FShooterMainMenu::OnSplitScreenSelected);

			TSharedPtr<FShooterMenuItem> NumberOfBotsOption = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("NumberOfBots", "NUMBER OF BOTS"), BotsCountList, this, &FShooterMainMenu::BotCountOptionChanged);				
			NumberOfBotsOption->SelectedMultiChoice = BotsCountOpt;																

			HostOnlineMapOption = MenuHelper::AddMenuOption(MenuItem, LOCTEXT("SELECTED_LEVEL", "Map"), MapList);
		}

		// JOIN menu option
		{
			// JOIN menu option
			MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("FindCustom", "FIND CUSTOM"), this, &FShooterMainMenu::OnJoinServer);

			// Server list widget that will be called up if appropriate
			MenuHelper::AddCustomMenuItem(JoinServerItem,SAssignNew(ServerListWidget,SShooterServerList).OwnerWidget(MenuWidget).PlayerOwner(GetPlayerOwner()));
		}

		// QUICK MATCH menu option
		{
			MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("QuickMatch", "QUICK MATCH"), this, &FShooterMainMenu::OnQuickMatchSelected);
		}

		// HOST OFFLINE menu option
		{
			MenuItem = MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("PlayOffline", "PLAY OFFLINE"),this, &FShooterMainMenu::OnHostOfflineSelected);

			// submenu under "HOST OFFLINE"
			MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("TDMLong", "TEAM DEATHMATCH"), this, &FShooterMainMenu::OnSplitScreenSelected);

			TSharedPtr<FShooterMenuItem> NumberOfBotsOption = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("NumberOfBots", "NUMBER OF BOTS"), BotsCountList, this, &FShooterMainMenu::BotCountOptionChanged);				
			NumberOfBotsOption->SelectedMultiChoice = BotsCountOpt;																

			HostOfflineMapOption = MenuHelper::AddMenuOption(MenuItem, LOCTEXT("SELECTED_LEVEL", "Map"), MapList);
		}
#elif SHOOTER_CONSOLE_UI
		TSharedPtr<FShooterMenuItem> MenuItem;

		// HOST ONLINE menu option
		{
			HostOnlineMenuItem = MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("HostOnline", "HOST ONLINE"), this, &FShooterMainMenu::OnHostOnlineSelected);

			// submenu under "HOST ONLINE"
#if LOGIN_REQUIRED_FOR_ONLINE_PLAY
			MenuHelper::AddMenuItemSP(HostOnlineMenuItem, LOCTEXT("TDMLong", "TEAM DEATHMATCH"), this, &FShooterMainMenu::OnSplitScreenSelectedHostOnlineLoginRequired);
#else
			MenuHelper::AddMenuItemSP(HostOnlineMenuItem, LOCTEXT("TDMLong", "TEAM DEATHMATCH"), this, &FShooterMainMenu::OnSplitScreenSelectedHostOnline);
#endif

			TSharedPtr<FShooterMenuItem> NumberOfBotsOption = MenuHelper::AddMenuOptionSP(HostOnlineMenuItem, LOCTEXT("NumberOfBots", "NUMBER OF BOTS"), BotsCountList, this, &FShooterMainMenu::BotCountOptionChanged);
			NumberOfBotsOption->SelectedMultiChoice = BotsCountOpt;																

			HostOnlineMapOption = MenuHelper::AddMenuOption(HostOnlineMenuItem, LOCTEXT("SELECTED_LEVEL", "Map"), MapList);
#if CONSOLE_LAN_SUPPORTED
			HostLANItem = MenuHelper::AddMenuOptionSP(HostOnlineMenuItem, LOCTEXT("LanMatch", "LAN"), OnOffList, this, &FShooterMainMenu::LanMatchChanged);
			HostLANItem->SelectedMultiChoice = bIsLanMatch;
#endif
		}

		// HOST OFFLINE menu option
		{
			MenuItem = MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("HostOffline", "HOST OFFLINE"),this, &FShooterMainMenu::OnHostOfflineSelected);

			// submenu under "HOST OFFLINE"
			MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("TDMLong", "TEAM DEATHMATCH"), this, &FShooterMainMenu::OnSplitScreenSelected);

			TSharedPtr<FShooterMenuItem> NumberOfBotsOption = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("NumberOfBots", "NUMBER OF BOTS"), BotsCountList, this, &FShooterMainMenu::BotCountOptionChanged);				
			NumberOfBotsOption->SelectedMultiChoice = BotsCountOpt;																

			HostOfflineMapOption = MenuHelper::AddMenuOption(MenuItem, LOCTEXT("SELECTED_LEVEL", "Map"), MapList);
		}

		// QUICK MATCH menu option
		{
#if LOGIN_REQUIRED_FOR_ONLINE_PLAY
			MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("QuickMatch", "QUICK MATCH"), this, &FShooterMainMenu::OnQuickMatchSelectedLoginRequired);
#else
			MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("QuickMatch", "QUICK MATCH"), this, &FShooterMainMenu::OnQuickMatchSelected);
#endif
		}

		// JOIN menu option
		{
			// JOIN menu option
			MenuItem = MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Join", "JOIN"), this, &FShooterMainMenu::OnJoinSelected);

			// submenu under "join"
#if LOGIN_REQUIRED_FOR_ONLINE_PLAY
			MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("Server", "SERVER"), this, &FShooterMainMenu::OnJoinServerLoginRequired);
#else
			MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("Server", "SERVER"), this, &FShooterMainMenu::OnJoinServer);
#endif
			JoinMapOption = MenuHelper::AddMenuOption(MenuItem, LOCTEXT("SELECTED_LEVEL", "Map"), JoinMapList);

			// Server list widget that will be called up if appropriate
			MenuHelper::AddCustomMenuItem(JoinServerItem,SAssignNew(ServerListWidget,SShooterServerList).OwnerWidget(MenuWidget).PlayerOwner(GetPlayerOwner()));

#if CONSOLE_LAN_SUPPORTED
			JoinLANItem = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("LanMatch", "LAN"), OnOffList, this, &FShooterMainMenu::LanMatchChanged);
			JoinLANItem->SelectedMultiChoice = bIsLanMatch;
#endif
		}

#else
		TSharedPtr<FShooterMenuItem> MenuItem;
		// HOST menu option
		MenuItem = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Host", "HOST"));

		// submenu under "host"
		MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("FFALong", "FREE FOR ALL"), this, &FShooterMainMenu::OnUIHostFreeForAll);
		MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("TDMLong", "TEAM DEATHMATCH"), this, &FShooterMainMenu::OnUIHostTeamDeathMatch);

		TSharedPtr<FShooterMenuItem> NumberOfBotsOption = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("NumberOfBots", "NUMBER OF BOTS"), BotsCountList, this, &FShooterMainMenu::BotCountOptionChanged);
		NumberOfBotsOption->SelectedMultiChoice = BotsCountOpt;

		HostOnlineMapOption = MenuHelper::AddMenuOption(MenuItem, LOCTEXT("SELECTED_LEVEL", "Map"), MapList);

		HostLANItem = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("LanMatch", "LAN"), OnOffList, this, &FShooterMainMenu::LanMatchChanged);
		HostLANItem->SelectedMultiChoice = bIsLanMatch;

		RecordDemoItem = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("RecordDemo", "Record Demo"), OnOffList, this, &FShooterMainMenu::RecordDemoChanged);
		RecordDemoItem->SelectedMultiChoice = bIsRecordingDemo;

		// JOIN menu option
		MenuItem = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Join", "JOIN"));

		// submenu under "join"
		MenuHelper::AddMenuItemSP(MenuItem, LOCTEXT("Server", "SERVER"), this, &FShooterMainMenu::OnJoinServer);
		JoinLANItem = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("LanMatch", "LAN"), OnOffList, this, &FShooterMainMenu::LanMatchChanged);
		JoinLANItem->SelectedMultiChoice = bIsLanMatch;

		DedicatedItem = MenuHelper::AddMenuOptionSP(MenuItem, LOCTEXT("Dedicated", "Dedicated"), OnOffList, this, &FShooterMainMenu::DedicatedServerChanged);
		DedicatedItem->SelectedMultiChoice = bIsDedicatedServer;

		// Server list widget that will be called up if appropriate
		MenuHelper::AddCustomMenuItem(JoinServerItem,SAssignNew(ServerListWidget,SShooterServerList).OwnerWidget(MenuWidget).PlayerOwner(GetPlayerOwner()));
#endif

		// Leaderboards
		MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Leaderboards", "LEADERBOARDS"), this, &FShooterMainMenu::OnShowLeaderboard);
		MenuHelper::AddCustomMenuItem(LeaderboardItem,SAssignNew(LeaderboardWidget,SShooterLeaderboard).OwnerWidget(MenuWidget).PlayerOwner(GetPlayerOwner()));

#if !SHOOTER_CONSOLE_UI

		// Demos
		{
			MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Demos", "DEMOS"), this, &FShooterMainMenu::OnShowDemoBrowser);
			MenuHelper::AddCustomMenuItem(DemoBrowserItem,SAssignNew(DemoListWidget,SShooterDemoList).OwnerWidget(MenuWidget).PlayerOwner(GetPlayerOwner()));
		}
#endif

		// Options
		MenuHelper::AddExistingMenuItem(RootMenuItem, ShooterOptions->OptionsItem.ToSharedRef());

		if(FSlateApplication::Get().SupportsSystemHelp())
		{
			TSharedPtr<FShooterMenuItem> HelpSubMenu = MenuHelper::AddMenuItem(RootMenuItem, LOCTEXT("Help", "HELP"));
			HelpSubMenu->OnConfirmMenuItem.BindStatic([](){ FSlateApplication::Get().ShowSystemHelp(); });
		}

		// QUIT option (for PC)
#if !SHOOTER_CONSOLE_UI
		MenuHelper::AddMenuItemSP(RootMenuItem, LOCTEXT("Quit", "QUIT"), this, &FShooterMainMenu::OnUIQuit);
#endif

		MenuWidget->CurrentMenuTitle = LOCTEXT("MainMenu","MAIN MENU");
		MenuWidget->OnGoBack.BindSP(this, &FShooterMainMenu::OnMenuGoBack);
		MenuWidget->MainMenu = MenuWidget->CurrentMenu = RootMenuItem->SubMenu;
		MenuWidget->OnMenuHidden.BindSP(this, &FShooterMainMenu::OnMenuHidden);

		ShooterOptions->UpdateOptions();
		MenuWidget->BuildAndShowMenu();
	}
}

void FShooterMainMenu::AddMenuToGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		UGameViewportClient* const GVC = GEngine->GameViewport;
		GVC->AddViewportWidgetContent(MenuWidgetContainer.ToSharedRef());
	}
}

void FShooterMainMenu::RemoveMenuFromGameViewport()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(MenuWidgetContainer.ToSharedRef());
	}
}

void FShooterMainMenu::Tick(float DeltaSeconds)
{
	if (bAnimateQuickmatchSearchingUI)
	{
		FLinearColor QuickMColor = QuickMatchSearchingWidget->GetColorAndOpacity();
		if (bIncQuickMAlpha)
		{
			if (QuickMColor.A >= 1.f)
			{
				bIncQuickMAlpha = false;
			}
			else
			{
				QuickMColor.A += DeltaSeconds;
			}
		}
		else
		{
			if (QuickMColor.A <= .1f)
			{
				bIncQuickMAlpha = true;
			}
			else
			{
				QuickMColor.A -= DeltaSeconds;
			}
		}
		QuickMatchSearchingWidget->SetColorAndOpacity(QuickMColor);
		QuickMatchStoppingWidget->SetColorAndOpacity(QuickMColor);
	}

	IPlatformChunkInstall* ChunkInstaller = FPlatformMisc::GetPlatformChunkInstall();
	if (ChunkInstaller)
	{
		EMap SelectedMap = GetSelectedMap();
		// use assetregistry when maps are added to it.
		int32 MapChunk = ChunkMapping[(int)SelectedMap];
		EChunkLocation::Type ChunkLocation = ChunkInstaller->GetChunkLocation(MapChunk);

		FText UpdatedText;
		bool bUpdateText = false;
		if (ChunkLocation == EChunkLocation::NotAvailable)
		{			
			float PercentComplete = FMath::Min(ChunkInstaller->GetChunkProgress(MapChunk, EChunkProgressReportingType::PercentageComplete), 100.0f);									
			UpdatedText = FText::FromString(FString::Printf(TEXT("%s %4.0f%%"),*LOCTEXT("SELECTED_LEVEL", "Map").ToString(), PercentComplete));
			bUpdateText = true;
			bShowingDownloadPct = true;
		}
		else if (bShowingDownloadPct)
		{
			UpdatedText = LOCTEXT("SELECTED_LEVEL", "Map");			
			bUpdateText = true;
			bShowingDownloadPct = false;			
		}

		if (bUpdateText)
		{
			if (GameInstance.IsValid() && GameInstance->GetOnlineMode() != EOnlineMode::Offline && HostOnlineMapOption.IsValid())
			{
				HostOnlineMapOption->SetText(UpdatedText);
			}
			else if (HostOfflineMapOption.IsValid())
			{
				HostOfflineMapOption->SetText(UpdatedText);
			}
		}
	}
}

bool FShooterMainMenu::IsTickable() const
{
	return true;
}

TStatId FShooterMainMenu::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FShooterMainMenu, STATGROUP_Tickables);
}

bool FShooterMainMenu::IsTickableWhenPaused() const
{
	return true;
}

void FShooterMainMenu::OnMenuHidden()
{	
#if SHOOTER_CONSOLE_UI
	// Menu was hidden from the top-level main menu, on consoles show the welcome screen again.
	if ( ensure(GameInstance.IsValid()))
	{
		GameInstance->GotoState(ShooterGameInstanceState::WelcomeScreen);
	}
#else
	RemoveMenuFromGameViewport();
#endif
}


void FShooterMainMenu::OnQuickMatchSelectedLoginRequired()
{
	IOnlineIdentityPtr Identity = IOnlineSubsystem::Get()->GetIdentityInterface();

	int32 ControllerId = GetPlayerOwner()->GetControllerId();

	OnLoginCompleteDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(ControllerId, FOnLoginCompleteDelegate::CreateRaw(this, &FShooterMainMenu::OnLoginCompleteQuickmatch));
	Identity->Login(ControllerId, FOnlineAccountCredentials());
}

void FShooterMainMenu::OnLoginCompleteQuickmatch(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IOnlineSubsystem::Get()->GetIdentityInterface()->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, OnLoginCompleteDelegateHandle);

	OnQuickMatchSelected();
}

void FShooterMainMenu::OnQuickMatchSelected()
{
	bQuickmatchSearchRequestCanceled = false;
#if SHOOTER_CONSOLE_UI
	if ( !ValidatePlayerForOnlinePlay(GetPlayerOwner()) )
	{
		return;
	}
#endif

	StartOnlinePrivilegeTask(IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateSP(this, &FShooterMainMenu::OnUserCanPlayOnlineQuickMatch));
}

void FShooterMainMenu::OnUserCanPlayOnlineQuickMatch(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	CleanupOnlinePrivilegeTask();
	MenuWidget->LockControls(false);
	if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{
		if (GameInstance.IsValid())
		{
			GameInstance->SetOnlineMode(EOnlineMode::Online);
		}

		MatchType = EMatchType::Quick;

		SplitScreenLobbyWidget->SetIsJoining(false);

		// Skip splitscreen for PS4
#if PLATFORM_PS4
		BeginQuickMatchSearch();
#else
		UGameViewportClient* const GVC = GEngine->GameViewport;

		RemoveMenuFromGameViewport();
		GVC->AddViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());

		SplitScreenLobbyWidget->Clear();
		FSlateApplication::Get().SetKeyboardFocus(SplitScreenLobbyWidget);
#endif
	}
	else if (GameInstance.IsValid())
	{

		GameInstance->DisplayOnlinePrivilegeFailureDialogs(UserId, Privilege, PrivilegeResults);
	}
}

FReply FShooterMainMenu::OnConfirmGeneric()
{
	UShooterGameViewportClient* ShooterViewport = Cast<UShooterGameViewportClient>(GameInstance->GetGameViewportClient());
	if (ShooterViewport)
	{
		ShooterViewport->HideDialog();
	}

	return FReply::Handled();
}

void FShooterMainMenu::BeginQuickMatchSearch()
{
	auto Sessions = Online::GetSessionInterface();
	if(!Sessions.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("Quick match is not supported: couldn't find online session interface."));
		return;
	}

	if (GetPlayerOwnerControllerId() == -1)
	{
		UE_LOG(LogOnline, Warning, TEXT("Quick match is not supported: Could not get controller id from player owner"));
		return;
	}

	QuickMatchSearchSettings = MakeShareable(new FShooterOnlineSearchSettings(false, true));
	QuickMatchSearchSettings->QuerySettings.Set(SEARCH_XBOX_LIVE_HOPPER_NAME, FString("TeamDeathmatch"), EOnlineComparisonOp::Equals);
	QuickMatchSearchSettings->QuerySettings.Set(SEARCH_XBOX_LIVE_SESSION_TEMPLATE_NAME, FString("MatchSession"), EOnlineComparisonOp::Equals);
	QuickMatchSearchSettings->TimeoutInSeconds = 120.0f;

	FShooterOnlineSessionSettings SessionSettings(false, true, 8);
	SessionSettings.Set(SETTING_GAMEMODE, FString("TDM"), EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
	SessionSettings.Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);

	auto QuickMatchSearchSettingsRef = QuickMatchSearchSettings.ToSharedRef();
	
	DisplayQuickmatchSearchingUI();

	Sessions->ClearOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegateHandle);
	OnMatchmakingCompleteDelegateHandle = Sessions->AddOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegate);

	// Perform matchmaking with all local players
	TArray<TSharedRef<const FUniqueNetId>> LocalPlayerIds;
	for (int i = 0; i < GameInstance->GetNumLocalPlayers(); ++i)
	{
		auto PlayerId = GameInstance->GetLocalPlayerByIndex(i)->GetPreferredUniqueNetId();
		if (PlayerId.IsValid())
		{
			LocalPlayerIds.Add(PlayerId.ToSharedRef());
		}
	}

	if (!Sessions->StartMatchmaking(LocalPlayerIds, NAME_GameSession, SessionSettings, QuickMatchSearchSettingsRef))
	{
		OnMatchmakingComplete(NAME_GameSession, false);
	}
}


void FShooterMainMenu::OnSplitScreenSelectedHostOnlineLoginRequired()
{
	IOnlineIdentityPtr Identity = IOnlineSubsystem::Get()->GetIdentityInterface();
	int32 ControllerId = GetPlayerOwner()->GetControllerId();

	if (bIsLanMatch)
	{
		Identity->Logout(ControllerId);
		OnSplitScreenSelected();
	}
	else
	{
		OnLoginCompleteDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(ControllerId, FOnLoginCompleteDelegate::CreateRaw(this, &FShooterMainMenu::OnLoginCompleteHostOnline));
		Identity->Login(ControllerId, FOnlineAccountCredentials());
	}
}

void FShooterMainMenu::OnSplitScreenSelected()
{
	if (!IsMapReady())
	{
		return;
	}

	RemoveMenuFromGameViewport();

#if PLATFORM_PS4
	if (GameInstance.IsValid())
	{
		if (GameInstance->GetOnlineMode() == EOnlineMode::Online)
		{
			OnUIHostTeamDeathMatch();
		}
		else
		{
			UGameViewportClient* const GVC = GEngine->GameViewport;
			GVC->AddViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());

			SplitScreenLobbyWidget->Clear();
			FSlateApplication::Get().SetKeyboardFocus(SplitScreenLobbyWidget);
		}
	}
#else

	UGameViewportClient* const GVC = GEngine->GameViewport;
	GVC->AddViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());

	SplitScreenLobbyWidget->Clear();
	FSlateApplication::Get().SetKeyboardFocus(SplitScreenLobbyWidget);
#endif
}

void FShooterMainMenu::OnHostOnlineSelected()
{
#if SHOOTER_CONSOLE_UI
	if (!ValidatePlayerIsSignedIn(GetPlayerOwner()))
	{
		return;
	}
#endif

	MatchType = EMatchType::Custom;

	EOnlineMode NewOnlineMode = bIsLanMatch ? EOnlineMode::LAN : EOnlineMode::Online;
	if (GameInstance.IsValid())
	{
		GameInstance->SetOnlineMode(NewOnlineMode);
	}
	SplitScreenLobbyWidget->SetIsJoining(false);
	MenuWidget->EnterSubMenu();
}

void FShooterMainMenu::OnUserCanPlayHostOnline(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	CleanupOnlinePrivilegeTask();
	MenuWidget->LockControls(false);
	if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)	
	{
		OnSplitScreenSelected();
	}
	else if (GameInstance.IsValid())
	{
		GameInstance->DisplayOnlinePrivilegeFailureDialogs(UserId, Privilege, PrivilegeResults);
	}
}

void FShooterMainMenu::OnLoginCompleteHostOnline(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IOnlineSubsystem::Get()->GetIdentityInterface()->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, OnLoginCompleteDelegateHandle);

	OnSplitScreenSelectedHostOnline();
}

void FShooterMainMenu::OnSplitScreenSelectedHostOnline()
{
#if SHOOTER_CONSOLE_UI
	if (!ValidatePlayerForOnlinePlay(GetPlayerOwner()))
	{
		return;
	}
#endif

	StartOnlinePrivilegeTask(IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateSP(this, &FShooterMainMenu::OnUserCanPlayHostOnline));
}
void FShooterMainMenu::StartOnlinePrivilegeTask(const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate& Delegate)
{
	if (GameInstance.IsValid())
	{
		// Lock controls for the duration of the async task
		MenuWidget->LockControls(true);
		TSharedPtr<const FUniqueNetId> UserId;
		if (PlayerOwner.IsValid())
		{
			UserId = PlayerOwner->GetPreferredUniqueNetId();
		}
		GameInstance->StartOnlinePrivilegeTask(Delegate, EUserPrivileges::CanPlayOnline, UserId);
	}	
}

void FShooterMainMenu::CleanupOnlinePrivilegeTask()
{
	if (GameInstance.IsValid())
	{
		GameInstance->CleanupOnlinePrivilegeTask();
	}
}

void FShooterMainMenu::OnHostOfflineSelected()
{
	MatchType = EMatchType::Custom;

#if LOGIN_REQUIRED_FOR_ONLINE_PLAY
	IOnlineSubsystem::Get()->GetIdentityInterface()->Logout(GetPlayerOwner()->GetControllerId());
#endif

	if (GameInstance.IsValid())
	{
		GameInstance->SetOnlineMode(EOnlineMode::Offline);
	}
	SplitScreenLobbyWidget->SetIsJoining( false );

	MenuWidget->EnterSubMenu();
}

FReply FShooterMainMenu::OnSplitScreenBackedOut()
{	
	SplitScreenLobbyWidget->Clear();
	SplitScreenBackedOut();
	return FReply::Handled();
}

FReply FShooterMainMenu::OnSplitScreenPlay()
{
	switch ( MatchType )
	{
		case EMatchType::Custom:
		{
#if SHOOTER_CONSOLE_UI
			if ( SplitScreenLobbyWidget->GetIsJoining() )
			{
#if 1
				// Until we can make split-screen menu support sub-menus, we need to do it this way
				if (GEngine && GEngine->GameViewport)
				{
					GEngine->GameViewport->RemoveViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());
				}
				AddMenuToGameViewport();

				FSlateApplication::Get().SetKeyboardFocus(MenuWidget);	

				MenuWidget->NextMenu = JoinServerItem->SubMenu;
				ServerListWidget->BeginServerSearch(bIsLanMatch, bIsDedicatedServer, ("ANY"));
				ServerListWidget->UpdateServerList();
				MenuWidget->EnterSubMenu();
#else
				SplitScreenLobbyWidget->NextMenu = JoinServerItem->SubMenu;
				ServerListWidget->BeginServerSearch(bIsLanMatch, bIsDedicatedServer, TEXT("ANY"));
				ServerListWidget->UpdateServerList();
				SplitScreenLobbyWidget->EnterSubMenu();
#endif
			}
			else
#endif
			{
				if (GEngine && GEngine->GameViewport)
				{
					GEngine->GameViewport->RemoveViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());
				}
				OnUIHostTeamDeathMatch();
			}
			break;
		}

		case EMatchType::Quick:
		{
			if (GEngine && GEngine->GameViewport)
			{
				GEngine->GameViewport->RemoveViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());
			}
			BeginQuickMatchSearch();
			break;
		}
	}

	return FReply::Handled();
}

void FShooterMainMenu::SplitScreenBackedOut()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());	
	}
	AddMenuToGameViewport();

	FSlateApplication::Get().SetKeyboardFocus(MenuWidget);	
}

void FShooterMainMenu::HelperQuickMatchSearchingUICancel(bool bShouldRemoveSession)
{
	auto Sessions = Online::GetSessionInterface();
	if (bShouldRemoveSession && Sessions.IsValid())
	{
		if (PlayerOwner.IsValid() && PlayerOwner->GetPreferredUniqueNetId().IsValid())
		{
			UGameViewportClient* const GVC = GEngine->GameViewport;
			GVC->RemoveViewportWidgetContent(QuickMatchSearchingWidgetContainer.ToSharedRef());
			GVC->AddViewportWidgetContent(QuickMatchStoppingWidgetContainer.ToSharedRef());
			FSlateApplication::Get().SetKeyboardFocus(QuickMatchStoppingWidgetContainer);
			
			OnCancelMatchmakingCompleteDelegateHandle = Sessions->AddOnCancelMatchmakingCompleteDelegate_Handle(OnCancelMatchmakingCompleteDelegate);
			Sessions->CancelMatchmaking(*PlayerOwner->GetPreferredUniqueNetId(), NAME_GameSession);
		}
	}
	else
	{
		UGameViewportClient* const GVC = GEngine->GameViewport;
		GVC->RemoveViewportWidgetContent(QuickMatchSearchingWidgetContainer.ToSharedRef());
		AddMenuToGameViewport();
		FSlateApplication::Get().SetKeyboardFocus(MenuWidget);
	}
}

FReply FShooterMainMenu::OnQuickMatchSearchingUICancel()
{
	HelperQuickMatchSearchingUICancel(true);
	bUsedInputToCancelQuickmatchSearch = true;
	bQuickmatchSearchRequestCanceled = true;
	return FReply::Handled();
}

FReply FShooterMainMenu::OnQuickMatchFailureUICancel()
{
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(QuickMatchFailureWidgetContainer.ToSharedRef());
	}
	AddMenuToGameViewport();
	FSlateApplication::Get().SetKeyboardFocus(MenuWidget);
	return FReply::Handled();
}

void FShooterMainMenu::DisplayQuickmatchFailureUI()
{
	UGameViewportClient* const GVC = GEngine->GameViewport;
	RemoveMenuFromGameViewport();
	GVC->AddViewportWidgetContent(QuickMatchFailureWidgetContainer.ToSharedRef());
	FSlateApplication::Get().SetKeyboardFocus(QuickMatchFailureWidget);
}

void FShooterMainMenu::DisplayQuickmatchSearchingUI()
{
	UGameViewportClient* const GVC = GEngine->GameViewport;
	RemoveMenuFromGameViewport();
	GVC->AddViewportWidgetContent(QuickMatchSearchingWidgetContainer.ToSharedRef());
	FSlateApplication::Get().SetKeyboardFocus(QuickMatchSearchingWidget);
	bAnimateQuickmatchSearchingUI = true;
}

void FShooterMainMenu::OnMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
	auto SessionInterface = Online::GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("OnMatchmakingComplete: Couldn't find session interface."));
		return;
	}

	SessionInterface->ClearOnMatchmakingCompleteDelegate_Handle(OnMatchmakingCompleteDelegateHandle);

	if (bQuickmatchSearchRequestCanceled && bUsedInputToCancelQuickmatchSearch)
	{
		bQuickmatchSearchRequestCanceled = false;
		// Clean up the session in case we get this event after canceling
		auto Sessions = Online::GetSessionInterface();
		if (bWasSuccessful && Sessions.IsValid())
		{
			if (PlayerOwner.IsValid() && PlayerOwner->GetPreferredUniqueNetId().IsValid())
			{
				Sessions->DestroySession(NAME_GameSession);
			}
		}
		return;
	}

	if (bAnimateQuickmatchSearchingUI)
	{
		bAnimateQuickmatchSearchingUI = false;
		HelperQuickMatchSearchingUICancel(false);
		bUsedInputToCancelQuickmatchSearch = false;
	}
	else
	{
		return;
	}

	if (!bWasSuccessful)
	{
		UE_LOG(LogOnline, Warning, TEXT("Matchmaking was unsuccessful."));
		DisplayQuickmatchFailureUI();
		return;
	}

	UE_LOG(LogOnline, Log, TEXT("Matchmaking successful! Session name is %s."), *SessionName.ToString());

	if (GetPlayerOwner() == NULL)
	{
		UE_LOG(LogOnline, Warning, TEXT("OnMatchmakingComplete: No owner."));
		return;
	}

	auto MatchmadeSession = SessionInterface->GetNamedSession(SessionName);

	if (!MatchmadeSession)
	{
		UE_LOG(LogOnline, Warning, TEXT("OnMatchmakingComplete: No session."));
		return;
	}

	if(!MatchmadeSession->OwningUserId.IsValid())
	{
		UE_LOG(LogOnline, Warning, TEXT("OnMatchmakingComplete: No session owner/host."));
		return;
	}

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(QuickMatchSearchingWidgetContainer.ToSharedRef());
	}
	bAnimateQuickmatchSearchingUI = false;

	UE_LOG(LogOnline, Log, TEXT("OnMatchmakingComplete: Session host is %d."), *MatchmadeSession->OwningUserId->ToString());

	if (ensure(GameInstance.IsValid()))
	{
		MenuWidget->LockControls(true);

		auto Subsystem = IOnlineSubsystem::Get();
		if (Subsystem != nullptr && Subsystem->IsLocalPlayer(*MatchmadeSession->OwningUserId))
		{
			// This console is the host, start the map.
			GameInstance->BeginHostingQuickMatch();
		}
		else
		{
			// We are the client, join the host.
			GameInstance->TravelToSession(SessionName);
		}
	}
}

FShooterMainMenu::EMap FShooterMainMenu::GetSelectedMap() const
{
	if (GameInstance.IsValid() && GameInstance->GetOnlineMode() != EOnlineMode::Offline && HostOnlineMapOption.IsValid())
	{
		return (EMap)HostOnlineMapOption->SelectedMultiChoice;
	}
	else if (HostOfflineMapOption.IsValid())
	{
		return (EMap)HostOfflineMapOption->SelectedMultiChoice;
	}

	return EMap::ESancturary;	// Need to return something (we can hit this path in cooking)
}

void FShooterMainMenu::CloseSubMenu()
{
	MenuWidget->MenuGoBack(true);
}

void FShooterMainMenu::OnMenuGoBack(MenuPtr Menu)
{
	// if we are going back from options menu
	if (ShooterOptions->OptionsItem->SubMenu == Menu)
	{
		ShooterOptions->RevertChanges();
	}

	// In case a Play Together event was received, don't act on it
	// if the player changes their mind.
	if (HostOnlineMenuItem.IsValid() && HostOnlineMenuItem->SubMenu == Menu)
	{
		GameInstance->ResetPlayTogetherInfo();
	}

	// if we've backed all the way out we need to make sure online is false.
	if (MenuWidget->GetMenuLevel() == 1)
	{
		GameInstance->SetOnlineMode(EOnlineMode::Offline);
	}
}

void FShooterMainMenu::BotCountOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	BotsCountOpt = MultiOptionIndex;

	if(GetPersistentUser())
	{
		GetPersistentUser()->SetBotsCount(BotsCountOpt);
	}
}

void FShooterMainMenu::LanMatchChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	if (HostLANItem.IsValid())
	{
		HostLANItem->SelectedMultiChoice = MultiOptionIndex;
	}

	check(JoinLANItem.IsValid());
	JoinLANItem->SelectedMultiChoice = MultiOptionIndex;
	bIsLanMatch = MultiOptionIndex > 0;
	UShooterGameUserSettings* UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->SetLanMatch(bIsLanMatch);

	EOnlineMode NewOnlineMode = bIsLanMatch ? EOnlineMode::LAN : EOnlineMode::Online;
	if (GameInstance.IsValid())
	{
		GameInstance->SetOnlineMode(NewOnlineMode);
	}
}

void FShooterMainMenu::DedicatedServerChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	check(DedicatedItem.IsValid());
	DedicatedItem->SelectedMultiChoice = MultiOptionIndex;
	bIsDedicatedServer = MultiOptionIndex > 0;
	UShooterGameUserSettings* UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
	UserSettings->SetDedicatedServer(bIsDedicatedServer);
}

void FShooterMainMenu::RecordDemoChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	if (RecordDemoItem.IsValid())
	{
		RecordDemoItem->SelectedMultiChoice = MultiOptionIndex;
	}

	bIsRecordingDemo = MultiOptionIndex > 0;

	if(GetPersistentUser())
	{
		GetPersistentUser()->SetIsRecordingDemos(bIsRecordingDemo);
		GetPersistentUser()->SaveIfDirty();
	}
}

void FShooterMainMenu::OnUIHostFreeForAll()
{
#if WITH_EDITOR
	if (GIsEditor == true)
	{
		return;
	}
#endif
	if (!IsMapReady())
	{
		return;
	}

#if !SHOOTER_CONSOLE_UI
	if (GameInstance.IsValid())
	{
		GameInstance->SetOnlineMode(bIsLanMatch ? EOnlineMode::LAN : EOnlineMode::Online);
	}
#endif

	MenuWidget->LockControls(true);
	MenuWidget->HideMenu();

	UWorld* const World = GameInstance.IsValid() ? GameInstance->GetWorld() : nullptr;
	if (World && GetPlayerOwnerControllerId() != -1)
	{
	const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle<FShooterMenuSoundsStyle>("DefaultShooterMenuSoundsStyle");
		MenuHelper::PlaySoundAndCall(World, MenuSounds.StartGameSound, GetPlayerOwnerControllerId(), this, &FShooterMainMenu::HostFreeForAll);
	}
}

void FShooterMainMenu::OnUIHostTeamDeathMatch()
{
#if WITH_EDITOR
	if (GIsEditor == true)
	{
		return;
	}
#endif
	if (!IsMapReady())
	{
		return;
	}

#if !SHOOTER_CONSOLE_UI
	if (GameInstance.IsValid())
	{
		GameInstance->SetOnlineMode(bIsLanMatch ? EOnlineMode::LAN : EOnlineMode::Online);
	}
#endif

	MenuWidget->LockControls(true);
	MenuWidget->HideMenu();

	UWorld* const World = GameInstance.IsValid() ? GameInstance->GetWorld() : nullptr;
	if (World && GetPlayerOwnerControllerId() != -1)
	{
	const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle<FShooterMenuSoundsStyle>("DefaultShooterMenuSoundsStyle");
		MenuHelper::PlaySoundAndCall(World, MenuSounds.StartGameSound, GetPlayerOwnerControllerId(), this, &FShooterMainMenu::HostTeamDeathMatch);
	}
}

void FShooterMainMenu::HostGame(const FString& GameType)
{	
	if (ensure(GameInstance.IsValid()) && GetPlayerOwner() != NULL)
	{
		FString const StartURL = FString::Printf(TEXT("/Game/Maps/%s?game=%s%s%s?%s=%d%s"), *GetMapName(), *GameType, GameInstance->GetOnlineMode() != EOnlineMode::Offline ? TEXT("?listen") : TEXT(""), GameInstance->GetOnlineMode() == EOnlineMode::LAN ? TEXT("?bIsLanMatch") : TEXT(""), *AShooterGameMode::GetBotsCountOptionName(), BotsCountOpt, bIsRecordingDemo ? TEXT("?DemoRec") : TEXT("") );

		// Game instance will handle success, failure and dialogs
		GameInstance->HostGame(GetPlayerOwner(), GameType, StartURL);
	}
}

void FShooterMainMenu::HostFreeForAll()
{
	HostGame(LOCTEXT("FFA", "FFA").ToString());
}

void FShooterMainMenu::HostTeamDeathMatch()
{	
	HostGame(LOCTEXT("TDM", "TDM").ToString());
}

FReply FShooterMainMenu::OnConfirm()
{
	if (GEngine && GEngine->GameViewport)
	{
		UShooterGameViewportClient * ShooterViewport = Cast<UShooterGameViewportClient>(GEngine->GameViewport);

		if (ShooterViewport)
		{
			// Hide the previous dialog
			ShooterViewport->HideDialog();
		}
	}

	return FReply::Handled();
}

bool FShooterMainMenu::ValidatePlayerForOnlinePlay(ULocalPlayer* LocalPlayer)
{
	if (!ensure(GameInstance.IsValid()))
	{
		return false;
	}

	return GameInstance->ValidatePlayerForOnlinePlay(LocalPlayer);
}

bool FShooterMainMenu::ValidatePlayerIsSignedIn(ULocalPlayer* LocalPlayer)
{
	if (!ensure(GameInstance.IsValid()))
	{
		return false;
	}

	return GameInstance->ValidatePlayerIsSignedIn(LocalPlayer);
}

void FShooterMainMenu::OnJoinServerLoginRequired()
{
	IOnlineIdentityPtr Identity = IOnlineSubsystem::Get()->GetIdentityInterface();
	int32 ControllerId = GetPlayerOwner()->GetControllerId();

	if (bIsLanMatch)
	{
		Identity->Logout(ControllerId);
		OnUserCanPlayOnlineJoin(*GetPlayerOwner()->GetCachedUniqueNetId(), EUserPrivileges::CanPlayOnline, (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures);
	}
	else
	{
		OnLoginCompleteDelegateHandle = Identity->AddOnLoginCompleteDelegate_Handle(ControllerId, FOnLoginCompleteDelegate::CreateRaw(this, &FShooterMainMenu::OnLoginCompleteJoin));
		Identity->Login(ControllerId, FOnlineAccountCredentials());
	}
}

void FShooterMainMenu::OnLoginCompleteJoin(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IOnlineSubsystem::Get()->GetIdentityInterface()->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, OnLoginCompleteDelegateHandle);

	OnJoinServer();
}

void FShooterMainMenu::OnJoinSelected()
{
#if SHOOTER_CONSOLE_UI
	if (!ValidatePlayerIsSignedIn(GetPlayerOwner()))
	{
		return;
	}
#endif

	MenuWidget->EnterSubMenu();
}

void FShooterMainMenu::OnJoinServer()
{
#if SHOOTER_CONSOLE_UI
	if ( !ValidatePlayerForOnlinePlay(GetPlayerOwner()) )
	{
		return;
	}
#endif

	StartOnlinePrivilegeTask(IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateSP(this, &FShooterMainMenu::OnUserCanPlayOnlineJoin));
}

void FShooterMainMenu::OnUserCanPlayOnlineJoin(const FUniqueNetId& UserId, EUserPrivileges::Type Privilege, uint32 PrivilegeResults)
{
	CleanupOnlinePrivilegeTask();
	MenuWidget->LockControls(false);

	if (PrivilegeResults == (uint32)IOnlineIdentity::EPrivilegeResults::NoFailures)
	{

		//make sure to switch to custom match type so we don't instead use Quick type
		MatchType = EMatchType::Custom;

		if (GameInstance.IsValid())
		{
			GameInstance->SetOnlineMode(bIsLanMatch ? EOnlineMode::LAN : EOnlineMode::Online);
		}

		MatchType = EMatchType::Custom;
		// Grab the map filter if there is one
		FString SelectedMapFilterName = MapNames[0];
		if( JoinMapOption.IsValid())
		{
			int32 FilterChoice = JoinMapOption->SelectedMultiChoice;
			if( FilterChoice != INDEX_NONE )
			{
				SelectedMapFilterName = JoinMapOption->MultiChoice[FilterChoice].ToString();
			}
		}

#if SHOOTER_CONSOLE_UI
		UGameViewportClient* const GVC = GEngine->GameViewport;
#if PLATFORM_PS4
		// Show server menu (skip splitscreen)
		AddMenuToGameViewport();
		FSlateApplication::Get().SetKeyboardFocus(MenuWidget);

		MenuWidget->NextMenu = JoinServerItem->SubMenu;
		ServerListWidget->BeginServerSearch(bIsLanMatch, bIsDedicatedServer, SelectedMapFilterName);
		ServerListWidget->UpdateServerList();
		MenuWidget->EnterSubMenu();
#else
		// Show splitscreen menu
		RemoveMenuFromGameViewport();	
		GVC->AddViewportWidgetContent(SplitScreenLobbyWidgetContainer.ToSharedRef());

		SplitScreenLobbyWidget->Clear();
		FSlateApplication::Get().SetKeyboardFocus(SplitScreenLobbyWidget);

		SplitScreenLobbyWidget->SetIsJoining( true );
#endif
#else
		MenuWidget->NextMenu = JoinServerItem->SubMenu;
		//FString SelectedMapFilterName = JoinMapOption->MultiChoice[JoinMapOption->SelectedMultiChoice].ToString();

		ServerListWidget->BeginServerSearch(bIsLanMatch, bIsDedicatedServer, SelectedMapFilterName);
		ServerListWidget->UpdateServerList();
		MenuWidget->EnterSubMenu();
#endif
	}
	else if (GameInstance.IsValid())
	{
		GameInstance->DisplayOnlinePrivilegeFailureDialogs(UserId, Privilege, PrivilegeResults);
	}
}

void FShooterMainMenu::OnShowLeaderboard()
{
	MenuWidget->NextMenu = LeaderboardItem->SubMenu;
#if LOGIN_REQUIRED_FOR_ONLINE_PLAY
	LeaderboardWidget->ReadStatsLoginRequired();
#else
	LeaderboardWidget->ReadStats();
#endif
	MenuWidget->EnterSubMenu();
}

void FShooterMainMenu::OnShowDemoBrowser()
{
	MenuWidget->NextMenu = DemoBrowserItem->SubMenu;
	DemoListWidget->BuildDemoList();
	MenuWidget->EnterSubMenu();
}

void FShooterMainMenu::OnUIQuit()
{
	LockAndHideMenu();

	const FShooterMenuSoundsStyle& MenuSounds = FShooterStyle::Get().GetWidgetStyle<FShooterMenuSoundsStyle>("DefaultShooterMenuSoundsStyle");
	UWorld* const World = GameInstance.IsValid() ? GameInstance->GetWorld() : nullptr;
	if (World != NULL && GetPlayerOwnerControllerId() != -1)
	{
		FSlateApplication::Get().PlaySound(MenuSounds.ExitGameSound, GetPlayerOwnerControllerId());
		MenuHelper::PlaySoundAndCall(World, MenuSounds.ExitGameSound, GetPlayerOwnerControllerId(), this, &FShooterMainMenu::Quit);
	}
}

void FShooterMainMenu::Quit()
{
	if (ensure(GameInstance.IsValid()))
	{
		UGameViewportClient* const Viewport = GameInstance->GetGameViewportClient();
		if (ensure(Viewport)) 
		{
			Viewport->ConsoleCommand("quit");
		}
	}
}

void FShooterMainMenu::LockAndHideMenu()
{
	MenuWidget->LockControls(true);
	MenuWidget->HideMenu();
}

void FShooterMainMenu::DisplayLoadingScreen()
{
	IShooterGameLoadingScreenModule* LoadingScreenModule = FModuleManager::LoadModulePtr<IShooterGameLoadingScreenModule>("ShooterGameLoadingScreen");
	if( LoadingScreenModule != NULL )
	{
		LoadingScreenModule->StartInGameLoadingScreen();
	}
}

bool FShooterMainMenu::IsMapReady() const
{
	bool bReady = true;
	IPlatformChunkInstall* ChunkInstaller = FPlatformMisc::GetPlatformChunkInstall();
	if (ChunkInstaller)
	{
		EMap SelectedMap = GetSelectedMap();
		// should use the AssetRegistry as soon as maps are added to the AssetRegistry
		int32 MapChunk = ChunkMapping[(int)SelectedMap];
		EChunkLocation::Type ChunkLocation = ChunkInstaller->GetChunkLocation(MapChunk);
		if (ChunkLocation == EChunkLocation::NotAvailable)
		{			
			bReady = false;
		}
	}
	return bReady;
}

UShooterPersistentUser* FShooterMainMenu::GetPersistentUser() const
{
	UShooterLocalPlayer* const ShooterLocalPlayer = Cast<UShooterLocalPlayer>(GetPlayerOwner());
	return ShooterLocalPlayer ? ShooterLocalPlayer->GetPersistentUser() : nullptr;
}

UWorld* FShooterMainMenu::GetTickableGameObjectWorld() const
{
	ULocalPlayer* LocalPlayerOwner = GetPlayerOwner();
	return (LocalPlayerOwner ? LocalPlayerOwner->GetWorld() : nullptr);
}

ULocalPlayer* FShooterMainMenu::GetPlayerOwner() const
{
	return PlayerOwner.Get();
}

int32 FShooterMainMenu::GetPlayerOwnerControllerId() const
{
	return ( PlayerOwner.IsValid() ) ? PlayerOwner->GetControllerId() : -1;
}

FString FShooterMainMenu::GetMapName() const
{
	 return MapNames[(int)GetSelectedMap()];
}

void FShooterMainMenu::OnCancelMatchmakingComplete(FName SessionName, bool bWasSuccessful)
{
	auto Sessions = Online::GetSessionInterface();
	if(Sessions.IsValid())
	{
		Sessions->ClearOnCancelMatchmakingCompleteDelegate_Handle(OnCancelMatchmakingCompleteDelegateHandle);
	}

	bAnimateQuickmatchSearchingUI = false;
	UGameViewportClient* const GVC = GEngine->GameViewport;
	GVC->RemoveViewportWidgetContent(QuickMatchStoppingWidgetContainer.ToSharedRef());
	AddMenuToGameViewport();
	FSlateApplication::Get().SetKeyboardFocus(MenuWidget);
}

void FShooterMainMenu::OnPlayTogetherEventReceived()
{
	HostOnlineMenuItem->Widget->SetMenuItemActive(true);
	MenuWidget->ConfirmMenuItem();
}

#undef LOCTEXT_NAMESPACE
