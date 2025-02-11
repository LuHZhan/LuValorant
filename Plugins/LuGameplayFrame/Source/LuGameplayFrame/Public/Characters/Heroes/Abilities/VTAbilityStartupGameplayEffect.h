// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "VTAbilityStartupGameplayEffect.generated.h"

/**
 * 
 */
UCLASS(DisplayName="AbilityStartupGameplayEffect")
class LUGAMEPLAYFRAME_API UVTAbilityStartupGameplayEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	// UVTAbilityStartupGameplayEffect();
	virtual void PostInitProperties() override;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="StartupData")
	TMap<FGameplayAttribute, float> AttributeStartupDefaultMap;
};
