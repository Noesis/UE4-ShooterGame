// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterPickup.h"
#include "ShooterPickup_Health.generated.h"

class AShooterCharacter;

// A pickup object that replenishes character health
UCLASS(Abstract, Blueprintable)
class AShooterPickup_Health : public AShooterPickup
{
	GENERATED_UCLASS_BODY()

	/** check if pawn can use this pickup */
	virtual bool CanBePickedUp(AShooterCharacter* TestPawn) const override;

protected:

	/** how much health does it give? */
	UPROPERTY(EditDefaultsOnly, Category=Pickup)
	int32 Health;

	/** give pickup */
	virtual void GivePickupTo(AShooterCharacter* Pawn) override;
};
