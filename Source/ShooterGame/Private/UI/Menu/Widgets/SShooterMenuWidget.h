// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "ShooterMenuItem.h"

//class declare
class SShooterMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SShooterMenuWidget)
	: _PlayerOwner()
	, _IsGameMenu(false)
	{
	}

	/** weak pointer to the parent HUD base */
	SLATE_ARGUMENT(TWeakObjectPtr<ULocalPlayer>, PlayerOwner)

	/** is this main menu or in game menu? */
	SLATE_ARGUMENT(bool, IsGameMenu)
	
	/** always goes here */
	SLATE_END_ARGS()

	/** delegate declaration */
	DECLARE_DELEGATE(FOnMenuHidden);

	/** external delegate to call when in-game menu should be hidden using controller buttons - 
	it's workaround as when joystick is captured, even when sending FReply::Unhandled, binding does not recieve input :( */
	DECLARE_DELEGATE(FOnToggleMenu);

	/** called when user is going back from submenu, useful for resetting changes if they were not confirmed */
	DECLARE_DELEGATE_OneParam(FOnMenuGoBack, MenuPtr);

	/** every widget needs a construction function */
	void Construct(const FArguments& InArgs);

	/** update function. Kind of a hack. Allows us to only start fading in once we are done loading. */
	virtual void Tick( const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime ) override;

	/** to have the mouse cursor show up at all times, we need the widget to handle all mouse events */
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

	/** key down handler */
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

	/** says that we can support keyboard focus */
	virtual bool SupportsKeyboardFocus() const override { return true; }

	/** The menu sets up the appropriate mouse settings upon focus */
	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override;

	/** setups animation lengths, start points and launches initial animations */
	void SetupAnimations();

	/** builds left menu panel */
	void BuildLeftPanel(bool bGoingBack);

	/** builds inactive next menu panel (current selections submenu preview) */
	void BuildRightPanel();

	/** starts animations to enter submenu, it will become active menu */
	void EnterSubMenu();

	/** starts reverse animations to go one level up in menu hierarchy */
	void MenuGoBack(bool bSilent=false);

	/** confirms current menu item and performs an action */
	void ConfirmMenuItem();

	/** views a friend's profile in the current user's in-game menu friend list */
	void ControllerFacebuttonLeftPressed();

	/** decrement the index of the friend that the user is currently selecting while in the in-game menu friend list */
	void ControllerUpInputPressed();

	/** increment the index of the friend that the user is currently selecting while in the in-game menu friend list */
	void ControllerDownInputPressed();

	/** Sends a friend invite to a friend in the current user's in-game menu friend list */
	void ControllerFacebuttonDownPressed();

	/** call to rebuild menu and start animating it */
	void BuildAndShowMenu();

	/** call to hide menu */
	void HideMenu();

	/** updates arrows visibility for multi-choice menu item */
	void UpdateArrows(TSharedPtr<FShooterMenuItem> MenuItem);

	/** changes option in multi-choice menu item */
	void ChangeOption(int32 MoveBy);

	/** get next valid index, ignoring invisible items */
	int32 GetNextValidIndex(int32 MoveBy);

	/** disable/enable moving around menu */
	void LockControls(bool bEnable);

	/** Cache the UserIndex from the owning PlayerController */
	int32 GetOwnerUserIndex();

	/** returns the number of sublevels on the menu stack */
	int32 GetMenuLevel();

	/** main menu for this instance of widget */
	MenuPtr MainMenu;

	/** currently active menu */
	MenuPtr CurrentMenu;

	/** next menu (for transition and displaying as the right menu) */
	MenuPtr NextMenu;

	/** stack of previous menus */
	TArray<FShooterMenuInfo> MenuHistory;

	/** delegate, which is executed when menu is finished hiding */
	FOnMenuHidden OnMenuHidden;

	/** bind if menu should be hidden from outside by controller button */
	FOnToggleMenu OnToggleMenu;

	/** executed when user wants to go back to previous menu */
	FOnMenuGoBack OnGoBack;

	/** current menu title if present */
	FText CurrentMenuTitle;

	/** default - start button, change to use different */
	FKey ControllerHideMenuKey;

	/** if console is currently opened */
	bool bConsoleVisible;	

private:

	/** sets hit test invisibility when console is up */
	EVisibility GetSlateVisibility() const;

	/** getters used for animating the menu */
	FVector2D GetBottomScale() const;
	FLinearColor GetBottomColor() const;
	FLinearColor GetTopColor() const;
	FMargin GetMenuOffset() const;
	FMargin GetLeftMenuOffset() const;
	FMargin GetSubMenuOffset() const;

	/** gets header image color */
	FSlateColor GetHeaderColor() const;

	/** callback for when one of the N buttons is clicked */
	FReply ButtonClicked(int32 ButtonIndex);

	/** gets currently selected multi-choice option */
	FText GetOptionText(TSharedPtr<FShooterMenuItem> MenuItem) const;

	/** gets current menu title string */
	FText GetMenuTitle() const; 

	/** gets the offset of the swap profile UI from the edge of the screen */
	FMargin GetProfileSwapOffset() const;

	/** should the profile swap be active */
	bool IsProfileSwapActive() const;

	/** gets the visibility of the swap profile UI */
	EVisibility GetProfileSwapVisibility() const;

	/** called when we want to swap the logged in user */
	bool ProfileUISwap(const int ControllerIndex) const; 

	/** delegate for if the profile is swapped */
	void HandleProfileUISwapClosed(TSharedPtr<const FUniqueNetId> UniqueId, const int ControllerIndex);

	/** this function starts the entire fade in process */
	void FadeIn();

	/** our curve sequence and the related handles */
	FCurveSequence MenuWidgetAnimation;

	/** used for menu background scaling animation at the beginning */ 
	FCurveHandle BottomScaleYCurve;

	/** used for main menu logo fade in animation at the beginning  */
	FCurveHandle TopColorCurve;

	/** used for menu background fade in animation at the beginning */
	FCurveHandle BottomColorCurve;

	/** used for menu buttons slide in animation at the beginning */
	FCurveHandle ButtonsPosXCurve;

	/** sub menu transition animation sequence */
	FCurveSequence SubMenuWidgetAnimation;

	/** sub menu transition animation curve */
	FCurveHandle SubMenuScrollOutCurve;

	/** current menu transition animation sequence */
	FCurveSequence LeftMenuWidgetAnimation;

	/** current menu transition animation curve */
	FCurveHandle LeftMenuScrollOutCurve;

	/** weak pointer to our parent PC */
	TWeakObjectPtr<class ULocalPlayer> PlayerOwner;

	/** screen resolution */
	FIntPoint ScreenRes;

	/** space between menu item and border */
	float OutlineWidth;

	/** menu header height */
	float MenuHeaderHeight;

	/** menu header width */
	float MenuHeaderWidth;

	/** menu profile width */
	float MenuProfileWidth;

	/** animation type index */
	int32 AnimNumber;

	/** selected index of current menu */
	int32 SelectedIndex;

	
	/** right panel animating flag */
	bool bSubMenuChanging;

	/** left panel animating flag */
	bool bLeftMenuChanging;

	/** going back to previous menu animation flag */
	bool bGoingBack;

	/** flag when playing hiding animation */
	bool bMenuHiding;

	/** if this is in game menu, do not show background or logo */
	bool bGameMenu;

	/** if moving around menu is currently locked */
	bool bControlsLocked;		
	
	/** menu that will override current one after transition animation */
	MenuPtr PendingLeftMenu;

	/** left(current) menu layout box */
	TSharedPtr<SVerticalBox> LeftBox;

	/** right(sub) menu layout box */
	TSharedPtr<SVerticalBox> RightBox;

	/** style for the menu widget */
	const struct FShooterMenuStyle *MenuStyle;
};

namespace MenuHelper
{
	FORCEINLINE void EnsureValid(TSharedPtr<FShooterMenuItem>& MenuItem)
	{
		if (!MenuItem.IsValid())
		{
			MenuItem = FShooterMenuItem::CreateRoot();
		}
	}

	//Helper functions for creating menu items
	FORCEINLINE TSharedRef<FShooterMenuItem> AddMenuItem(TSharedPtr<FShooterMenuItem>& MenuItem, const FText& Text)
	{
		EnsureValid(MenuItem);
		TSharedPtr<FShooterMenuItem> Item = MakeShareable(new FShooterMenuItem(Text));
		MenuItem->SubMenu.Add(Item);
		return Item.ToSharedRef();
	}

	/** add standard item to menu with UObject delegate */
	template< class UserClass >	
	FORCEINLINE TSharedRef<FShooterMenuItem> AddMenuItem(TSharedPtr<FShooterMenuItem>& MenuItem, const FText& Text, UserClass* inObj, typename FShooterMenuItem::FOnConfirmMenuItem::TUObjectMethodDelegate< UserClass >::FMethodPtr inMethod)
	{
		EnsureValid(MenuItem);
		TSharedPtr<FShooterMenuItem> Item = MakeShareable(new FShooterMenuItem(Text));
		Item->OnConfirmMenuItem.BindUObject(inObj,inMethod);
		MenuItem->SubMenu.Add(Item);
		return Item.ToSharedRef();
	}

	/** add standard item to menu with TSharedPtr delegate */
	template< class UserClass >	
	FORCEINLINE TSharedRef<FShooterMenuItem> AddMenuItemSP(TSharedPtr<FShooterMenuItem>& MenuItem, const FText& Text, UserClass* inObj, typename FShooterMenuItem::FOnConfirmMenuItem::TSPMethodDelegate< UserClass >::FMethodPtr inMethod)
	{
		EnsureValid(MenuItem);
		TSharedPtr<FShooterMenuItem> Item = MakeShareable(new FShooterMenuItem(Text));
		Item->OnConfirmMenuItem.BindSP(inObj,inMethod);
		MenuItem->SubMenu.Add(Item);
		return Item.ToSharedRef();
	}


	FORCEINLINE TSharedRef<FShooterMenuItem> AddMenuOption(TSharedPtr<FShooterMenuItem>& MenuItem, const FText& Text, const TArray<FText>& OptionsList)
	{
		EnsureValid(MenuItem);
		TSharedPtr<FShooterMenuItem> Item = MakeShareable(new FShooterMenuItem(Text, OptionsList));
		MenuItem->SubMenu.Add(Item);
		return MenuItem->SubMenu.Last().ToSharedRef();
	}

	/** add multi-choice item to menu with UObject delegate */
	template< class UserClass >	
	FORCEINLINE TSharedRef<FShooterMenuItem> AddMenuOption(TSharedPtr<FShooterMenuItem>& MenuItem, const FText& Text, const TArray<FText>& OptionsList, UserClass* inObj, typename FShooterMenuItem::FOnOptionChanged::TUObjectMethodDelegate< UserClass >::FMethodPtr inMethod)
	{
		EnsureValid(MenuItem);
		TSharedPtr<FShooterMenuItem> Item = MakeShareable(new FShooterMenuItem(Text, OptionsList));
		Item->OnOptionChanged.BindUObject(inObj, inMethod);
		MenuItem->SubMenu.Add(Item);
		return MenuItem->SubMenu.Last().ToSharedRef();
	}

	/** add multi-choice item to menu with TSharedPtr delegate */
	template< class UserClass >	
	FORCEINLINE TSharedRef<FShooterMenuItem> AddMenuOptionSP(TSharedPtr<FShooterMenuItem>& MenuItem, const FText& Text, const TArray<FText>& OptionsList, UserClass* inObj, typename FShooterMenuItem::FOnOptionChanged::TSPMethodDelegate< UserClass >::FMethodPtr inMethod)
	{
		EnsureValid(MenuItem);
		TSharedPtr<FShooterMenuItem> Item = MakeShareable(new FShooterMenuItem(Text, OptionsList));
		Item->OnOptionChanged.BindSP(inObj, inMethod);
		MenuItem->SubMenu.Add(Item);
		return MenuItem->SubMenu.Last().ToSharedRef();
	}


	FORCEINLINE TSharedRef<FShooterMenuItem> AddExistingMenuItem(TSharedPtr<FShooterMenuItem>& MenuItem, TSharedRef<FShooterMenuItem> SubMenuItem)
	{
		EnsureValid(MenuItem);
		MenuItem->SubMenu.Add(SubMenuItem);
		return MenuItem->SubMenu.Last().ToSharedRef();
	}


	FORCEINLINE TSharedRef<FShooterMenuItem> AddCustomMenuItem(TSharedPtr<FShooterMenuItem>& MenuItem, TSharedPtr<SWidget> CustomWidget)
	{
		EnsureValid(MenuItem);
		MenuItem->SubMenu.Add(MakeShareable(new FShooterMenuItem(CustomWidget)));
		return MenuItem->SubMenu.Last().ToSharedRef();
	}

	FORCEINLINE void ClearSubMenu(TSharedPtr<FShooterMenuItem>& MenuItem)
	{
		EnsureValid(MenuItem);
		MenuItem->SubMenu.Empty();
	}

	template< class UserClass >	
	FORCEINLINE void PlaySoundAndCall(UWorld* World, const FSlateSound& Sound, int32 UserIndex, UserClass* inObj, typename FShooterMenuItem::FOnConfirmMenuItem::TSPMethodDelegate< UserClass >::FMethodPtr inMethod)
	{
		FSlateApplication::Get().PlaySound(Sound, UserIndex);
		if (World)
		{
			const float SoundDuration = FMath::Max(FSlateApplication::Get().GetSoundDuration(Sound), 0.1f);
			FTimerHandle DummyHandle;
			World->GetTimerManager().SetTimer(DummyHandle, FTimerDelegate::CreateSP(inObj, inMethod), SoundDuration, false);
		}
		else
		{
			FTimerDelegate D = FTimerDelegate::CreateSP(inObj, inMethod);
			D.ExecuteIfBound();
		}
	}

}
