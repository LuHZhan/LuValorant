// Copyright 2024 Dan Kestranek.


#include "Characters/Abilities/VTAbilitySystemGlobals.h"
#include "..\..\..\Public\Characters\Abilities\VTGameplayEffectTypes.h"

UVTAbilitySystemGlobals::UVTAbilitySystemGlobals()
{

}

FGameplayEffectContext* UVTAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FVTGameplayEffectContext();
}

void UVTAbilitySystemGlobals::InitGlobalTags()
{
	Super::InitGlobalTags();

	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
	InteractingRemovalTag = FGameplayTag::RequestGameplayTag("State.InteractingRemoval");
}
