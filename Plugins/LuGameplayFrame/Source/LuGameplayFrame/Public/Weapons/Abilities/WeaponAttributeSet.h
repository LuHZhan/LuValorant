// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "WeaponAttributeSet.generated.h"

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
class LUGAMEPLAYFRAME_API UWeaponAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UWeaponAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, Damage)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData FireRate;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, FireRate)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData MagazineSize;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MagazineSize)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info", ReplicatedUsing=OnRep_CurrentAmmo)
	FGameplayAttributeData CurrentAmmo;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, CurrentAmmo)
	UFUNCTION()
	virtual void OnRep_CurrentAmmo(const FGameplayAttributeData& OldCurrentAmmo);

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info", ReplicatedUsing=OnRep_ReserveAmmo)
	FGameplayAttributeData ReserveAmmo;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, ReserveAmmo)
	UFUNCTION()
	virtual void OnRep_ReserveAmmo(const FGameplayAttributeData& OldReserveAmmo);

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData Spread;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, Spread)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData MovingAccuracy;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, MovingAccuracy)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData ZoomLevel;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, ZoomLevel)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData Cost;
	ATTRIBUTE_ACCESSORS(UWeaponAttributeSet, Cost)
};
