// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateWidgetStyleContainerBase.h"
#include "ShooterMenuWidgetStyle.generated.h"

/**
 * Represents the appearance of an SShooterMenuWidget
 */
USTRUCT()
struct FShooterMenuStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FShooterMenuStyle();
	virtual ~FShooterMenuStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FShooterMenuStyle& GetDefault();

	/**
	 * The brush used for the header background
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush HeaderBackgroundBrush;
	FShooterMenuStyle& SetHeaderBackgroundBrush(const FSlateBrush& InHeaderBackgroundBrush) { HeaderBackgroundBrush = InHeaderBackgroundBrush; return *this; }

	/**
	 * The brush used for the left side of the menu
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush LeftBackgroundBrush;
	FShooterMenuStyle& SetLeftBackgroundBrush(const FSlateBrush& InLeftBackgroundBrush) { LeftBackgroundBrush = InLeftBackgroundBrush; return *this; }

	/**
	 * The brush used for the right side of the menu
	 */	
	UPROPERTY(EditAnywhere, Category=Appearance)
	FSlateBrush RightBackgroundBrush;
	FShooterMenuStyle& SetRightBackgroundBrush(const FSlateBrush& InRightBackgroundBrush) { RightBackgroundBrush = InRightBackgroundBrush; return *this; }

	/**
	 * The sound that should play when entering a sub-menu
	 */	
	UPROPERTY(EditAnywhere, Category=Sound)
	FSlateSound MenuEnterSound;
	FShooterMenuStyle& SetMenuEnterSound(const FSlateSound& InMenuEnterSound) { MenuEnterSound = InMenuEnterSound; return *this; }

	/**
	 * The sound that should play when leaving a sub-menu
	 */	
	UPROPERTY(EditAnywhere, Category=Sound)
	FSlateSound MenuBackSound;
	FShooterMenuStyle& SetMenuBackSound(const FSlateSound& InMenuBackSound) { MenuBackSound = InMenuBackSound; return *this; }

	/**
	 * The sound that should play when changing an option
	 */	
	UPROPERTY(EditAnywhere, Category=Sound)
	FSlateSound OptionChangeSound;
	FShooterMenuStyle& SetOptionChangeSound(const FSlateSound& InOptionChangeSound) { OptionChangeSound = InOptionChangeSound; return *this; }

	/**
	 * The sound that should play when changing the selected menu item
	 */	
	UPROPERTY(EditAnywhere, Category=Sound)
	FSlateSound MenuItemChangeSound;
	FShooterMenuStyle& SetMenuItemChangeSound(const FSlateSound& InMenuItemChangeSound) { MenuItemChangeSound = InMenuItemChangeSound; return *this; }
};


/**
 */
UCLASS(hidecategories=Object, MinimalAPI)
class UShooterMenuWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_UCLASS_BODY()

public:
	/** The actual data describing the menu's appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FShooterMenuStyle MenuStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast< const struct FSlateWidgetStyle* >( &MenuStyle );
	}
};
