// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/VTHeroDataAsset.h"


FAbilityBaseInfo FHeroAbilityData::GetFAbilityBaseInfo(const FGameplayTag TargetHeroTag, const FGameplayTag TargetAbilityTag)
{
	if (TargetHeroTag == HeroTag && HeroAbilities.HasTag(TargetAbilityTag))
	{
		return AbilityInfoMap[TargetAbilityTag].BaseInfo;
	}
	return FAbilityBaseInfo{};
}

FAbilityPerformanceInfo FHeroAbilityData::GetFAbilityPerformanceInfo(const FGameplayTag TargetHeroTag, const FGameplayTag TargetAbilityTag)
{
	if (TargetHeroTag == HeroTag && HeroAbilities.HasTag(TargetAbilityTag))
	{
		return AbilityInfoMap[TargetAbilityTag].PerformanceInfo;
	}
	return FAbilityPerformanceInfo{};
}

FAbilityBaseInfo UVTHeroDataAsset::GetBaseInfo(FGameplayTag HeroTag, FGameplayTag AbilityTag)
{
	if (HeroAbilityData.Num() > 0)
	{
		for (FHeroAbilityData Data : HeroAbilityData)
		{
			return Data.GetFAbilityBaseInfo(HeroTag, AbilityTag);
		}
	}
	return FAbilityBaseInfo{};
}

FAbilityPerformanceInfo UVTHeroDataAsset::GetPerformanceInfo(FGameplayTag HeroTag, FGameplayTag AbilityTag)
{
	if (HeroAbilityData.Num() > 0)
	{
		for (FHeroAbilityData Data : HeroAbilityData)
		{
			return Data.GetFAbilityPerformanceInfo(HeroTag, AbilityTag);
		}
	}
	return FAbilityPerformanceInfo{};
}

void UVTHeroDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	for (auto& Ability : HeroAbilityData)
	{
		for (TTuple<FGameplayTag, FAbilityInfo>& Info : Ability.AbilityInfoMap)
		{
			Info.Value.PerformanceInfo.IsAdvancedSettingsEnabled();
		}

		for (FGameplayTag Tag : Ability.HeroAbilities)
		{
			if (!Ability.AbilityInfoMap.Contains(Tag))
			{
				Ability.AbilityInfoMap.Add(Tag, FAbilityInfo{}); // Add default value
			}
		}
	}
}
