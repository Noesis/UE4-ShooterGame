// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once


/**
 * Helps HUD widgets know their context within the game world.
 * e.g. Is this a widget for player 1 or player 2?
 * e.g. In case of multiple PIE sessions, which world do I belong to?
 */
class ShooterHUDPCTrackerBase
{
public:

	virtual ~ShooterHUDPCTrackerBase(){}

	/** Initialize with a world context. */
	void Init( const FLocalPlayerContext& InContext );

	/** Returns a pointer to the player controller */
	TWeakObjectPtr<class AShooterPlayerController> GetPlayerController() const;

	/** Returns a pointer to the World. (Via Player Controller) */
	UWorld* GetWorld() const;

	/** Get the current game GameState */
	class AShooterGameState* GetGameState() const;

	/** @return the game world context */
	const FLocalPlayerContext& GetContext() const;

private:
	/** Which player and world this piece of UI belongs to. This is necessary to support  */
	FLocalPlayerContext Context;

};