// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "Data/VTHeroDataAsset.h"
#include "VTAbilityAttributeSet.generated.h"

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
class LUGAMEPLAYFRAME_API UVTAbilityAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UVTAbilityAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAttributeData AbilityCurCharges;
	ATTRIBUTE_ACCESSORS(UVTAbilityAttributeSet, AbilityCurCharges)

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAttributeData AbilityMaxCharges;
	ATTRIBUTE_ACCESSORS(UVTAbilityAttributeSet, AbilityMaxCharges)

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAttributeData AbilityCooldownTime;
	ATTRIBUTE_ACCESSORS(UVTAbilityAttributeSet, AbilityCooldownTime)

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAttributeData AbilityMaxCooldownTime;
	ATTRIBUTE_ACCESSORS(UVTAbilityAttributeSet, AbilityMaxCooldownTime)

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAttributeData AbilityDamage;
	ATTRIBUTE_ACCESSORS(UVTAbilityAttributeSet, AbilityDamage)

	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayAttributeData AbilityMaxDamage;
	ATTRIBUTE_ACCESSORS(UVTAbilityAttributeSet, AbilityMaxDamage)
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	EAbilityType AbilityType;
	
	void SetAbilityType(EAbilityType NewType);
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
};
