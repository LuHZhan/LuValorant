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

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData BodyDamage;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, BodyDamage)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData HeadDamage;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, HeadDamage)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData FireRate;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, FireRate)

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

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData Spread;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, Spread)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData MovingAccuracy;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, MovingAccuracy)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData ZoomLevel;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, ZoomLevel)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	FGameplayAttributeData Cost;
	ATTRIBUTE_ACCESSORS(UVTWeaponAttributeSetBase, Cost)

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	float MinDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Weapon|Info")
	float MaxDamage = 1000.0f;
};
