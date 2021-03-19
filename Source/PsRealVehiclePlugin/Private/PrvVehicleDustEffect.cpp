// Copyright 2016 Pushkin Studio. All Rights Reserved.

#include "PrvVehicleDustEffect.h"

UPrvVehicleDustEffect::UPrvVehicleDustEffect(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UParticleSystem* UPrvVehicleDustEffect::GetDustFX(EPhysicalSurface SurfaceType, float CurrentSpeed,FVector& Scale)
{
	for (auto DustEffect : DustEffects)
	{
		if (DustEffect.SurfaceType == SurfaceType &&
			CurrentSpeed >= DustEffect.ActivationMinSpeed)
		{
			Scale=DustEffect.Scale;
			return DustEffect.DustFX;
		}
	}

	if (CurrentSpeed >= DefaultMinSpeed)
	{
		Scale=DefaulScale;
		return DefaultFX;
	}

	return nullptr;
}
