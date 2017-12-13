// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterTypes.generated.h"
#pragma once

namespace EShooterMatchState
{
	enum Type
	{
		Warmup,
		Playing,
		Won,
		Lost,
	};
}

namespace EShooterCrosshairDirection
{
	enum Type
	{
		Left=0,
		Right=1,
		Top=2,
		Bottom=3,
		Center=4
	};
}

namespace EShooterHudPosition
{
	enum Type
	{
		Left=0,
		FrontLeft=1,
		Front=2,
		FrontRight=3,
		Right=4,
		BackRight=5,
		Back=6,
		BackLeft=7,
	};
}

/** keep in sync with ShooterImpactEffect */
UENUM()
namespace EShooterPhysMaterialType
{
	enum Type
	{
		Unknown,
		Concrete,
		Dirt,
		Water,
		Metal,
		Wood,
		Grass,
		Glass,
		Flesh,
	};
}

namespace EShooterDialogType
{
	enum Type
	{
		None,
		Generic,
		ControllerDisconnected
	};
}

#define SHOOTER_SURFACE_Default		SurfaceType_Default
#define SHOOTER_SURFACE_Concrete	SurfaceType1
#define SHOOTER_SURFACE_Dirt		SurfaceType2
#define SHOOTER_SURFACE_Water		SurfaceType3
#define SHOOTER_SURFACE_Metal		SurfaceType4
#define SHOOTER_SURFACE_Wood		SurfaceType5
#define SHOOTER_SURFACE_Grass		SurfaceType6
#define SHOOTER_SURFACE_Glass		SurfaceType7
#define SHOOTER_SURFACE_Flesh		SurfaceType8

USTRUCT()
struct FDecalData
{
	GENERATED_USTRUCT_BODY()

	/** material */
	UPROPERTY(EditDefaultsOnly, Category=Decal)
	UMaterial* DecalMaterial;

	/** quad size (width & height) */
	UPROPERTY(EditDefaultsOnly, Category=Decal)
	float DecalSize;

	/** lifespan */
	UPROPERTY(EditDefaultsOnly, Category=Decal)
	float LifeSpan;

	/** defaults */
	FDecalData()
		: DecalSize(256.f)
		, LifeSpan(10.f)
	{
	}
};

/** replicated information on a hit we've taken */
USTRUCT()
struct FTakeHitInfo
{
	GENERATED_USTRUCT_BODY()

	/** The amount of damage actually applied */
	UPROPERTY()
	float ActualDamage;

	/** The damage type we were hit with. */
	UPROPERTY()
	UClass* DamageTypeClass;

	/** Who hit us */
	UPROPERTY()
	TWeakObjectPtr<class AShooterCharacter> PawnInstigator;

	/** Who actually caused the damage */
	UPROPERTY()
	TWeakObjectPtr<class AActor> DamageCauser;

	/** Specifies which DamageEvent below describes the damage received. */
	UPROPERTY()
	int32 DamageEventClassID;

	/** Rather this was a kill */
	UPROPERTY()
	uint32 bKilled:1;

private:

	/** A rolling counter used to ensure the struct is dirty and will replicate. */
	UPROPERTY()
	uint8 EnsureReplicationByte;

	/** Describes general damage. */
	UPROPERTY()
	FDamageEvent GeneralDamageEvent;

	/** Describes point damage, if that is what was received. */
	UPROPERTY()
	FPointDamageEvent PointDamageEvent;

	/** Describes radial damage, if that is what was received. */
	UPROPERTY()
	FRadialDamageEvent RadialDamageEvent;

public:
	FTakeHitInfo();

	FDamageEvent& GetDamageEvent();
	void SetDamageEvent(const FDamageEvent& DamageEvent);
	void EnsureReplication();
};