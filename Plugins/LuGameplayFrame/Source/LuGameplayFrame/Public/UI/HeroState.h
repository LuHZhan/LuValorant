// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitiesWidget.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "HeroState.generated.h"

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UHeroState : public UUserWidget
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TMap<FGameplayTag, UAbilitiesWidget*> GetAbilityWidget(FGameplayTag Tag) { return Abilitys; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAbilitySystemComponent* GetAbilityComponent() const
	{
		if (WeakComponent.IsValid())
		{
			return WeakComponent.Get();
		}
		return nullptr;
	}

	UFUNCTION(BlueprintCallable)
	void SetAbilityComponent(UAbilitySystemComponent* Component)
	{
		WeakComponent = Component;
	}

	

private:
	UPROPERTY()
	TMap<FGameplayTag, UAbilitiesWidget*> Abilitys;

	TWeakObjectPtr<UAbilitySystemComponent> WeakComponent;
};
