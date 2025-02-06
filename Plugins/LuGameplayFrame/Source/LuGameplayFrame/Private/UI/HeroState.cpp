// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HeroState.h"

#include "Components/PanelWidget.h"


void UHeroState::InitWidget()
{
	if (bIsUseDefaultDataAsset && HeroDataAsset == nullptr)
	{
		HeroDataAsset = UBFLCommon::GetDefaultHeroDataAssetSync();
	}
	UpdateAbilitys();
}

void UHeroState::UpdateAbilitys()
{
	if (HeroTag.IsValid() && HeroDataAsset != nullptr)
	{
		ResetAbilitys();
		FHeroAbilityData HeroAbilityData = HeroDataAsset->GetHeroAbilityData(HeroTag);
		if (HeroAbilityData.IsValid())
		{
			CurHeroAbilities = HeroAbilityData.HeroAbilities;
			for (FGameplayTag AbilityTag : CurHeroAbilities)
			{
				CurAbilityBaseInfos.Add(AbilityTag, HeroAbilityData.GetFAbilityBaseInfo(HeroTag, AbilityTag));
			}
		}
	}

	for (auto& Ability : CurAbilityBaseInfos)
	{
		bool bIsCreateTag = false;
		UAbilitiesWidget* Ptr = nullptr;
		if (Ptr = CreateWidget(Ability.Key, bIsCreateTag); Ptr == nullptr || bIsCreateTag == false)
		{
			UE_LOG(LogTemp, Error, TEXT("%s() Create UAbilitiesWidget failed"), *FString(__FUNCTION__));
			continue;
		}
		CurAbilityWidgets.Add(Ability.Key, Ptr);
	}
	LoadAbilitiesWidgetToPanel();
}

void UHeroState::ResetAbilitys()
{
	if (CurAbilityWidgets.Num() > 0)
	{
		TArray<FGameplayTag> KeyTags;
		CurAbilityWidgets.GetKeys(KeyTags);
		for (int i = 0; i < KeyTags.Num(); i++)
		{
			CurAbilityWidgets[KeyTags[i]]->Reset();
		}
	}

	// HeroTag = FGameplayTag::RequestGameplayTag("None");
	CurAbilityWidgets.Empty();
	CurHeroAbilities.Reset();
	CurAbilityBaseInfos.Empty();
}

UAbilitySystemComponent* UHeroState::GetAbilityComponent() const
{
	if (WeakComponent.IsValid())
	{
		return WeakComponent.Get();
	}
	return nullptr;
}

void UHeroState::LoadAbilitiesWidgetToPanel_Implementation()
{
	if (AbilitysPanel != nullptr)
	{
		for (const auto& Ability : CurAbilityWidgets)
		{
			AbilitysPanel->AddChild(Ability.Value);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("%s() AbilitysPanel is nullptr"), *FString(__FUNCTION__));
	}
}

void UHeroState::SetAbilitiesPanel_Implementation(UPanelWidget* Panel)
{
	if (Panel != nullptr)
	{
		AbilitysPanel = Panel;
	}
}
