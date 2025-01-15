// Copyright 2024 Dan Kestranek.


#include "Characters/Abilities/AttributeSets/VTAmmoAttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

UVTAmmoAttributeSet::UVTAmmoAttributeSet()
{
	RifleAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle"));
	RocketAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rocket"));
	ShotgunAmmoTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Shotgun"));
}

void UVTAmmoAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UVTAmmoAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (Data.EvaluatedData.Attribute == GetRifleReserveAmmoAttribute())
	{
		float Ammo = GetRifleReserveAmmo();
		SetRifleReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxRifleReserveAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetRocketReserveAmmoAttribute())
	{
		float Ammo = GetRocketReserveAmmo();
		SetRocketReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxRocketReserveAmmo()));
	}
	else if (Data.EvaluatedData.Attribute == GetShotgunReserveAmmoAttribute())
	{
		float Ammo = GetShotgunReserveAmmo();
		SetShotgunReserveAmmo(FMath::Clamp<float>(Ammo, 0, GetMaxShotgunReserveAmmo()));
	}
}

void UVTAmmoAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UVTAmmoAttributeSet, RifleReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVTAmmoAttributeSet, MaxRifleReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVTAmmoAttributeSet, RocketReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVTAmmoAttributeSet, MaxRocketReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVTAmmoAttributeSet, ShotgunReserveAmmo, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UVTAmmoAttributeSet, MaxShotgunReserveAmmo, COND_None, REPNOTIFY_Always);
}

FGameplayAttribute UVTAmmoAttributeSet::GetReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag)
{
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle")))
	{
		return GetRifleReserveAmmoAttribute();
	}
	else if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rocket")))
	{
		return GetRocketReserveAmmoAttribute();
	}
	else if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Shotgun")))
	{
		return GetShotgunReserveAmmoAttribute();
	}

	return FGameplayAttribute();
}

FGameplayAttribute UVTAmmoAttributeSet::GetMaxReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag)
{
	if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rifle")))
	{
		return GetMaxRifleReserveAmmoAttribute();
	}
	else if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Rocket")))
	{
		return GetMaxRocketReserveAmmoAttribute();
	}
	else if (PrimaryAmmoTag == FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.Shotgun")))
	{
		return GetMaxShotgunReserveAmmoAttribute();
	}

	return FGameplayAttribute();
}

void UVTAmmoAttributeSet::AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty)
{
	UAbilitySystemComponent* AbilityComp = GetOwningAbilitySystemComponent();
	const float CurrentMaxValue = MaxAttribute.GetCurrentValue();
	if (!FMath::IsNearlyEqual(CurrentMaxValue, NewMaxValue) && AbilityComp)
	{
		// Change current value to maintain the current Val / Max percent
		const float CurrentValue = AffectedAttribute.GetCurrentValue();
		float NewDelta = (CurrentMaxValue > 0.f) ? (CurrentValue * NewMaxValue / CurrentMaxValue) - CurrentValue : NewMaxValue;

		AbilityComp->ApplyModToAttributeUnsafe(AffectedAttributeProperty, EGameplayModOp::Additive, NewDelta);
	}
}

void UVTAmmoAttributeSet::OnRep_RifleReserveAmmo(const FGameplayAttributeData& OldRifleReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVTAmmoAttributeSet, RifleReserveAmmo, OldRifleReserveAmmo);
}

void UVTAmmoAttributeSet::OnRep_MaxRifleReserveAmmo(const FGameplayAttributeData& OldMaxRifleReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVTAmmoAttributeSet, MaxRifleReserveAmmo, OldMaxRifleReserveAmmo);
}

void UVTAmmoAttributeSet::OnRep_RocketReserveAmmo(const FGameplayAttributeData& OldRocketReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVTAmmoAttributeSet, RocketReserveAmmo, OldRocketReserveAmmo);
}

void UVTAmmoAttributeSet::OnRep_MaxRocketReserveAmmo(const FGameplayAttributeData& OldMaxRocketReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVTAmmoAttributeSet, MaxRocketReserveAmmo, OldMaxRocketReserveAmmo);
}

void UVTAmmoAttributeSet::OnRep_ShotgunReserveAmmo(const FGameplayAttributeData& OldShotgunReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVTAmmoAttributeSet, ShotgunReserveAmmo, OldShotgunReserveAmmo);
}

void UVTAmmoAttributeSet::OnRep_MaxShotgunReserveAmmo(const FGameplayAttributeData& OldMaxShotgunReserveAmmo)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UVTAmmoAttributeSet, MaxShotgunReserveAmmo, OldMaxShotgunReserveAmmo);
}
