// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "ShooterOptions.h"
#include "ShooterTypes.h"
#include "ShooterStyle.h"
#include "ShooterOptionsWidgetStyle.h"
#include "ShooterGameUserSettings.h"
#include "Player/ShooterPersistentUser.h"
#include "Player/ShooterLocalPlayer.h"

#define LOCTEXT_NAMESPACE "ShooterGame.HUD.Menu"

void FShooterOptions::Construct(ULocalPlayer* InPlayerOwner)
{
	OptionsStyle = &FShooterStyle::Get().GetWidgetStyle<FShooterOptionsStyle>("DefaultShooterOptionsStyle");

	PlayerOwner = InPlayerOwner;
	MinSensitivity = 1;
	
	TArray<FText> ResolutionList;
	TArray<FText> OnOffList;
	TArray<FText> SensitivityList;
	TArray<FText> GammaList;
	TArray<FText> LowHighList;

	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetInitialDisplayMetrics(DisplayMetrics);

	bool bAddedNativeResolution = false;
	const FIntPoint NativeResolution(DisplayMetrics.PrimaryDisplayWidth, DisplayMetrics.PrimaryDisplayHeight);

	for (int32 i = 0; i < DefaultShooterResCount; i++)
	{
		if (DefaultShooterResolutions[i].X <= DisplayMetrics.PrimaryDisplayWidth && DefaultShooterResolutions[i].Y <= DisplayMetrics.PrimaryDisplayHeight)
		{
			ResolutionList.Add(FText::Format(FText::FromString("{0}x{1}"), FText::FromString(FString::FromInt(DefaultShooterResolutions[i].X)), FText::FromString(FString::FromInt(DefaultShooterResolutions[i].Y))));
			Resolutions.Add(DefaultShooterResolutions[i]);

			bAddedNativeResolution = bAddedNativeResolution || (DefaultShooterResolutions[i] == NativeResolution);
		}
	}

	// Always make sure that the native resolution is available
	if (!bAddedNativeResolution)
	{
		ResolutionList.Add(FText::Format(FText::FromString("{0}x{1}"), FText::FromString(FString::FromInt(NativeResolution.X)), FText::FromString(FString::FromInt(NativeResolution.Y))));
		Resolutions.Add(NativeResolution);
	}

	OnOffList.Add(LOCTEXT("Off","OFF"));
	OnOffList.Add(LOCTEXT("On","ON"));

	LowHighList.Add(LOCTEXT("Low","LOW"));
	LowHighList.Add(LOCTEXT("High","HIGH"));

	//Mouse sensitivity 0-50
	for (int32 i = 0; i < 51; i++)
	{
		SensitivityList.Add(FText::AsNumber(i));
	}

	for (int32 i = -50; i < 51; i++)
	{
		GammaList.Add(FText::AsNumber(i));
	}

	/** Options menu root item */
	TSharedPtr<FShooterMenuItem> OptionsRoot = FShooterMenuItem::CreateRoot();

	/** Cheats menu root item */
	TSharedPtr<FShooterMenuItem> CheatsRoot = FShooterMenuItem::CreateRoot();

	CheatsItem = MenuHelper::AddMenuItem(CheatsRoot,LOCTEXT("Cheats", "CHEATS"));
	MenuHelper::AddMenuOptionSP(CheatsItem, LOCTEXT("InfiniteAmmo", "INFINITE AMMO"), OnOffList, this, &FShooterOptions::InfiniteAmmoOptionChanged);
	MenuHelper::AddMenuOptionSP(CheatsItem, LOCTEXT("InfiniteClip", "INFINITE CLIP"), OnOffList, this, &FShooterOptions::InfiniteClipOptionChanged);
	MenuHelper::AddMenuOptionSP(CheatsItem, LOCTEXT("FreezeMatchTimer", "FREEZE MATCH TIMER"), OnOffList, this, &FShooterOptions::FreezeTimerOptionChanged);
	MenuHelper::AddMenuOptionSP(CheatsItem, LOCTEXT("HealthRegen", "HP REGENERATION"), OnOffList, this, &FShooterOptions::HealthRegenOptionChanged);

	OptionsItem = MenuHelper::AddMenuItem(OptionsRoot,LOCTEXT("Options", "OPTIONS"));
#if PLATFORM_DESKTOP
	VideoResolutionOption = MenuHelper::AddMenuOptionSP(OptionsItem,LOCTEXT("Resolution", "RESOLUTION"), ResolutionList, this, &FShooterOptions::VideoResolutionOptionChanged);
	GraphicsQualityOption = MenuHelper::AddMenuOptionSP(OptionsItem,LOCTEXT("Quality", "QUALITY"),LowHighList, this, &FShooterOptions::GraphicsQualityOptionChanged);
	FullScreenOption = MenuHelper::AddMenuOptionSP(OptionsItem,LOCTEXT("FullScreen", "FULL SCREEN"),OnOffList, this, &FShooterOptions::FullScreenOptionChanged);
#endif
	GammaOption = MenuHelper::AddMenuOptionSP(OptionsItem,LOCTEXT("Gamma", "GAMMA CORRECTION"),GammaList, this, &FShooterOptions::GammaOptionChanged);
	AimSensitivityOption = MenuHelper::AddMenuOptionSP(OptionsItem,LOCTEXT("AimSensitivity", "AIM SENSITIVITY"),SensitivityList, this, &FShooterOptions::AimSensitivityOptionChanged);
	InvertYAxisOption = MenuHelper::AddMenuOptionSP(OptionsItem,LOCTEXT("InvertYAxis", "INVERT Y AXIS"),OnOffList, this, &FShooterOptions::InvertYAxisOptionChanged);
	VibrationOption = MenuHelper::AddMenuOptionSP(OptionsItem, LOCTEXT("Vibration", "VIBRATION"), OnOffList, this, &FShooterOptions::ToggleVibration);
	
	MenuHelper::AddMenuItemSP(OptionsItem,LOCTEXT("ApplyChanges", "APPLY CHANGES"), this, &FShooterOptions::OnApplySettings);

	//Do not allow to set aim sensitivity to 0
	AimSensitivityOption->MinMultiChoiceIndex = MinSensitivity;
    
    //Default vibration to On.
	VibrationOption->SelectedMultiChoice = 1;

	UserSettings = CastChecked<UShooterGameUserSettings>(GEngine->GetGameUserSettings());
	ResolutionOpt = UserSettings->GetScreenResolution();
	bFullScreenOpt = UserSettings->GetFullscreenMode();
	GraphicsQualityOpt = UserSettings->GetGraphicsQuality();

	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		bInvertYAxisOpt = PersistentUser->GetInvertedYAxis();
		SensitivityOpt = PersistentUser->GetAimSensitivity();
		GammaOpt = PersistentUser->GetGamma();
		bVibrationOpt = PersistentUser->GetVibration();
	}
	else
	{
		bVibrationOpt = true;
		bInvertYAxisOpt = false;
		SensitivityOpt = 1.0f;
		GammaOpt = 2.2f;
	}

	if (ensure(PlayerOwner != nullptr))
	{
		APlayerController* BaseController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(PlayerOwner->GetWorld(), GetOwnerUserIndex()));
		AShooterPlayerController* ShooterPlayerController = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(PlayerOwner->GetWorld(), GetOwnerUserIndex()));
		ensure(BaseController);
		if (BaseController)
		{
			if (ShooterPlayerController)
			{
				ShooterPlayerController->SetIsVibrationEnabled(bVibrationOpt);
			}
			else
			{
				// We are in the menus and therefore don't need to do anything as the controller is different
				// and can't store the vibration setting.
			}
		}
	}
}

void FShooterOptions::OnApplySettings()
{
	FSlateApplication::Get().PlaySound(OptionsStyle->AcceptChangesSound, GetOwnerUserIndex());
	ApplySettings();
}

void FShooterOptions::ApplySettings()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->SetAimSensitivity(SensitivityOpt);
		PersistentUser->SetInvertedYAxis(bInvertYAxisOpt);
		PersistentUser->SetGamma(GammaOpt);
		PersistentUser->SetVibration(bVibrationOpt);
		PersistentUser->TellInputAboutKeybindings();

		PersistentUser->SaveIfDirty();
	}

	UserSettings->SetScreenResolution(ResolutionOpt);
	UserSettings->SetFullscreenMode(bFullScreenOpt);
	UserSettings->SetGraphicsQuality(GraphicsQualityOpt);
	UserSettings->ApplySettings(false);

	OnApplyChanges.ExecuteIfBound();
}

void FShooterOptions::TellInputAboutKeybindings()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		PersistentUser->TellInputAboutKeybindings();
	}
}

void FShooterOptions::RevertChanges()
{
	FSlateApplication::Get().PlaySound(OptionsStyle->DiscardChangesSound, GetOwnerUserIndex());
	UpdateOptions();
	GEngine->DisplayGamma =  2.2f + 2.0f * (-0.5f + GammaOption->SelectedMultiChoice / 100.0f);
}

int32 FShooterOptions::GetCurrentResolutionIndex(FIntPoint CurrentRes)
{
	int32 Result = 0; // return first valid resolution if match not found
	for (int32 i = 0; i < Resolutions.Num(); i++)
	{
		if (Resolutions[i] == CurrentRes)
		{
			Result = i;
			break;
		}
	}
	return Result;
}

int32 FShooterOptions::GetCurrentMouseYAxisInvertedIndex()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		return InvertYAxisOption->SelectedMultiChoice = PersistentUser->GetInvertedYAxis() ? 1 : 0;
	}
	else
	{
		return 0;
	}
}

int32 FShooterOptions::GetCurrentMouseSensitivityIndex()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		//mouse sensitivity is a floating point value ranged from 0.0f to 1.0f
		int32 IntSensitivity = FMath::RoundToInt((PersistentUser->GetAimSensitivity() - 0.5f) * 10.0f);
		//Clamp to valid index range
		return FMath::Clamp(IntSensitivity, MinSensitivity, 100);
	}

	return FMath::RoundToInt((1.0f - 0.5f) * 10.0f);
}

int32 FShooterOptions::GetCurrentGammaIndex()
{
	UShooterPersistentUser* PersistentUser = GetPersistentUser();
	if(PersistentUser)
	{
		//reverse gamma calculation
		int32 GammaIndex = FMath::TruncToInt(((PersistentUser->GetGamma() - 2.2f) / 2.0f + 0.5f) * 100);
		//Clamp to valid index range
		return FMath::Clamp(GammaIndex, 0, 100);
	}

	return FMath::TruncToInt(((2.2f - 2.2f) / 2.0f + 0.5f) * 100);
}

int32 FShooterOptions::GetOwnerUserIndex() const
{
	return PlayerOwner ? PlayerOwner->GetControllerId() : 0;
}

UShooterPersistentUser* FShooterOptions::GetPersistentUser() const
{
	UShooterLocalPlayer* const SLP = Cast<UShooterLocalPlayer>(PlayerOwner);
	if (SLP)
	{
		return SLP->GetPersistentUser();
	}

	return nullptr;
}

void FShooterOptions::UpdateOptions()
{
#if UE_BUILD_SHIPPING
	CheatsItem->bVisible = false;
#else
	//Toggle Cheat menu visibility depending if we are client or server
	UWorld* const World = PlayerOwner->GetWorld();
	if (World && World->GetNetMode() == NM_Client)
	{
		CheatsItem->bVisible = false;
	}
	else
	{
		CheatsItem->bVisible = true;
	}
#endif

	//grab the user settings
	UShooterPersistentUser* const PersistentUser = GetPersistentUser();
	if (PersistentUser)
	{
		// Update bInvertYAxisOpt, SensitivityOpt and GammaOpt because the ShooterOptions can be created without the controller having a player
		// by the in-game menu which will leave them with default values
		bInvertYAxisOpt = PersistentUser->GetInvertedYAxis();
		SensitivityOpt = PersistentUser->GetAimSensitivity();
		GammaOpt = PersistentUser->GetGamma();
		bVibrationOpt = PersistentUser->GetVibration();
	} 

	InvertYAxisOption->SelectedMultiChoice =  GetCurrentMouseYAxisInvertedIndex();
	AimSensitivityOption->SelectedMultiChoice = GetCurrentMouseSensitivityIndex();
	GammaOption->SelectedMultiChoice = GetCurrentGammaIndex();

	GammaOptionChanged(GammaOption, GammaOption->SelectedMultiChoice);
#if PLATFORM_DESKTOP
	VideoResolutionOption->SelectedMultiChoice = GetCurrentResolutionIndex(UserSettings->GetScreenResolution());
	GraphicsQualityOption->SelectedMultiChoice = UserSettings->GetGraphicsQuality();
	FullScreenOption->SelectedMultiChoice = UserSettings->GetFullscreenMode() != EWindowMode::Windowed ? 1 : 0;
#endif
}

void FShooterOptions::InfiniteAmmoOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	UWorld* const World = PlayerOwner->GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AShooterPlayerController* ShooterPC = Cast<AShooterPlayerController>(*It);
			if (ShooterPC)
			{
				ShooterPC->SetInfiniteAmmo(MultiOptionIndex > 0 ? true : false);
			}
		}
	}
}

void FShooterOptions::InfiniteClipOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	UWorld* const World = PlayerOwner->GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AShooterPlayerController* const ShooterPC = Cast<AShooterPlayerController>(*It);
			if (ShooterPC)
			{
				ShooterPC->SetInfiniteClip(MultiOptionIndex > 0 ? true : false);
			}
		}
	}
}

void FShooterOptions::FreezeTimerOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	UWorld* const World = PlayerOwner->GetWorld();
	AShooterGameState* const GameState = World ? World->GetGameState<AShooterGameState>() : nullptr;
	if (GameState)
	{
		GameState->bTimerPaused = MultiOptionIndex > 0  ? true : false;
	}
}


void FShooterOptions::HealthRegenOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	UWorld* const World = PlayerOwner->GetWorld();
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			AShooterPlayerController* const ShooterPC = Cast<AShooterPlayerController>(*It);
			if (ShooterPC)
			{
				ShooterPC->SetHealthRegen(MultiOptionIndex > 0 ? true : false);
			}
		}
	}
}

void FShooterOptions::VideoResolutionOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	ResolutionOpt = Resolutions[MultiOptionIndex];
}

void FShooterOptions::GraphicsQualityOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	GraphicsQualityOpt = MultiOptionIndex;
}

void FShooterOptions::FullScreenOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	static auto CVar = IConsoleManager::Get().FindTConsoleVariableDataInt(TEXT("r.FullScreenMode"));
	auto FullScreenMode = CVar->GetValueOnGameThread() == 1 ? EWindowMode::WindowedFullscreen : EWindowMode::Fullscreen;
	bFullScreenOpt = MultiOptionIndex == 0 ? EWindowMode::Windowed : FullScreenMode;
}

void FShooterOptions::AimSensitivityOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	SensitivityOpt = 0.5f + (MultiOptionIndex / 10.0f);
}

void FShooterOptions::GammaOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	GammaOpt = 2.2f + 2.0f * (-0.5f + MultiOptionIndex / 100.0f);
	GEngine->DisplayGamma = GammaOpt;
}

void FShooterOptions::ToggleVibration(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	bVibrationOpt = MultiOptionIndex > 0 ? true : false;
	APlayerController* BaseController = Cast<APlayerController>(UGameplayStatics::GetPlayerController(PlayerOwner->GetWorld(), GetOwnerUserIndex()));
	AShooterPlayerController* ShooterPlayerController = Cast<AShooterPlayerController>(UGameplayStatics::GetPlayerController(PlayerOwner->GetWorld(), GetOwnerUserIndex()));
	ensure(BaseController);
    if(BaseController)
    {
		if (ShooterPlayerController)
		{
			ShooterPlayerController->SetIsVibrationEnabled(bVibrationOpt);
		}
		else
		{
			// We are in the menus and therefore don't need to do anything as the controller is different
			// and can't store the vibration setting.
		}
    }
}

void FShooterOptions::InvertYAxisOptionChanged(TSharedPtr<FShooterMenuItem> MenuItem, int32 MultiOptionIndex)
{
	bInvertYAxisOpt = MultiOptionIndex > 0  ? true : false;
}

#undef LOCTEXT_NAMESPACE
