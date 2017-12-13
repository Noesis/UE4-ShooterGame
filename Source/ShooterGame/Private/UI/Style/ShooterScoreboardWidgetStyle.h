// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateWidgetStyleContainerBase.h"
#include "ShooterScoreboardWidgetStyle.generated.h"

/**
 * Represents the appearance of an SShooterScoreboardWidget
 */
USTRUCT()
struct FShooterScoreboardStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FShooterScoreboardStyle();
	virtual ~FShooterScoreboardStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FShooterScoreboardStyle& GetDefault();

	/**
	 * The brush used for the item border
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush ItemBorderBrush;
	FShooterScoreboardStyle& SetItemBorderBrush(const FSlateBrush& InItemBorderBrush) { ItemBorderBrush = InItemBorderBrush; return *this; }

	/**
	 * The color used for the kill stat
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateColor KillStatColor;
	FShooterScoreboardStyle& SetKillStatColor(const FSlateColor& InKillStatColor) { KillStatColor = InKillStatColor; return *this; }

	/**
	 * The color used for the death stat
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateColor DeathStatColor;
	FShooterScoreboardStyle& SetDeathStatColor(const FSlateColor& InDeathStatColor) { DeathStatColor = InDeathStatColor; return *this; }

	/**
	 * The color used for the score stat
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateColor ScoreStatColor;
	FShooterScoreboardStyle& SetScoreStatColor(const FSlateColor& InScoreStatColor) { ScoreStatColor = InScoreStatColor; return *this; }

	/**
	 * The sound that should play when the highlighted player changes in the scoreboard
	 */	
	UPROPERTY(EditAnywhere, Category=Sound)
	FSlateSound PlayerChangeSound;
	FShooterScoreboardStyle& SetPlayerChangeSound(const FSlateSound& InPlayerChangeSound) { PlayerChangeSound = InPlayerChangeSound; return *this; }
};


/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UShooterScoreboardWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_UCLASS_BODY()

public:
	/** The actual data describing the scoreboard's appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FShooterScoreboardStyle ScoreboardStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &ScoreboardStyle );
	}
};
