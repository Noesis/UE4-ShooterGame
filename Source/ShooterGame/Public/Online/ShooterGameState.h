// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterGameState.generated.h"

/** ranked PlayerState map, created from the GameState */
typedef TMap<int32, TWeakObjectPtr<AShooterPlayerState> > RankedPlayerMap; 

UCLASS()
class AShooterGameState : public AGameState
{
	GENERATED_UCLASS_BODY()

public:

	/** number of teams in current game (doesn't deprecate when no players are left in a team) */
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	int32 NumTeams;

	/** accumulated score per team */
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	TArray<int32> TeamScores;

	/** time left for warmup / match */
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	int32 RemainingTime;

	/** is timer paused? */
	UPROPERTY(Transient, Replicated, BlueprintReadWrite)
	bool bTimerPaused;

	/** gets ranked PlayerState map for specific team */
	void GetRankedMap(int32 TeamIndex, RankedPlayerMap& OutRankedMap) const;	

	void RequestFinishAndExitToMainMenu();
};
