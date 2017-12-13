// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Sound/SoundNode.h"
#include "SoundNodeLocalPlayer.generated.h"

/**
 * Choose different branch for sounds attached to locally controlled player
 */
UCLASS(hidecategories=Object, editinlinenew)
class USoundNodeLocalPlayer : public USoundNode
{
	GENERATED_UCLASS_BODY()

	// Begin USoundNode interface.
	virtual void ParseNodes( FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances ) override;
	virtual int32 GetMaxChildNodes() const override;
	virtual int32 GetMinChildNodes() const override;
#if WITH_EDITOR
	virtual FText GetInputPinName(int32 PinIndex) const override;
#endif
	// End USoundNode interface.

	static TMap<uint32, bool>& GetLocallyControlledActorCache()
	{
		check(IsInAudioThread());
		return LocallyControlledActorCache;
	}

private:

	static TMap<uint32, bool> LocallyControlledActorCache;
};
