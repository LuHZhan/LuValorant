// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Abilities/VTWeaponAttributeSetBase.h"

UVTWeaponAttributeSetBase::UVTWeaponAttributeSetBase()
{
	
}

// using PreAttributeChangeFunc = void(*)(const FGameplayAttribute&, float&);
// TMap<FGameplayAttribute, PreAttributeChangeFunc> UWeaponAttributeSet::MessageHandlerMap = {
//
// };
void UVTWeaponAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	if (Attribute == GetBodyDamageAttribute())
	{
		SetBodyDamage(FMath::Clamp(NewValue, MinDamage, MaxDamage));
	}
	else if (Attribute == GetHeadDamageAttribute())
	{
		SetHeadDamage(FMath::Clamp(NewValue, MinDamage, MaxDamage));
	}
}

void UVTWeaponAttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

bool UVTWeaponAttributeSetBase::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	return Super::PreGameplayEffectExecute(Data);
}

void UVTWeaponAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
}

void UVTWeaponAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UVTWeaponAttributeSetBase::OnRep_CurrentAmmo(const FGameplayAttributeData& OldCurrentAmmo)
{
	
}

void UVTWeaponAttributeSetBase::OnRep_ReserveAmmo(const FGameplayAttributeData& OldReserveAmmo)
{
	
}
