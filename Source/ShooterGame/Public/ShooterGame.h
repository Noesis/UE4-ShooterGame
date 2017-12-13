// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine.h"
#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ParticleDefinitions.h"
#include "SoundDefinitions.h"
#include "Net/UnrealNetwork.h"
#include "ShooterGameMode.h"
#include "ShooterGameState.h"
#include "ShooterCharacter.h"
#include "ShooterCharacterMovement.h"
#include "ShooterPlayerController.h"
#include "ShooterGameClasses.h"


class UBehaviorTreeComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogShooter, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogShooterWeapon, Log, All);

/** when you modify this, please note that this information can be saved with instances
 * also DefaultEngine.ini [/Script/Engine.CollisionProfile] should match with this list **/
#define COLLISION_WEAPON		ECC_GameTraceChannel1
#define COLLISION_PROJECTILE	ECC_GameTraceChannel2
#define COLLISION_PICKUP		ECC_GameTraceChannel3

#define MAX_PLAYER_NAME_LENGTH 16


/** Set to 1 to pretend we're building for console even on a PC, for testing purposes */
#define SHOOTER_SIMULATE_CONSOLE_UI	0

#if PLATFORM_PS4 || PLATFORM_XBOXONE || PLATFORM_SWITCH || SHOOTER_SIMULATE_CONSOLE_UI
	#define SHOOTER_CONSOLE_UI 1
#else
	#define SHOOTER_CONSOLE_UI 0
#endif

