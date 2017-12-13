// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterChatWidgetStyle.h"

FShooterChatStyle::FShooterChatStyle()
{
}

FShooterChatStyle::~FShooterChatStyle()
{
}

const FName FShooterChatStyle::TypeName(TEXT("FShooterChatStyle"));

const FShooterChatStyle& FShooterChatStyle::GetDefault()
{
	static FShooterChatStyle Default;
	return Default;
}

void FShooterChatStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	TextEntryStyle.GetResources(OutBrushes);

	OutBrushes.Add(&BackingBrush);
}


UShooterChatWidgetStyle::UShooterChatWidgetStyle( const FObjectInitializer& ObjectInitializer )
	: Super(ObjectInitializer)
{
	
}
