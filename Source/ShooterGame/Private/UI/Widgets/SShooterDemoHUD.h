// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

class APlayerController;

/** Shows the replay timeline bar, current time and total time of the replay, current playback speed, and a pause toggle button. */
class SShooterDemoHUD : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterDemoHUD)
		: _PlayerOwner(nullptr)
		{}

	SLATE_ARGUMENT(TWeakObjectPtr<APlayerController>, PlayerOwner)

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override { return true; }

private:

	TWeakObjectPtr<APlayerController> PlayerOwner;

	FText GetCurrentReplayTime() const;
	FText GetTotalReplayTime() const;
	FText GetPlaybackSpeed() const;

	ECheckBoxState IsPauseChecked() const;
	void OnPauseCheckStateChanged(ECheckBoxState CheckState) const;
};


