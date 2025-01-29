// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/VTGameplayAbility.h"
// #include "Weapons/Interface/VTWeaponInterface.h"
#include "VTWeaponGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTWeaponGameplayAbility : public UVTGameplayAbility
{
	GENERATED_BODY()

public:
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	                                const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;

	// virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	// virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;


	

	/**
	 * 限制只有当前装备武器等于SourceObject时才能激活
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bSourceObjectMustEqualCurrentWeaponToActivate;

	// ---------------- IVTWeaponInterface ----------------

	UFUNCTION(BlueprintCallable, Category="IVTWeaponInterface")
	virtual AVTWeapon* GetWeapon() const override;

	// ================ IVTWeaponInterface ================
};
