// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Heroes/Abilities/VTAbilityStartupGameplayEffect.h"
#include "Characters/Heroes/Abilities/VTAbilityAttributeSet.h"


void UVTAbilityStartupGameplayEffect::PostInitProperties()
{
	Super::PostInitProperties();
	if (Modifiers.IsEmpty())
	{
		AttributeStartupDefaultMap = {
			{UVTAbilityAttributeSet::GetAbilityCurChargesAttribute(), 3.0},
			{UVTAbilityAttributeSet::GetAbilityMaxChargesAttribute(), 3.0},

			{UVTAbilityAttributeSet::GetAbilityCooldownTimeAttribute(), 10.0},
			{UVTAbilityAttributeSet::GetAbilityMaxCooldownTimeAttribute(), 360.0},

			{UVTAbilityAttributeSet::GetAbilityDamageAttribute(), 0.0},
			{UVTAbilityAttributeSet::GetAbilityMaxDamageAttribute(), 1000.0},

		};
		DurationPolicy = EGameplayEffectDurationType::Instant;
		for (const auto Attribute : AttributeStartupDefaultMap)
		{
			FGameplayModifierInfo CurrentAttribute;
			CurrentAttribute.Attribute = Attribute.Key;
			CurrentAttribute.ModifierOp = EGameplayModOp::Override;
			CurrentAttribute.ModifierMagnitude = FScalableFloat{Attribute.Value};
			Modifiers.Add(CurrentAttribute);
		}
	}
}
