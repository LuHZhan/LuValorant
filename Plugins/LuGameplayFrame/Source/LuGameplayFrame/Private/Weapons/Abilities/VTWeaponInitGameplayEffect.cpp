// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Abilities/VTWeaponInitGameplayEffect.h"

#include "Weapons/Abilities/VTWeaponAttributeSetBase.h"


UVTWeaponInitGameplayEffect::UVTWeaponInitGameplayEffect()
{
	AttributeCurrentDataMap = {
		{UVTWeaponAttributeSetBase::GetBodyDamageAttribute(), 40.0},
		{UVTWeaponAttributeSetBase::GetHeadDamageAttribute(), 100.0},

		{UVTWeaponAttributeSetBase::GetFireRateAttribute(), 10.0},

		{UVTWeaponAttributeSetBase::GetMagazineSizeAttribute(), 25.0},

		{UVTWeaponAttributeSetBase::GetCurrentAmmoAttribute(), 25.0},
		{UVTWeaponAttributeSetBase::GetReserveAmmoAttribute(), 75.0},

		{UVTWeaponAttributeSetBase::GetAimZoomLevelAttribute(), 2.0},

		{UVTWeaponAttributeSetBase::GetBaseSpreadAttribute(), 1.0},
		{UVTWeaponAttributeSetBase::GetAimSpreadModAttribute(), 0.15},
		{UVTWeaponAttributeSetBase::GetSpreadIncrementAttribute(), 1.2},
		{UVTWeaponAttributeSetBase::GetMaxSpreadAttribute(), 10.0},

		{UVTWeaponAttributeSetBase::GetCostAttribute(), 1000.0},
		{UVTWeaponAttributeSetBase::GetFireAmmoCountAttribute(), 1.0},
	};

	DurationPolicy = EGameplayEffectDurationType::Instant;

	for (const auto Attribute : AttributeCurrentDataMap)
	{
		FGameplayModifierInfo CurrentAttribute;
		CurrentAttribute.Attribute = Attribute.Key;
		CurrentAttribute.ModifierOp = EGameplayModOp::Override;
		CurrentAttribute.ModifierMagnitude = FScalableFloat{Attribute.Value};
		Modifiers.Add(CurrentAttribute);
	}
}

void UVTWeaponInitGameplayEffect::PostInitProperties()
{
	Super::PostInitProperties();

	

	// OnFinishedInitProperties();
}
