// Copyright 2024 Dan Kestranek.


#include "..\..\..\Public\Characters\Abilities\VTGameplayEffectTypes.h"

bool FVTGameplayEffectContext::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	return Super::NetSerialize(Ar, Map, bOutSuccess) && TargetData.NetSerialize(Ar, Map, bOutSuccess);
}
