// Copyright 2024 Dan Kestranek.


#include "VTEngineSubsystem.h"
#include "AbilitySystemGlobals.h"

void UVTEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAbilitySystemGlobals::Get().InitGlobalData();
}
