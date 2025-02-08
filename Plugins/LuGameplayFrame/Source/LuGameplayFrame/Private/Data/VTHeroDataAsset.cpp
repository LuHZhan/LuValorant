// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/VTHeroDataAsset.h"


FAbilityInfo FHeroAbilityData::GetFAbilityInfo(const FGameplayTag TargetAbilityTag) const
{
	if (HeroAbilities.HasTag(TargetAbilityTag))
	{
		return AbilityInfoMap[TargetAbilityTag];
	}
	return FAbilityInfo{};
}

FAbilityBaseInfo FHeroAbilityData::GetFAbilityBaseInfo(const FGameplayTag TargetHeroTag, const FGameplayTag TargetAbilityTag) const
{
	if (TargetHeroTag == HeroTag && HeroAbilities.HasTag(TargetAbilityTag))
	{
		return GetFAbilityInfo(TargetAbilityTag).BaseInfo;
	}
	return FAbilityBaseInfo{};
}

FAbilityPerformanceInfo FHeroAbilityData::GetFAbilityPerformanceInfo(const FGameplayTag TargetHeroTag, const FGameplayTag TargetAbilityTag) const
{
	if (TargetHeroTag == HeroTag && HeroAbilities.HasTag(TargetAbilityTag))
	{
		return GetFAbilityInfo(TargetAbilityTag).PerformanceInfo;
	}
	return FAbilityPerformanceInfo{};
}

bool FHeroAbilityData::IsValid() const
{
	return HeroTag.IsValid();
}

FHeroAbilityData UVTHeroDataAsset::GetHeroAbilityData(const FGameplayTag HeroTag)
{
	for (FHeroAbilityData Data : HeroAbilityData)
	{
		if (Data.HeroTag == HeroTag)
		{
			return Data;
		}
	}
	return FHeroAbilityData{};
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

FAbilityPerformanceInfo UVTHeroDataAsset::GetPerformanceInfo(FGameplayTag HeroTag, FGameplayTag AbilityTag) const
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

TArray<FAbilityPerformanceInfo> UVTHeroDataAsset::GetPerformanceInfoArray(const FGameplayTag AbilityTag) const
{
	TArray<FAbilityPerformanceInfo> Result = {};
	for (const auto Data : HeroAbilityData)
	{
		for (const auto Ability : Data.HeroAbilities)
		{
			Result.Add(Data.GetFAbilityInfo(Ability).PerformanceInfo);
		}
	}
	return Result;
}

void UVTHeroDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	for (auto& Ability : HeroAbilityData)
	{
		for (TTuple<FGameplayTag, FAbilityInfo>& Info : Ability.AbilityInfoMap)
		{
			Info.Value.PerformanceInfo.IsAdvancedSettingsEnabled();
			Info.Value.BaseInfo.IsAdvancedSettingsEnabled();
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
