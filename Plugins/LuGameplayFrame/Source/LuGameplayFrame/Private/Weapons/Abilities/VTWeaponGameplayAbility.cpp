// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Abilities/VTWeaponGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Characters/Abilities/VTAbilitySystemComponent.h"
#include "Weapons/VTWeapon.h"
#include "Characters/Heroes/VTHeroCharacter.h"


bool UVTWeaponGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                  const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (bSourceObjectMustEqualCurrentWeaponToActivate)
	{
		if (const AVTHeroCharacter* Hero = Cast<AVTHeroCharacter>(ActorInfo->AvatarActor);
			Hero && Hero->GetCurrentWeapon() && Cast<UObject>(Hero->GetCurrentWeapon()) == GetSourceObject(Handle, ActorInfo))
		{
			return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
		}
		else
		{
			return false;
		}
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

bool UVTWeaponGameplayAbility::IsHitTargetBodyName(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const int Index, const FName& BodyName)
{
	if (const FHitResult Hit = UAbilitySystemBlueprintLibrary::GetHitResultFromTargetData(TargetDataHandle, Index); Hit.IsValidBlockingHit())
	{
		return Hit.BoneName == BodyName;
	}
	return false;
}


AVTWeapon* UVTWeaponGameplayAbility::GetWeapon() const
{
	if (UObject* Object = GetCurrentSourceObject(); Object != nullptr)
	{
		return Cast<AVTWeapon>(Object);
	}
	return nullptr;
}
