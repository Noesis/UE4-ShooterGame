// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ShooterGame.h"
#include "ShooterTypes.h"

class SShooterConfirmationDialog : public SCompoundWidget
{
public:
	/** The player that owns the dialog. */
	TWeakObjectPtr<ULocalPlayer> PlayerOwner;

	/** The delegate for confirming */
	FOnClicked OnConfirm;

	/** The delegate for cancelling */
	FOnClicked OnCancel;

	/* The type of dialog this is */
	EShooterDialogType::Type DialogType;

	SLATE_BEGIN_ARGS( SShooterConfirmationDialog )
	{}

	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)
	SLATE_ARGUMENT(EShooterDialogType::Type, DialogType)

	SLATE_ARGUMENT(FText, MessageText)
	SLATE_ARGUMENT(FText, ConfirmText)
	SLATE_ARGUMENT(FText, CancelText)

	SLATE_ARGUMENT(FOnClicked, OnConfirmClicked)
	SLATE_ARGUMENT(FOnClicked, OnCancelClicked)

	SLATE_END_ARGS()	

	void Construct(const FArguments& InArgs);

	virtual bool SupportsKeyboardFocus() const override;
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& KeyEvent) override;

private:

	FReply OnConfirmHandler();
	FReply ExecuteConfirm(const int32 UserIndex);

};
