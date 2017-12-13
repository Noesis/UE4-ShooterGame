// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterGameDelegates.h"

#include "ShooterMenuSoundsWidgetStyle.h"
#include "ShooterMenuWidgetStyle.h"
#include "ShooterMenuItemWidgetStyle.h"
#include "ShooterOptionsWidgetStyle.h"
#include "ShooterScoreboardWidgetStyle.h"
#include "ShooterChatWidgetStyle.h"
#include "AssetRegistryModule.h"
#include "IAssetRegistry.h"



#include "UI/Style/ShooterStyle.h"


class FShooterGameModule : public FDefaultGameModuleImpl
{
	virtual void StartupModule() override
	{
		InitializeShooterGameDelegates();
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		//Hot reload hack
		FSlateStyleRegistry::UnRegisterSlateStyle(FShooterStyle::GetStyleSetName());
		FShooterStyle::Initialize();
	}

	virtual void ShutdownModule() override
	{
		FShooterStyle::Shutdown();
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FShooterGameModule, ShooterGame, "ShooterGame");

DEFINE_LOG_CATEGORY(LogShooter)
DEFINE_LOG_CATEGORY(LogShooterWeapon)
