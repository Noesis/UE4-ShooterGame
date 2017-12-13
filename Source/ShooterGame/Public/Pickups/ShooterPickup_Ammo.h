// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterPickup.h"
#include "ShooterPickup_Ammo.generated.h"

class AShooterCharacter;
class AShooterWeapon;

// A pickup object that replenishes ammunition for a weapon
UCLASS(Abstract, Blueprintable)
class AShooterPickup_Ammo : public AShooterPickup
{
	GENERATED_UCLASS_BODY()

	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(AShooterCharacter* TestPawn) const override;

	bool IsForWeapon(UClass* WeaponClass);

protected:

	/** how much ammo does it give? */
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	int32 AmmoClips;

	/** which weapon gets ammo? */
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	TSubclassOf<AShooterWeapon> WeaponType;

	/** give pickup */
	virtual void GivePickupTo(AShooterCharacter* Pawn) override;
};
