// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Abilities/VTWeaponGameplayAbility.h"
#include "Characters/Heroes/VTHeroCharacter.h"


class AVTHeroCharacter;

bool UVTWeaponGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                                  const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (bSourceObjectMustEqualCurrentWeaponToActivate)
	{
		AVTHeroCharacter* Hero = Cast<AVTHeroCharacter>(ActorInfo->AvatarActor);
		if (Hero && Hero->GetCurrentWeapon() && Cast<UObject>(Hero->GetCurrentWeapon()) == GetSourceObject(Handle, ActorInfo))
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
