// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "ShooterGame.h"
#include "Sound/SoundNodeLocalPlayer.h"
#include "SoundDefinitions.h"

#define LOCTEXT_NAMESPACE "SoundNodeLocalPlayer"

TMap<uint32, bool> USoundNodeLocalPlayer::LocallyControlledActorCache;

USoundNodeLocalPlayer::USoundNodeLocalPlayer(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void USoundNodeLocalPlayer::ParseNodes(FAudioDevice* AudioDevice, const UPTRINT NodeWaveInstanceHash, FActiveSound& ActiveSound, const FSoundParseParameters& ParseParams, TArray<FWaveInstance*>& WaveInstances)
{
	bool bLocallyControlled = false;
	if (bool* LocallyControlledPtr = LocallyControlledActorCache.Find(ActiveSound.GetOwnerID()))
	{
		bLocallyControlled = *LocallyControlledPtr;
	}

	const int32 PlayIndex = bLocallyControlled ? 0 : 1;

	if (PlayIndex < ChildNodes.Num() && ChildNodes[PlayIndex])
	{
		ChildNodes[PlayIndex]->ParseNodes(AudioDevice, GetNodeWaveInstanceHash(NodeWaveInstanceHash, ChildNodes[PlayIndex], PlayIndex), ActiveSound, ParseParams, WaveInstances);
	}
}

#if WITH_EDITOR
FText USoundNodeLocalPlayer::GetInputPinName(int32 PinIndex) const
{
	return (PinIndex == 0) ? LOCTEXT("LocalPlayerLabel", "Local") : LOCTEXT("RemotePlayerLabel","Remote");
}
#endif

int32 USoundNodeLocalPlayer::GetMaxChildNodes() const
{
	return 2;
}

int32 USoundNodeLocalPlayer::GetMinChildNodes() const
{
	return 2;
}

#undef LOCTEXT_NAMESPACE