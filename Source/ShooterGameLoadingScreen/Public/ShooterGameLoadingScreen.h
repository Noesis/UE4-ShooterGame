// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#ifndef __SHOOTERGAMELOADINGSCREEN_H__
#define __SHOOTERGAMELOADINGSCREEN_H__

#include "ModuleInterface.h"


/** Module interface for this game's loading screens */
class IShooterGameLoadingScreenModule : public IModuleInterface
{
public:
	/** Kicks off the loading screen for in game loading (not startup) */
	virtual void StartInGameLoadingScreen() = 0;
};

#endif // __SHOOTERGAMELOADINGSCREEN_H__
