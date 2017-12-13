// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateWidgetStyleContainerBase.h"
#include "ShooterOptionsWidgetStyle.generated.h"

/**
 * Represents the appearance of an FShooterOptions
 */
USTRUCT()
struct FShooterOptionsStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FShooterOptionsStyle();
	virtual ~FShooterOptionsStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FShooterOptionsStyle& GetDefault();

	/**
	 * The sound the options should play when changes are accepted
	 */	
	UPROPERTY(EditAnywhere, Category=Sound)
	FSlateSound AcceptChangesSound;
	FShooterOptionsStyle& SetAcceptChangesSound(const FSlateSound& InAcceptChangesSound) { AcceptChangesSound = InAcceptChangesSound; return *this; }

	/**
	 * The sound the options should play when changes are discarded
	 */	
	UPROPERTY(EditAnywhere, Category=Sound)
	FSlateSound DiscardChangesSound;
	FShooterOptionsStyle& SetDiscardChangesSound(const FSlateSound& InDiscardChangesSound) { DiscardChangesSound = InDiscardChangesSound; return *this; }
};


/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UShooterOptionsWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_UCLASS_BODY()

public:
	/** The actual data describing the menu's appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FShooterOptionsStyle OptionsStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &OptionsStyle );
	}
};
