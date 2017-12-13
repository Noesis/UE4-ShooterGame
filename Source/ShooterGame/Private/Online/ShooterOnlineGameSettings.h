// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

/**
 * General session settings for a Shooter game
 */
class FShooterOnlineSessionSettings : public FOnlineSessionSettings
{
public:

	FShooterOnlineSessionSettings(bool bIsLAN = false, bool bIsPresence = false, int32 MaxNumPlayers = 4);
	virtual ~FShooterOnlineSessionSettings() {}
};

/**
 * General search setting for a Shooter game
 */
class FShooterOnlineSearchSettings : public FOnlineSessionSearch
{
public:
	FShooterOnlineSearchSettings(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FShooterOnlineSearchSettings() {}
};

/**
 * Search settings for an empty dedicated server to host a match
 */
class FShooterOnlineSearchSettingsEmptyDedicated : public FShooterOnlineSearchSettings
{
public:
	FShooterOnlineSearchSettingsEmptyDedicated(bool bSearchingLAN = false, bool bSearchingPresence = false);

	virtual ~FShooterOnlineSearchSettingsEmptyDedicated() {}
};
