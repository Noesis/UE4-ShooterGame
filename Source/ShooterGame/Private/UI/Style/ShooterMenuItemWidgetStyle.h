// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateWidgetStyleContainerBase.h"
#include "ShooterMenuItemWidgetStyle.generated.h"

/**
 * Represents the appearance of an FShooterMenuItem
 */
USTRUCT()
struct FShooterMenuItemStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FShooterMenuItemStyle();
	virtual ~FShooterMenuItemStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FShooterMenuItemStyle& GetDefault();

	/**
	 * The brush used for the item background
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush BackgroundBrush;
	FShooterMenuItemStyle& SetBackgroundBrush(const FSlateBrush& InBackgroundBrush) { BackgroundBrush = InBackgroundBrush; return *this; }

	/**
	 * The image used for the left arrow
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush LeftArrowImage;
	FShooterMenuItemStyle& SetLeftArrowImage(const FSlateBrush& InLeftArrowImage) { LeftArrowImage = InLeftArrowImage; return *this; }

	/**
	 * The image used for the right arrow
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush RightArrowImage;
	FShooterMenuItemStyle& SetRightArrowImage(const FSlateBrush& InRightArrowImage) { RightArrowImage = InRightArrowImage; return *this; }
};


/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UShooterMenuItemWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_UCLASS_BODY()

public:
	/** The actual data describing the menu's appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FShooterMenuItemStyle MenuItemStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &MenuItemStyle );
	}
};
