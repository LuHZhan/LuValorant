// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Heroes/Abilities/VTAbilityAttributeSet.h"


UVTAbilityAttributeSet::UVTAbilityAttributeSet()
{
	AbilityType = EAbilityType::None;
}

void UVTAbilityAttributeSet::SetAbilityType(EAbilityType NewType)
{
	AbilityType = NewType;
}

void UVTAbilityAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetAbilityCurChargesAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0, GetAbilityMaxCharges());
	}
	else if (Attribute == GetAbilityCooldownTimeAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0, GetAbilityMaxCooldownTime());
	}
	else if (Attribute == GetAbilityDamageAttribute())
	{
		NewValue = FMath::Clamp<float>(NewValue, 0, GetAbilityMaxDamage());
	}
}
