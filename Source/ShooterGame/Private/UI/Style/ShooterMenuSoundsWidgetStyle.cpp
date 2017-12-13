// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterMenuSoundsWidgetStyle.h"

FShooterMenuSoundsStyle::FShooterMenuSoundsStyle()
{
}

FShooterMenuSoundsStyle::~FShooterMenuSoundsStyle()
{
}

const FName FShooterMenuSoundsStyle::TypeName(TEXT("FShooterMenuSoundsStyle"));

const FShooterMenuSoundsStyle& FShooterMenuSoundsStyle::GetDefault()
{
	static FShooterMenuSoundsStyle Default;
	return Default;
}

void FShooterMenuSoundsStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
}


UShooterMenuSoundsWidgetStyle::UShooterMenuSoundsWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
