// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "VTWeaponAttributeSetBase.generated.h"

// Uses macros from AttributeSet.h
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTWeaponAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public:
	UVTWeaponAttributeSetBase();

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// using PreAttributeChangeFunc = void(*)(const FGameplayAttribute&, float&);
	// static TMap<FGameplayAttribute, PreAttributeChangeFunc> MessageHandlerMap;

	// ---------------- 枪械伤害 ----------------

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData BodyDamage;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, BodyDamage)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData HeadDamage;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, HeadDamage)

	// UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	// FGameplayAttributeData CurrentDamage;
	// ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, CurrentDamage)

	// ================ 枪械伤害 ================

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData FireRate;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, FireRate)

	// ---------------- 弹匣情况 ----------------

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData MagazineSize;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, MagazineSize)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info", ReplicatedUsing=OnRep_CurrentAmmo)
	FGameplayAttributeData CurrentAmmo;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, CurrentAmmo)
	UFUNCTION()
	virtual void OnRep_CurrentAmmo(const FGameplayAttributeData& OldCurrentAmmo);

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info", ReplicatedUsing=OnRep_ReserveAmmo)
	FGameplayAttributeData ReserveAmmo;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, ReserveAmmo)
	UFUNCTION()
	virtual void OnRep_ReserveAmmo(const FGameplayAttributeData& OldReserveAmmo);

	// ================ 弹匣情况 ================

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData AimZoomLevel;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, AimZoomLevel)

	// ---------------- 枪械散布 ----------------

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData BaseSpread;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, BaseSpread)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData AimSpreadMod;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, AimSpreadMod)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData SpreadIncrement;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, SpreadIncrement)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData MaxSpread;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, MaxSpread)

	// ================ 枪械散布 ================

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData Cost;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, Cost)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData FireAmmoCount;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, FireAmmoCount)

	/** Const Variable */

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	float MinDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	float MaxDamage = 1000.0f;
};
