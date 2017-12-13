// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterPersistentUser.h"
#include "ShooterLocalPlayer.generated.h"

UCLASS(BlueprintType, config=Engine, transient)
class UShooterLocalPlayer : public ULocalPlayer
{
	GENERATED_UCLASS_BODY()

public:

	virtual void SetControllerId(int32 NewControllerId) override;

	UFUNCTION(BlueprintCallable)
	virtual FString GetNickname() const;

	UFUNCTION(BlueprintCallable)
	class UShooterPersistentUser* GetPersistentUser() const;
	
	/** Initializes the PersistentUser */
	void LoadPersistentUser();

private:
	/** Persistent user data stored between sessions (i.e. the user's savegame) */
	UPROPERTY()
	class UShooterPersistentUser* PersistentUser;
};



