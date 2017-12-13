// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.


#include "ShooterGame.h"
#include "Player/ShooterSpectatorPawn.h"

AShooterSpectatorPawn::AShooterSpectatorPawn(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

void AShooterSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ADefaultPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ADefaultPawn::MoveRight);
	PlayerInputComponent->BindAxis("MoveUp", this, &ADefaultPawn::MoveUp_World);
	PlayerInputComponent->BindAxis("Turn", this, &ADefaultPawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ADefaultPawn::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ADefaultPawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterSpectatorPawn::LookUpAtRate);
}

void AShooterSpectatorPawn::LookUpAtRate(float Val)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Val * BaseLookUpRate * GetWorld()->GetDeltaSeconds() * CustomTimeDilation);
}
