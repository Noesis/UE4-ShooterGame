// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"

//class declare
class SShooterMenuItem : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_OneParam( FOnArrowPressed, int );

	SLATE_BEGIN_ARGS(SShooterMenuItem)
	{}

	/** weak pointer to the parent PC */
	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)

	/** called when the button is clicked */
	SLATE_EVENT(FOnClicked, OnClicked)

	/** called when the left or right arrow is clicked */
	SLATE_EVENT(FOnArrowPressed, OnArrowPressed)

	/** menu item text attribute */
	SLATE_ATTRIBUTE(FText, Text)

	/** is it multi-choice item? */
	SLATE_ARGUMENT(bool, bIsMultichoice)

	/** menu item option text attribute */
	SLATE_ATTRIBUTE(FText, OptionText)

	/** menu item text transparency when item is not active, optional argument */
	SLATE_ARGUMENT(TOptional<float>, InactiveTextAlpha)

	/** end of slate attributes definition */
	SLATE_END_ARGS()

	/** needed for every widget */
	void Construct(const FArguments& InArgs);

	/** says that we can support keyboard focus */
	virtual bool SupportsKeyboardFocus() const override { return true; }

	/** mouse button down callback */
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** mouse button up callback */
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** sets this menu item as active (selected) */
	void SetMenuItemActive(bool bIsMenuItemActive);

	/** modify the displayed item text */
	void UpdateItemText(const FText& UpdatedText);

	/** set in option item to enable left arrow*/
	EVisibility LeftArrowVisible;

	/** set in option item to enable right arrow*/
	EVisibility RightArrowVisible;

protected:
	/** the delegate to execute when the button is clicked */
	FOnClicked OnClicked;

	/** the delegate to execute when one of arrows was pressed */
	FOnArrowPressed OnArrowPressed;

private:
	/** menu item text attribute */
	TAttribute< FText > Text;

	/** menu item option text attribute */
	TAttribute< FText > OptionText;

	/** menu item text widget */
	TSharedPtr<STextBlock> TextWidget;

	/** menu item text color */
	FLinearColor TextColor;

	/** item margin */
	float ItemMargin;

	/** getter for menu item background color */
	FSlateColor GetButtonBgColor() const;

	/** getter for menu item text color */
	FSlateColor GetButtonTextColor() const;

	/** getter for menu item text shadow color */
	FLinearColor GetButtonTextShadowColor() const;

	/** getter for left option arrow visibility */
	EVisibility GetLeftArrowVisibility() const;

	/** getter for right option arrow visibility */
	EVisibility GetRightArrowVisibility() const;

	/** getter option padding (depends on right arrow visibility) */
	FMargin GetOptionPadding() const;

	/** calls OnArrowPressed */
	FReply OnRightArrowDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	/** calls OnArrowPressed */
	FReply OnLeftArrowDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);

	/** inactive text alpha value*/
	float InactiveTextAlpha;

	/** active item flag */
	bool bIsActiveMenuItem;

	/** is this menu item represents multi-choice field */
	bool bIsMultichoice;

	/** pointer to our parent PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	/** style for the menu item */
	const struct FShooterMenuItemStyle *ItemStyle;
};


