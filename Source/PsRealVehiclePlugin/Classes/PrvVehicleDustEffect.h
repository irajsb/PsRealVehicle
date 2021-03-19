// Copyright 2016 Pushkin Studio. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"

#include "PrvVehicleDustEffect.generated.h"

USTRUCT(BlueprintType)
struct FDustInfo
{
	GENERATED_USTRUCT_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly)
	TEnumAsByte<EPhysicalSurface> SurfaceType;
	UPROPERTY(EditDefaultsOnly)
	FVector Scale=FVector(1);
	/** Cm/s */
	UPROPERTY(EditDefaultsOnly)
	float ActivationMinSpeed;

	/** */
	UPROPERTY(EditDefaultsOnly, Category = Effect)
	UParticleSystem* DustFX;

	FDustInfo()
	{
		SurfaceType = EPhysicalSurface::SurfaceType_Default;
		ActivationMinSpeed = 300.f;
		DustFX = nullptr;
	}
};

/*
 * Collection of dust effects shown under the wheels
 */
UCLASS()
class UPrvVehicleDustEffect : public UDataAsset
{
	GENERATED_UCLASS_BODY()

	/** */
	UPROPERTY(EditDefaultsOnly)
	UParticleSystem* DefaultFX;

	UPROPERTY(EditDefaultsOnly)
	FVector DefaulScale=FVector(1);
	/** Cm/s */
	UPROPERTY(EditDefaultsOnly)
	float DefaultMinSpeed;

	/** */
	UPROPERTY(EditDefaultsOnly)
	TArray<FDustInfo> DustEffects;

	/** Determine correct FX */
	UParticleSystem* GetDustFX(EPhysicalSurface SurfaceType, float TargetSpeed,FVector& Scale);

	UPrvVehicleDustEffect()
	{
		DefaultFX = nullptr;
		DefaultMinSpeed = 0.f;
	}
};
