// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/VTAbilitySystemComponent.h"
#include "Data/VTHeroDataAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BFLCommon.generated.h"

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UBFLCommon : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GameplayEffect")
	static bool GetAttributeValue(UAbilitySystemComponent* ASC, FGameplayAttribute Attribute, float& BaseValue, float& CurValue);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	static UVTHeroDataAsset* GetDefaultHeroDataAssetSync();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Data")
	static UVTHeroDataAsset* GetDefaultHeroDataAssetAsyn();
};
