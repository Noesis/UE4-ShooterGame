// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "SShooterScoreboardWidget.h"
#include "ShooterStyle.h"
#include "ShooterScoreboardWidgetStyle.h"
#include "ShooterUIHelpers.h"
#include "Online/ShooterPlayerState.h"

#define LOCTEXT_NAMESPACE "ShooterScoreboard"

// @todo: prevent interaction on PC for now (see OnFocusReceived for reasons)
#if PLATFORM_XBOXONE
#define INTERACTIVE_SCOREBOARD	1
#else
#define INTERACTIVE_SCOREBOARD	0
#endif

#define	NORM_PADDING	(FMargin(5))

void SShooterScoreboardWidget::Construct(const FArguments& InArgs)
{
	ScoreboardStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterScoreboardStyle>("DefaultShooterScoreboardStyle");

	PCOwner = InArgs._PCOwner;
	ScoreboardTint = FLinearColor(0.0f,0.0f,0.0f,0.4f);
	ScoreBoxWidth = 140.0f;
	ScoreCountUpTime = 2.0f;

	ScoreboardStartTime = FPlatformTime::Seconds();
	MatchState = InArgs._MatchState.Get();

	UpdatePlayerStateMaps();
	
	Columns.Add(FColumnData(LOCTEXT("KillsColumn", "Kills"),
		ScoreboardStyle->KillStatColor,
		FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Kills)));

	Columns.Add(FColumnData(LOCTEXT("DeathsColumn", "Deaths"),
		ScoreboardStyle->DeathStatColor,
		FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Deaths)));

	Columns.Add(FColumnData(LOCTEXT("ScoreColumn", "Score"),
		ScoreboardStyle->ScoreStatColor,
		FOnGetPlayerStateAttribute::CreateSP(this, &SShooterScoreboardWidget::GetAttributeValue_Score)));

	TSharedPtr<SHorizontalBox> HeaderCols;

	const TSharedRef<SVerticalBox> ScoreboardGrid = SNew(SVerticalBox)
	// HEADER ROW			
	+SVerticalBox::Slot() .AutoHeight()
	[
		//Padding in the header row (invisible) for speaker icon
		SAssignNew(HeaderCols, SHorizontalBox)
		+ SHorizontalBox::Slot().Padding(NORM_PADDING+FMargin(2,0,0,0)).AutoWidth()
		[
			SNew(SImage)
			.Image(FShooterStyle::Get().GetBrush("ShooterGame.Speaker"))
			.Visibility(EVisibility::Hidden)
		]

		//Player Name autosized column
		+SHorizontalBox::Slot() .Padding(NORM_PADDING)
		[
			SNew(SBorder)
			.Padding(NORM_PADDING)
			.VAlign(VAlign_Center)	
			.HAlign(HAlign_Center)
			.BorderImage(&ScoreboardStyle->ItemBorderBrush)
			.BorderBackgroundColor(ScoreboardTint)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PlayerNameColumn", "Player Name"))
				.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
			]
		]
	];

	for (uint8 ColIdx = 0; ColIdx < Columns.Num(); ColIdx++ )
	{
		//Header constant sized columns
		HeaderCols->AddSlot() .VAlign(VAlign_Center) .HAlign(HAlign_Center) .AutoWidth() .Padding(NORM_PADDING)
		[
			SNew(SBorder)
			.Padding(NORM_PADDING)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(&ScoreboardStyle->ItemBorderBrush)
			.BorderBackgroundColor(ScoreboardTint)
			[
				SNew(SBox)
				.WidthOverride(ScoreBoxWidth)
				.HAlign(HAlign_Center)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot() .AutoWidth() .VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(Columns[ColIdx].Name)
						.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
						.ColorAndOpacity(Columns[ColIdx].Color)
					]
				]
			]
		];
	}

	ScoreboardGrid->AddSlot() .AutoHeight()
	[
		SAssignNew(ScoreboardData, SVerticalBox)
	];
	UpdateScoreboardGrid();

	SBorder::Construct(
		SBorder::FArguments()
		.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		.BorderBackgroundColor(ScoreboardTint)
		[
			ScoreboardGrid
		]			
	);
}

void SShooterScoreboardWidget::StoreTalkingPlayerData(const FUniqueNetId& PlayerId, bool bIsTalking)
{
	bool bIsFound = false;
	int32 FoundIndex = -1;
	for (int32 i = 0; i < PlayersTalkingThisFrame.Num(); ++i)
	{
		if (PlayersTalkingThisFrame[i].Key.Get() == PlayerId)
		{
			FoundIndex = i;
			bIsFound = true;
		}
	}
	if (bIsFound && FoundIndex >= 0)
	{
		if (!bIsTalking)
		{
			PlayersTalkingThisFrame[FoundIndex].Value = bIsTalking;
		}
		PlayersTalkingThisFrame[FoundIndex].Value = bIsTalking;
	}
	else
	{
		PlayersTalkingThisFrame.Emplace(PlayerId.AsShared(), bIsTalking);
	}
}

FText SShooterScoreboardWidget::GetMatchRestartText() const
{
	if (PCOwner.IsValid() && (PCOwner->GetWorld() != NULL ))
	{
		AShooterGameState* const GameState = PCOwner->GetWorld()->GetGameState<AShooterGameState>();
		if (GameState)
		{
			if (GameState->RemainingTime > 0)
			{
				return FText::Format(LOCTEXT("MatchRestartTimeString", "New match begins in: {0}"), FText::AsNumber(GameState->RemainingTime));
			}
			else
			{
				return LOCTEXT("MatchRestartingString", "Starting new match...");
			}
		}
	}

	return FText::GetEmpty();
}

FText SShooterScoreboardWidget::GetMatchOutcomeText() const
{
	FText OutcomeText = FText::GetEmpty();

	if (MatchState == EShooterMatchState::Won)
	{
		OutcomeText = LOCTEXT("Winner", "YOU ARE THE WINNER!");
	} 
	else if (MatchState == EShooterMatchState::Lost)
	{
		OutcomeText = LOCTEXT("Loser", "Match has finished");
	}

	return OutcomeText;
}

void SShooterScoreboardWidget::UpdateScoreboardGrid()
{
	ScoreboardData->ClearChildren();
	for (uint8 TeamNum = 0; TeamNum < PlayerStateMaps.Num(); TeamNum++)
	{
		//Player rows from each team
		ScoreboardData->AddSlot() .AutoHeight()
			[
				MakePlayerRows(TeamNum)
			];
		//If we have more than one team, we are playing team based game mode, add totals
		if (PlayerStateMaps.Num() > 1 && PlayerStateMaps[TeamNum].Num() > 0)
		{
			// Horizontal Ruler
			ScoreboardData->AddSlot() .AutoHeight() .Padding(NORM_PADDING)
				[
					SNew(SBorder)
					.Padding(1)
					.BorderImage(&ScoreboardStyle->ItemBorderBrush)
				];
			ScoreboardData->AddSlot() .AutoHeight()
				[
					MakeTotalsRow(TeamNum)
				];
		}
	}

	if (MatchState > EShooterMatchState::Playing)
	{
		ScoreboardData->AddSlot() .AutoHeight() .Padding(NORM_PADDING)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot() .HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SShooterScoreboardWidget::GetMatchOutcomeText)
						.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
					]
				]
			];

		ScoreboardData->AddSlot() .AutoHeight() .Padding(NORM_PADDING)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot() .HAlign(HAlign_Fill)
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SShooterScoreboardWidget::GetMatchRestartText)
						.TextStyle(FShooterStyle::Get(), "ShooterGame.MenuHeaderTextStyle")
					]
				]
			];

		ScoreboardData->AddSlot() .AutoHeight() .Padding(NORM_PADDING)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot() .HAlign(HAlign_Right)
				[
					SNew(SBox)
					.HAlign(HAlign_Right)
					[
						SNew(STextBlock)
						.Text(ShooterUIHelpers::Get().GetProfileOpenText())
						.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.StatTextStyle")
						.Visibility(this, &SShooterScoreboardWidget::GetProfileUIVisibility)
					]
				]
			];
	}
}

void SShooterScoreboardWidget::UpdatePlayerStateMaps()
{
	if (PCOwner.IsValid())
	{
		AShooterGameState* const GameState = PCOwner->GetWorld()->GetGameState<AShooterGameState>();
		if (GameState)
		{
			bool bRequiresWidgetUpdate = false;
			const int32 NumTeams = FMath::Max(GameState->NumTeams, 1);
			LastTeamPlayerCount.Reset();
			LastTeamPlayerCount.AddZeroed(PlayerStateMaps.Num());
			for (int32 i = 0; i < PlayerStateMaps.Num(); i++)
			{
				LastTeamPlayerCount[i] = PlayerStateMaps[i].Num();
			}

			PlayerStateMaps.Reset();
			PlayerStateMaps.AddZeroed(NumTeams);
		
			for (int32 i = 0; i < NumTeams; i++)
			{
				GameState->GetRankedMap(i, PlayerStateMaps[i]);

				if (LastTeamPlayerCount.Num() > 0 && PlayerStateMaps[i].Num() != LastTeamPlayerCount[i])
				{
					bRequiresWidgetUpdate = true;
				}
			}
			if (bRequiresWidgetUpdate)
			{
				UpdateScoreboardGrid();
			}
		}
	}

	UpdateSelectedPlayer();
}

void SShooterScoreboardWidget::Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime )
{
	UpdatePlayerStateMaps();
}

bool SShooterScoreboardWidget::SupportsKeyboardFocus() const
{
#if INTERACTIVE_SCOREBOARD
	if (MatchState > EShooterMatchState::Playing)
	{
		return true;
	}
#endif
	return false;
}

FReply SShooterScoreboardWidget::OnFocusReceived( const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent )
{
	// @todo: may not want to affect all controllers if split screen

	// @todo: not-pc: need to support mouse focus too (alt+tabbing, windowed, etc)
	// @todo: not-pc: after each round, the mouse is released but as soon as you click it's recaptured and input stops working
	return FReply::Handled().ReleaseMouseCapture().SetUserFocus(SharedThis(this), EFocusCause::SetDirectly, true);
}

FReply SShooterScoreboardWidget::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) 
{
	FReply Result = FReply::Unhandled();
	const FKey Key = InKeyEvent.GetKey();
	if (MatchState > EShooterMatchState::Playing)
	{
		if (Key == EKeys::Up || Key == EKeys::Gamepad_DPad_Up || Key == EKeys::Gamepad_LeftStick_Up)
		{
			OnSelectedPlayerPrev();
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Down || Key == EKeys::Gamepad_DPad_Down || Key == EKeys::Gamepad_LeftStick_Down)
		{
			OnSelectedPlayerNext();
			Result = FReply::Handled();
		}
		else if (Key == EKeys::Enter || Key == EKeys::Virtual_Accept)
		{
			ProfileUIOpened();
			Result = FReply::Handled();
		}
		else if ((Key == EKeys::Escape || Key == EKeys::Gamepad_Special_Right || Key == EKeys::Global_Play || Key == EKeys::Global_Menu) && !InKeyEvent.IsRepeat())
		{
			// Allow the user to pause from the scoreboard still
			if (AShooterPlayerController* const PC = Cast<AShooterPlayerController>(PCOwner.Get()))
			{
				PC->OnToggleInGameMenu();
			}
			Result = FReply::Handled();
		}
	}
	return Result;
}

void SShooterScoreboardWidget::PlaySound(const FSlateSound& SoundToPlay) const
{
	if( PCOwner.IsValid() )
	{
		if( ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(PCOwner->Player) )
		{
			FSlateApplication::Get().PlaySound(SoundToPlay, LocalPlayer->GetControllerId());	
		}
	}
}

FReply SShooterScoreboardWidget::OnMouseOverPlayer(const FGeometry& Geometry, const FPointerEvent& Event, const FTeamPlayer TeamPlayer)
{
#if INTERACTIVE_SCOREBOARD
	if( !(SelectedPlayer == TeamPlayer) )
	{
		SelectedPlayer = TeamPlayer;
		PlaySound(ScoreboardStyle->PlayerChangeSound);
	}
#endif
	return FReply::Handled();
}

void SShooterScoreboardWidget::OnSelectedPlayerPrev()
{
	// Make sure we have a valid index to start with
	if( !SelectedPlayer.IsValid() && !SetSelectedPlayerUs())
	{
		return;
	}

	// We don't want to decrease the player Id, we want to find the current one in the list and then select the one before it
	int32 PrevPlayerId = -1;
	for (RankedPlayerMap::TConstIterator PlayerIt(PlayerStateMaps[SelectedPlayer.TeamNum]); PlayerIt; ++PlayerIt)	// This would be easier with a .ReverseIterator call
	{
		const int32 PlayerId = PlayerIt.Key();
		if( PlayerId == SelectedPlayer.PlayerId )
		{
			break;
		}
		PrevPlayerId = PlayerId;
	}

	if( PrevPlayerId == -1 )
	{
		// If the id is still invalid, that means the current selection was first in their team, try the previous team...
		SelectedPlayer.TeamNum--;
		if (!PlayerStateMaps.IsValidIndex(SelectedPlayer.TeamNum))
		{
			// If there isn't a previous team, move to the last team
			SelectedPlayer.TeamNum = PlayerStateMaps.Num() - 1;
			check(PlayerStateMaps.IsValidIndex(SelectedPlayer.TeamNum));
		}

		// We want the last player in the team
		for (RankedPlayerMap::TConstIterator PlayerIt(PlayerStateMaps[SelectedPlayer.TeamNum]); PlayerIt; ++PlayerIt)	// This would be easier with a .Last call
		{
			const int32 PlayerId = PlayerIt.Key();
			PrevPlayerId = PlayerId;
		}
	}

	check( SelectedPlayer.PlayerId != -1 );
	SelectedPlayer.PlayerId = PrevPlayerId;
	PlaySound(ScoreboardStyle->PlayerChangeSound);
}

void SShooterScoreboardWidget::OnSelectedPlayerNext()
{
	// Make sure we have a valid index to start with
	if( !SelectedPlayer.IsValid() && !SetSelectedPlayerUs())
	{
		return;
	}

	// We don't want to increase the player Id, we want to find the current one in the list and then select the one after
	bool bNext = false;
	for (RankedPlayerMap::TConstIterator PlayerIt(PlayerStateMaps[SelectedPlayer.TeamNum]); PlayerIt; ++PlayerIt)
	{
		const int32 PlayerId = PlayerIt.Key();
		if( PlayerId == SelectedPlayer.PlayerId )
		{
			bNext = true;
		}
		else if( bNext == true )
		{
			SelectedPlayer.PlayerId = PlayerId;
			PlaySound(ScoreboardStyle->PlayerChangeSound);

			bNext = false;
			break;
		}
	}

	if( bNext == true )
	{
		// If next is still true, our current selection was last in their team, try the next team...
		SelectedPlayer.TeamNum++;
		if (!PlayerStateMaps.IsValidIndex(SelectedPlayer.TeamNum))
		{
			// If there isn't a next team, move to the first team
			SelectedPlayer.TeamNum = 0;
			check(PlayerStateMaps.IsValidIndex(SelectedPlayer.TeamNum));
		}

		SelectedPlayer.PlayerId = 0;
		PlaySound(ScoreboardStyle->PlayerChangeSound);
	}
}

void SShooterScoreboardWidget::ResetSelectedPlayer()
{
	SelectedPlayer = FTeamPlayer();
}

void SShooterScoreboardWidget::UpdateSelectedPlayer()
{
	// Make sure the selected player is still valid...
	if( SelectedPlayer.IsValid() )
	{
		const AShooterPlayerState* PlayerState = GetSortedPlayerState(SelectedPlayer);
		if( !PlayerState )
		{
			// Player is no longer valid, reset (note: reset implies 'us' in IsSelectedPlayer and IsPlayerSelectedAndValid)
			ResetSelectedPlayer();
		}
	}
}

bool SShooterScoreboardWidget::SetSelectedPlayerUs()
{
	ResetSelectedPlayer();

	// Set the owner player to be the default focused one
	if( APlayerController* const PC = PCOwner.Get() )
	{
		for (uint8 TeamNum = 0; TeamNum < PlayerStateMaps.Num(); TeamNum++)
		{
			for (RankedPlayerMap::TConstIterator PlayerIt(PlayerStateMaps[TeamNum]); PlayerIt; ++PlayerIt)
			{
				const TWeakObjectPtr<AShooterPlayerState> PlayerState = PlayerIt.Value();
				if( PlayerState.IsValid() && PC->PlayerState && PC->PlayerState == PlayerState.Get() )
				{
					SelectedPlayer = FTeamPlayer(TeamNum, PlayerIt.Key());
					return true;
				}
			}
		}
	}
	return false;
}

bool SShooterScoreboardWidget::IsSelectedPlayer(const FTeamPlayer& TeamPlayer) const
{
	if( !SelectedPlayer.IsValid() )
	{
		// If not explicitly set, test to see if the owner player was passed.
		if( IsOwnerPlayer(TeamPlayer) )
		{
			return true;
		}
	}
	else if( SelectedPlayer == TeamPlayer )
	{
		return true;
	}
	return false;
}

bool SShooterScoreboardWidget::IsPlayerSelectedAndValid() const
{
#if INTERACTIVE_SCOREBOARD
	if( !SelectedPlayer.IsValid() )
	{
		// Nothing is selected, default to the player
		if( PCOwner.IsValid() && PCOwner->PlayerState )
		{
			const TSharedPtr<const FUniqueNetId> OwnerNetId = PCOwner->PlayerState->UniqueId.GetUniqueNetId();
			return OwnerNetId.IsValid();
		}
	}
	else if( const AShooterPlayerState* PlayerState = GetSortedPlayerState(SelectedPlayer) ) 
	{
		const TSharedPtr<const FUniqueNetId>& PlayerId = PlayerState->UniqueId.GetUniqueNetId();
		return PlayerId.IsValid();
	}
#endif
	return false;
}

EVisibility SShooterScoreboardWidget::GetProfileUIVisibility() const
{
	return IsPlayerSelectedAndValid() ? EVisibility::Visible : EVisibility::Hidden;
}

bool SShooterScoreboardWidget::ProfileUIOpened() const
{
	if( IsPlayerSelectedAndValid() )
	{
		check( PCOwner.IsValid() && PCOwner->PlayerState );
		const TSharedPtr<const FUniqueNetId>& OwnerNetId = PCOwner->PlayerState->UniqueId.GetUniqueNetId();
		check( OwnerNetId.IsValid() );

		const TSharedPtr<const FUniqueNetId>& PlayerId = ( !SelectedPlayer.IsValid() ? OwnerNetId : GetSortedPlayerState(SelectedPlayer)->UniqueId.GetUniqueNetId() );
		check( PlayerId.IsValid() );
		return ShooterUIHelpers::Get().ProfileOpenedUI(*OwnerNetId.Get(), *PlayerId.Get(), NULL);
	}
	return false;
}

EVisibility SShooterScoreboardWidget::PlayerPresenceToItemVisibility(const FTeamPlayer TeamPlayer) const
{
	return GetSortedPlayerState(TeamPlayer) ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility SShooterScoreboardWidget::SpeakerIconVisibility(const FTeamPlayer TeamPlayer) const
{
	AShooterPlayerState* PlayerState = GetSortedPlayerState(TeamPlayer);
	if (PlayerState)
	{
		for (int32 i = 0; i < PlayersTalkingThisFrame.Num(); ++i)
		{
			if (PlayerState->UniqueId == PlayersTalkingThisFrame[i].Key && PlayersTalkingThisFrame[i].Value)
			{
				return EVisibility::Visible;
			}
		}
	}
	return EVisibility::Hidden;
}

FSlateColor SShooterScoreboardWidget::GetScoreboardBorderColor(const FTeamPlayer TeamPlayer) const
{
	const bool bIsSelected = IsSelectedPlayer(TeamPlayer);
	const int32 RedTeam = 0;
	const float BaseValue = bIsSelected == true ? 0.15f : 0.0f;
	const float AlphaValue = bIsSelected == true ? 1.0f : 0.3f;
	float RedValue = TeamPlayer.TeamNum == RedTeam ? 0.25f : 0.0f;
	float BlueValue = TeamPlayer.TeamNum != RedTeam ? 0.25f : 0.0f;
	return FLinearColor(BaseValue + RedValue, BaseValue, BaseValue + BlueValue, AlphaValue);
}

FText SShooterScoreboardWidget::GetPlayerName(const FTeamPlayer TeamPlayer) const
{
	const AShooterPlayerState* PlayerState = GetSortedPlayerState(TeamPlayer);
	if (PlayerState)
	{
		return FText::FromString(PlayerState->GetShortPlayerName());
	}

	return FText::GetEmpty();
}

bool SShooterScoreboardWidget::ShouldPlayerBeDisplayed(const FTeamPlayer TeamPlayer) const
{
	const AShooterPlayerState* PlayerState = GetSortedPlayerState(TeamPlayer);
	
	return PlayerState != nullptr && !PlayerState->bOnlySpectator;
}

FSlateColor SShooterScoreboardWidget::GetPlayerColor(const FTeamPlayer TeamPlayer) const
{
	// If this is the owner players row, tint the text color to show ourselves more clearly
	if( IsOwnerPlayer(TeamPlayer) )
	{
		return FSlateColor(FLinearColor::Yellow);
	}

	const FTextBlockStyle& TextStyle = FShooterStyle::Get().GetWidgetStyle<FTextBlockStyle>("ShooterGame.DefaultScoreboard.Row.StatTextStyle");
	return TextStyle.ColorAndOpacity;
}

FSlateColor SShooterScoreboardWidget::GetColumnColor(const FTeamPlayer TeamPlayer, uint8 ColIdx) const
{
	// If this is the owner players row, tint the text color to show ourselves more clearly
	if( IsOwnerPlayer(TeamPlayer) )
	{
		return FSlateColor(FLinearColor::Yellow);
	}

	check(Columns.IsValidIndex(ColIdx));
	return Columns[ColIdx].Color;
}

bool SShooterScoreboardWidget::IsOwnerPlayer(const FTeamPlayer& TeamPlayer) const
{
	return ( PCOwner.IsValid() && PCOwner->PlayerState && PCOwner->PlayerState == GetSortedPlayerState(TeamPlayer) );
}

FText SShooterScoreboardWidget::GetStat(FOnGetPlayerStateAttribute Getter, const FTeamPlayer TeamPlayer) const
{
	int32 StatTotal = 0;
	if (TeamPlayer.PlayerId != SpecialPlayerIndex::All)
	{
		AShooterPlayerState* PlayerState = GetSortedPlayerState(TeamPlayer);
		if (PlayerState)
		{
			StatTotal = Getter.Execute(PlayerState);
		}
	} 
	else
	{
		for (RankedPlayerMap::TConstIterator PlayerIt(PlayerStateMaps[TeamPlayer.TeamNum]); PlayerIt; ++PlayerIt)
		{
			AShooterPlayerState* PlayerState = PlayerIt.Value().Get();
			if (PlayerState)
			{
				StatTotal += Getter.Execute(PlayerState);
			}
		}
	}

	return FText::AsNumber(LerpForCountup(StatTotal));
}

int32 SShooterScoreboardWidget::LerpForCountup(int32 ScoreValue) const
{
	if (MatchState > EShooterMatchState::Playing)
	{
		const float LerpAmount = FMath::Min<float>(1.0f, (FPlatformTime::Seconds() - ScoreboardStartTime) / ScoreCountUpTime);
		return FMath::Lerp(0, ScoreValue, LerpAmount);
	}
	else
	{
		return ScoreValue;
	}
}

TSharedRef<SWidget> SShooterScoreboardWidget::MakeTotalsRow(uint8 TeamNum) const
{
	TSharedPtr<SHorizontalBox> TotalsRow;

	SAssignNew(TotalsRow, SHorizontalBox)
	+SHorizontalBox::Slot() .Padding(NORM_PADDING)
	[
		SNew(SBorder)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.Padding(NORM_PADDING)
		.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		.BorderBackgroundColor(ScoreboardTint)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("ScoreTotal", "Team Score"))
			.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
		]
	];

	//Leave two columns empty
	for (uint8 i = 0; i < 2; i++)
	{
		TotalsRow->AddSlot() .Padding(NORM_PADDING) .AutoWidth() .HAlign(HAlign_Center) .VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(NORM_PADDING)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.BorderImage(FStyleDefaults::GetNoBrush())
			.BorderBackgroundColor(ScoreboardTint)
			[
				SNew(SBox)
				.WidthOverride(ScoreBoxWidth)
				.HAlign(HAlign_Center)
			]
		];
	}
	//Total team score / captures in CTF mode
	TotalsRow->AddSlot() .Padding(NORM_PADDING) .AutoWidth() .HAlign(HAlign_Center) .VAlign(VAlign_Center)
	[
		SNew(SBorder)
		.Padding(NORM_PADDING)
		.VAlign(VAlign_Center)
		.HAlign(HAlign_Center)
		.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		.BorderBackgroundColor(ScoreboardTint)
		[
			SNew(SBox)
			.WidthOverride(ScoreBoxWidth)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SShooterScoreboardWidget::GetStat, Columns.Last().AttributeGetter, FTeamPlayer(TeamNum, SpecialPlayerIndex::All))
				.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.HeaderTextStyle")
			]
		]
	];

	return TotalsRow.ToSharedRef();
}

TSharedRef<SWidget> SShooterScoreboardWidget::MakePlayerRows(uint8 TeamNum) const
{
	TSharedRef<SVerticalBox> PlayerRows = SNew(SVerticalBox);

	for (int32 PlayerIndex=0; PlayerIndex < PlayerStateMaps[TeamNum].Num(); PlayerIndex++ )
	{
		FTeamPlayer TeamPlayer(TeamNum, PlayerIndex);

		if (ShouldPlayerBeDisplayed(TeamPlayer))
		{
			PlayerRows->AddSlot().AutoHeight()
				[
					MakePlayerRow(TeamPlayer)
				];
		}
	}

	return PlayerRows;
}

TSharedRef<SWidget> SShooterScoreboardWidget::MakePlayerRow(const FTeamPlayer& TeamPlayer) const
{
	// Make the padding here slightly smaller than NORM_PADDING, to fit in more players
	const FMargin Pad = FMargin(5,1);

	TSharedPtr<SHorizontalBox> PlayerRow;
	//Speaker Icon display
	SAssignNew(PlayerRow, SHorizontalBox)
	+ SHorizontalBox::Slot().Padding(Pad+FMargin(2,0,0,0)).AutoWidth()
	[
		SNew(SImage)
		.Image(FShooterStyle::Get().GetBrush("ShooterGame.Speaker"))
		.Visibility(this, &SShooterScoreboardWidget::SpeakerIconVisibility, TeamPlayer)
	];

	//first autosized row with player name
	PlayerRow->AddSlot() .Padding(Pad)
	[
		SNew(SBorder)
		.Padding(Pad)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Visibility(this, &SShooterScoreboardWidget::PlayerPresenceToItemVisibility, TeamPlayer)
		.OnMouseMove(this, &SShooterScoreboardWidget::OnMouseOverPlayer, TeamPlayer)
		.BorderBackgroundColor(this, &SShooterScoreboardWidget::GetScoreboardBorderColor, TeamPlayer)
		.BorderImage(&ScoreboardStyle->ItemBorderBrush)
		[
			SNew(STextBlock)
			.Text(this, &SShooterScoreboardWidget::GetPlayerName, TeamPlayer)
			.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.StatTextStyle")
			.ColorAndOpacity(this, &SShooterScoreboardWidget::GetPlayerColor, TeamPlayer)
		]
	];
	//attributes rows (kills, deaths, score/captures)
	for (uint8 ColIdx = 0; ColIdx < Columns.Num(); ColIdx++)
	{
		PlayerRow->AddSlot()
		.Padding(Pad) .AutoWidth() .HAlign(HAlign_Center) .VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(Pad)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.Visibility(this, &SShooterScoreboardWidget::PlayerPresenceToItemVisibility, TeamPlayer)
			.OnMouseMove(this, &SShooterScoreboardWidget::OnMouseOverPlayer, TeamPlayer)
			.BorderBackgroundColor(this, &SShooterScoreboardWidget::GetScoreboardBorderColor, TeamPlayer)
			.BorderImage(&ScoreboardStyle->ItemBorderBrush)
			[
				SNew(SBox)
				.WidthOverride(ScoreBoxWidth)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SShooterScoreboardWidget::GetStat, Columns[ColIdx].AttributeGetter, TeamPlayer)
					.TextStyle(FShooterStyle::Get(), "ShooterGame.DefaultScoreboard.Row.StatTextStyle")
					.ColorAndOpacity(this, &SShooterScoreboardWidget::GetColumnColor, TeamPlayer, ColIdx)
				]
			]
		];
	}
	return PlayerRow.ToSharedRef();
}

AShooterPlayerState* SShooterScoreboardWidget::GetSortedPlayerState(const FTeamPlayer& TeamPlayer) const
{
	if (PlayerStateMaps.IsValidIndex(TeamPlayer.TeamNum) && PlayerStateMaps[TeamPlayer.TeamNum].Contains(TeamPlayer.PlayerId))
	{
		return PlayerStateMaps[TeamPlayer.TeamNum].FindRef(TeamPlayer.PlayerId).Get();
	}
	
	return NULL;
}

int32 SShooterScoreboardWidget::GetAttributeValue_Kills(AShooterPlayerState* PlayerState) const
{
	return PlayerState->GetKills();
}

int32 SShooterScoreboardWidget::GetAttributeValue_Deaths(AShooterPlayerState* PlayerState) const
{
	return PlayerState->GetDeaths();
}

int32 SShooterScoreboardWidget::GetAttributeValue_Score(AShooterPlayerState* PlayerState) const
{
	return FMath::TruncToInt(PlayerState->Score);
}

#undef LOCTEXT_NAMESPACE
