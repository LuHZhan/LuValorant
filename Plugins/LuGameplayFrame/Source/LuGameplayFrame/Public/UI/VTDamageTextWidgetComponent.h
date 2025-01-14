// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "GameplayTagContainer.h"
#include "VTDamageTextWidgetComponent.generated.h"

/**
 * 伤害浮动数字
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTDamageTextWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetDamageText(float Damage, const FGameplayTagContainer& Tags);
};
