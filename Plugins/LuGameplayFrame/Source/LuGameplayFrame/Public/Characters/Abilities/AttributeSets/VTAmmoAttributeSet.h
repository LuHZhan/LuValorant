// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "VTAmmoAttributeSet.generated.h"

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
class GASSHOOTER_API UVTAmmoAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UVTAmmoAttributeSet();

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_RifleReserveAmmo)
	FGameplayAttributeData RifleReserveAmmo;
	ATTRIBUTE_ACCESSORS(UVTAmmoAttributeSet, RifleReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxRifleReserveAmmo)
	FGameplayAttributeData MaxRifleReserveAmmo;
	ATTRIBUTE_ACCESSORS(UVTAmmoAttributeSet, MaxRifleReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_RocketReserveAmmo)
	FGameplayAttributeData RocketReserveAmmo;
	ATTRIBUTE_ACCESSORS(UVTAmmoAttributeSet, RocketReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxRocketReserveAmmo)
	FGameplayAttributeData MaxRocketReserveAmmo;
	ATTRIBUTE_ACCESSORS(UVTAmmoAttributeSet, MaxRocketReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_ShotgunReserveAmmo)
	FGameplayAttributeData ShotgunReserveAmmo;
	ATTRIBUTE_ACCESSORS(UVTAmmoAttributeSet, ShotgunReserveAmmo)

	UPROPERTY(BlueprintReadOnly, Category = "Ammo", ReplicatedUsing = OnRep_MaxShotgunReserveAmmo)
	FGameplayAttributeData MaxShotgunReserveAmmo;
	ATTRIBUTE_ACCESSORS(UVTAmmoAttributeSet, MaxShotgunReserveAmmo)

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	static FGameplayAttribute GetReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag);
	static FGameplayAttribute GetMaxReserveAmmoAttributeFromTag(FGameplayTag& PrimaryAmmoTag);

protected:
	// Cache tags
	FGameplayTag RifleAmmoTag;
	FGameplayTag RocketAmmoTag;
	FGameplayTag ShotgunAmmoTag;

	// 辅助函数，当与属性相关的最大属性变化时，按比例调整属性的值。（即当max血量增加时，血量增加的数量与之前保持相同的百分比）
	void AdjustAttributeForMaxChange(FGameplayAttributeData& AffectedAttribute, const FGameplayAttributeData& MaxAttribute, float NewMaxValue, const FGameplayAttribute& AffectedAttributeProperty);

	/**
	* 这些OnRep函数的存在是为了确保在复制期间正确地同步ability
	**/
	
	UFUNCTION()
	virtual void OnRep_RifleReserveAmmo(const FGameplayAttributeData& OldRifleReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxRifleReserveAmmo(const FGameplayAttributeData& OldMaxRifleReserveAmmo);

	UFUNCTION()
	virtual void OnRep_RocketReserveAmmo(const FGameplayAttributeData& OldRocketReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxRocketReserveAmmo(const FGameplayAttributeData& OldMaxRocketReserveAmmo);

	UFUNCTION()
	virtual void OnRep_ShotgunReserveAmmo(const FGameplayAttributeData& OldShotgunReserveAmmo);

	UFUNCTION()
	virtual void OnRep_MaxShotgunReserveAmmo(const FGameplayAttributeData& OldMaxShotgunReserveAmmo);
};
