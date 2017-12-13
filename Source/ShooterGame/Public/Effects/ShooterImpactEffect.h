// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ShooterTypes.h"
#include "ShooterImpactEffect.generated.h"

//
// Spawnable effect for weapon hit impact - NOT replicated to clients
// Each impact type should be defined as separate blueprint
//
UCLASS(Abstract, Blueprintable)
class AShooterImpactEffect : public AActor
{
	GENERATED_UCLASS_BODY()

	/** default impact FX used when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category=Defaults)
	UParticleSystem* DefaultFX;

	/** impact FX on concrete */
	UPROPERTY(EditDefaultsOnly, Category=Visual)
	UParticleSystem* ConcreteFX;

	/** impact FX on dirt */
	UPROPERTY(EditDefaultsOnly, Category=Visual)
	UParticleSystem* DirtFX;

	/** impact FX on water */
	UPROPERTY(EditDefaultsOnly, Category=Visual)
	UParticleSystem* WaterFX;

	/** impact FX on metal */
	UPROPERTY(EditDefaultsOnly, Category=Visual)
	UParticleSystem* MetalFX;

	/** impact FX on wood */
	UPROPERTY(EditDefaultsOnly, Category=Visual)
	UParticleSystem* WoodFX;

	/** impact FX on glass */
	UPROPERTY(EditDefaultsOnly, Category=Visual)
	UParticleSystem* GlassFX;

	/** impact FX on grass */
	UPROPERTY(EditDefaultsOnly, Category=Visual)
	UParticleSystem* GrassFX;

	/** impact FX on flesh */
	UPROPERTY(EditDefaultsOnly, Category=Visual)
	UParticleSystem* FleshFX;

	/** default impact sound used when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category=Defaults)
	USoundCue* DefaultSound;

	/** impact FX on concrete */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* ConcreteSound;

	/** impact FX on dirt */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* DirtSound;

	/** impact FX on water */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* WaterSound;

	/** impact FX on metal */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* MetalSound;

	/** impact FX on wood */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* WoodSound;

	/** impact FX on glass */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* GlassSound;

	/** impact FX on grass */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* GrassSound;

	/** impact FX on flesh */
	UPROPERTY(EditDefaultsOnly, Category=Sound)
	USoundCue* FleshSound;

	/** default decal when material specific override doesn't exist */
	UPROPERTY(EditDefaultsOnly, Category=Defaults)
	struct FDecalData DefaultDecal;

	/** surface data for spawning */
	UPROPERTY(BlueprintReadOnly, Category=Surface)
	FHitResult SurfaceHit;

	/** spawn effect */
	virtual void PostInitializeComponents() override;

protected:

	/** get FX for material type */
	UParticleSystem* GetImpactFX(TEnumAsByte<EPhysicalSurface> SurfaceType) const;

	/** get sound for material type */
	USoundCue* GetImpactSound(TEnumAsByte<EPhysicalSurface> SurfaceType) const;
};
