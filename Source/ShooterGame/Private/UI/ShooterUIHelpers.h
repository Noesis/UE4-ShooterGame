// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ShooterGame.h"

/** Singleton that contains commonly used UI actions */
class ShooterUIHelpers
{
public:
	/** gets the singleton. */
	static ShooterUIHelpers& Get()
	{
		static ShooterUIHelpers Singleton;
		return Singleton;
	}

	/** fetches the string for displaying the profile */
	FText GetProfileOpenText() const;

	/** profile open ui handler */
	bool ProfileOpenedUI(const FUniqueNetId& Requestor, const FUniqueNetId& Requestee, const FOnProfileUIClosedDelegate* Delegate) const;

	/** fetches the string for swapping the profile */
	FText GetProfileSwapText() const;

	/** profile swap ui handler */
	bool ProfileSwapUI(const int ControllerIndex, bool bShowOnlineOnly, const FOnLoginUIClosedDelegate* Delegate) const;
};