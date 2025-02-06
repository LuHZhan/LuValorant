// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/BFLCommon.h"

#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"


bool UBFLCommon::GetAttributeValue(UAbilitySystemComponent* ASC, FGameplayAttribute Attribute, float& BaseValue, float& CurValue)
{
	bool Tag = false;
	if (ASC != nullptr)
	{
		Tag = true;
		BaseValue = ASC->GetNumericAttributeBase(Attribute);
		CurValue = ASC->GetNumericAttribute(Attribute);
	}
	return Tag;
}

UVTHeroDataAsset* UBFLCommon::GetDefaultHeroDataAssetSync()
{
	const FSoftObjectPath Path(TEXT("/LuGameplayFrame/Data/DA_HeroData.DA_HeroData"));
	static UVTHeroDataAsset* Asset = nullptr;
	if (Asset == nullptr)
	{
		Asset = Cast<UVTHeroDataAsset>(StaticLoadObject(UDataAsset::StaticClass(), nullptr, *Path.ToString()));
		if (!Asset)
		{
			UE_LOG(LogTemp, Error, TEXT("%s() Failed to find Object. If it was moved, please update the reference location in C++."), *FString(__FUNCTION__));
		}
	}
	return Asset;
}

UVTHeroDataAsset* UBFLCommon::GetDefaultHeroDataAssetAsyn()
{
	const FSoftObjectPath Path(TEXT("/LuGameplayFrame/Data/DA_HeroData.DA_HeroData"));
	UVTHeroDataAsset* Asset = nullptr;

	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	Streamable.RequestAsyncLoad(
		Path,
		FStreamableDelegate::CreateLambda([Path,&Asset]()
		{
			Asset = Cast<UVTHeroDataAsset>(Path.ResolveObject());
			if (Asset)
			{
				UE_LOG(LogTemp, Log, TEXT("DataAsset loaded: %s"), *GetNameSafe(Asset));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load DataAsset from path: %s"), *Path.ToString());
			}
		}), FStreamableManager::AsyncLoadHighPriority);

	return Asset;
}
