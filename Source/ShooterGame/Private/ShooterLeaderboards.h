// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "ShooterTypes.h"
#include "OnlineLeaderboardInterface.h"

// these are normally exported from platform-specific tools
#define LEADERBOARD_STAT_SCORE				"Score"
#define LEADERBOARD_STAT_KILLS				"Frags"
#define LEADERBOARD_STAT_DEATHS				"Deaths"
#define LEADERBOARD_STAT_MATCHESPLAYED		"MatchesPlayed"

/**
 *	'AllTime' leaderboard read object
 */
class FShooterAllTimeMatchResultsRead : public FOnlineLeaderboardRead
{
public:

	FShooterAllTimeMatchResultsRead()
	{
		// Default properties
		LeaderboardName = FName(TEXT("ShooterAllTimeMatchResults"));
		SortedColumn = LEADERBOARD_STAT_SCORE;

		// Define default columns
		new (ColumnMetadata) FColumnMetaData(LEADERBOARD_STAT_SCORE, EOnlineKeyValuePairDataType::Int32);
		new (ColumnMetadata) FColumnMetaData(LEADERBOARD_STAT_KILLS, EOnlineKeyValuePairDataType::Int32);
		new (ColumnMetadata) FColumnMetaData(LEADERBOARD_STAT_DEATHS, EOnlineKeyValuePairDataType::Int32);
		new (ColumnMetadata) FColumnMetaData(LEADERBOARD_STAT_MATCHESPLAYED, EOnlineKeyValuePairDataType::Int32);
	}
};

/**
 *	'AllTime' leaderboard write object
 */
class FShooterAllTimeMatchResultsWrite : public FOnlineLeaderboardWrite
{
public:

	FShooterAllTimeMatchResultsWrite()
	{
		// Default properties
		new (LeaderboardNames) FName(TEXT("ShooterAllTimeMatchResults"));
		RatedStat = LEADERBOARD_STAT_SCORE;
		DisplayFormat = ELeaderboardFormat::Number;
		SortMethod = ELeaderboardSort::Descending;
		UpdateMethod = ELeaderboardUpdateMethod::KeepBest;
	}
};

