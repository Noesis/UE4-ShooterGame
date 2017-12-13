// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterTypes.h"
#include "ShooterGameViewportClient.generated.h"

class SShooterConfirmationDialog;

struct FShooterGameLoadingScreenBrush : public FSlateDynamicImageBrush, public FGCObject
{
	FShooterGameLoadingScreenBrush( const FName InTextureName, const FVector2D& InImageSize )
		: FSlateDynamicImageBrush( InTextureName, InImageSize )
	{
		ResourceObject = LoadObject<UObject>( NULL, *InTextureName.ToString() );
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector)
	{
		if( ResourceObject )
		{
			Collector.AddReferencedObject(ResourceObject);
		}
	}
};

class SShooterLoadingScreen : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterLoadingScreen) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	EVisibility GetLoadIndicatorVisibility() const
	{
		return EVisibility::Visible;
	}

	/** loading screen image brush */
	TSharedPtr<FSlateDynamicImageBrush> LoadingScreenBrush;
};

UCLASS(Within=Engine, transient, config=Engine)
class UShooterGameViewportClient : public UGameViewportClient
{
	GENERATED_UCLASS_BODY()

public:

 	// start UGameViewportClient interface
 	void NotifyPlayerAdded( int32 PlayerIndex, ULocalPlayer* AddedPlayer ) override;
	void AddViewportWidgetContent( TSharedRef<class SWidget> ViewportContent, const int32 ZOrder = 0 ) override;
	void RemoveViewportWidgetContent( TSharedRef<class SWidget> ViewportContent ) override;

	void ShowDialog(TWeakObjectPtr<ULocalPlayer> PlayerOwner, EShooterDialogType::Type DialogType, const FText& Message, const FText& Confirm, const FText& Cancel, const FOnClicked& OnConfirm, const FOnClicked& OnCancel);
	void HideDialog();

	void ShowLoadingScreen();
	void HideLoadingScreen();

	bool IsShowingDialog() const { return DialogWidget.IsValid(); }

	EShooterDialogType::Type GetDialogType() const;
	TWeakObjectPtr<ULocalPlayer> GetDialogOwner() const;

	TSharedPtr<SShooterConfirmationDialog> GetDialogWidget() { return DialogWidget; }

	//FTicker Funcs
	virtual void Tick(float DeltaSeconds) override;	

#if WITH_EDITOR
	virtual void DrawTransition(class UCanvas* Canvas) override;
#endif //WITH_EDITOR
	// end UGameViewportClient interface

protected:
	void HideExistingWidgets();
	void ShowExistingWidgets();

	/** List of viewport content that the viewport is tracking */
	TArray<TSharedRef<class SWidget>>				ViewportContentStack;

	TArray<TSharedRef<class SWidget>>				HiddenViewportContentStack;

	TSharedPtr<class SWidget>						OldFocusWidget;

	/** Dialog widget to show temporary messages ("Controller disconnected", "Parental Controls don't allow you to play online", etc) */
	TSharedPtr<SShooterConfirmationDialog>			DialogWidget;

	TSharedPtr<SShooterLoadingScreen>				LoadingScreenWidget;
};