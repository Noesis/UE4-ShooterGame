// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterScoreboardWidgetStyle.h"

FShooterScoreboardStyle::FShooterScoreboardStyle()
{
}

FShooterScoreboardStyle::~FShooterScoreboardStyle()
{
}

const FName FShooterScoreboardStyle::TypeName(TEXT("FShooterScoreboardStyle"));

const FShooterScoreboardStyle& FShooterScoreboardStyle::GetDefault()
{
	static FShooterScoreboardStyle Default;
	return Default;
}

void FShooterScoreboardStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	OutBrushes.Add(&ItemBorderBrush);
}


UShooterScoreboardWidgetStyle::UShooterScoreboardWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
