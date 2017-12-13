// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once
#include "ShooterPersistentUser.generated.h"

UCLASS(BlueprintType)
class UShooterPersistentUser : public USaveGame
{
	GENERATED_UCLASS_BODY()

public:
	/** Loads user persistence data if it exists, creates an empty record otherwise. */
	static UShooterPersistentUser* LoadPersistentUser(FString SlotName, const int32 UserIndex);

	/** Saves data if anything has changed. */
	UFUNCTION(BlueprintCallable)
	void SaveIfDirty();

	/** Records the result of a match. */
	void AddMatchResult(int32 MatchKills, int32 MatchDeaths, int32 MatchBulletsFired, int32 MatchRocketsFired, bool bIsMatchWinner);

	/** needed because we can recreate the subsystem that stores it */
	UFUNCTION(BlueprintCallable)
	void TellInputAboutKeybindings();

	int32 GetUserIndex() const;

	FORCEINLINE int32 GetKills() const
	{
		return Kills;
	}

	FORCEINLINE int32 GetDeaths() const
	{
		return Deaths;
	}

	FORCEINLINE int32 GetWins() const
	{
		return Wins;
	}

	FORCEINLINE int32 GetLosses() const
	{
		return Losses;
	}

	FORCEINLINE int32 GetBulletsFired() const
	{
		return BulletsFired;
	}

	FORCEINLINE int32 GetRocketsFired() const
	{
		return RocketsFired;
	}

	/** Is controller vibration turned on? */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetVibration() const 
	{
		return bVibrationOpt;
	}

	/** Is the y axis inverted? */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetInvertedYAxis() const
	{
		return bInvertedYAxis;
	}

	/** Setter for controller vibration option */
	UFUNCTION(BlueprintCallable)
	void SetVibration(bool bVibration);

	/** Setter for inverted y axis */
	UFUNCTION(BlueprintCallable)
	void SetInvertedYAxis(bool bInvert);

	/** Getter for the aim sensitivity */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetAimSensitivity() const
	{
		return AimSensitivity;
	}

	UFUNCTION(BlueprintCallable)
	void SetAimSensitivity(float InSensitivity);

	/** Getter for the gamma correction */
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetGamma() const
	{
		return Gamma;
	}

	UFUNCTION(BlueprintCallable)
	void SetGamma(float InGamma);

	FORCEINLINE int32 GetBotsCount() const
	{
		return BotsCount;
	}

	void SetBotsCount(int32 InCount);

	FORCEINLINE bool IsRecordingDemos() const
	{
		return bIsRecordingDemos;
	}

	void SetIsRecordingDemos(const bool InbIsRecordingDemos);

	FORCEINLINE FString GetName() const
	{
		return SlotName;
	}

protected:
	void SetToDefaults();

	/** Checks if the Mouse Sensitivity user setting is different from current */
	bool IsAimSensitivityDirty() const;

	/** Checks if the Inverted Mouse user setting is different from current */
	bool IsInvertedYAxisDirty() const;

	/** Triggers a save of this data. */
	void SavePersistentUser();

	/** Lifetime count of kills */
	UPROPERTY()
	int32 Kills;

	/** Lifetime count of deaths */
	UPROPERTY()
	int32 Deaths;

	/** Lifetime count of match wins */
	UPROPERTY()
	int32 Wins;

	/** Lifetime count of match losses */
	UPROPERTY()
	int32 Losses;

	/** Lifetime count of bullets fired */
	UPROPERTY()
	int32 BulletsFired;

	/** Lifetime count of rockets fired */
	UPROPERTY()
	int32 RocketsFired;

	/** how many bots join hosted game */
	UPROPERTY()
	int32 BotsCount;

	/** is recording demos? */
	UPROPERTY()
	bool bIsRecordingDemos;

	/** Holds the gamma correction setting */
	UPROPERTY()
	float Gamma;

	/** Holds the mouse sensitivity */
	UPROPERTY()
	float AimSensitivity;

	/** Is the y axis inverted or not? */
	UPROPERTY()
	bool bInvertedYAxis;

	UPROPERTY()
	bool bVibrationOpt;

private:
	/** Internal.  True if data is changed but hasn't been saved. */
	bool bIsDirty;

	/** The string identifier used to save/load this persistent user. */
	FString SlotName;
	int32 UserIndex;
};