// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/Abilities/VTAbilityTypes.h"
#include "AbilitiesWidget.generated.h"

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UAbilitiesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Notify")
	bool UpdateAbilityCount(EAbilityState NewState);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Notify")
	EAbilityState GetCurAbilityState();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Dispose")
	bool AbilityStateSwitchToStart(EAbilityState OldState);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Dispose")
	bool AbilityStateSwitchToCanUse(EAbilityState OldState);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Dispose")
	bool AbilityStateSwitchToCooldown(EAbilityState OldState);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Dispose")
	bool AbilityStateSwitchToSilence(EAbilityState OldState);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Dispose")
	bool AbilityStateSwitchToClear(EAbilityState OldState);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="Ability")
	UAbilitySystemComponent* GetAbilityComponent(APawn* TargetPawn);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="Ability")
	TArray<float> GetActiveGameplayEffectDurationFromTag(APawn* TargetPawn, FGameplayTagContainer Tags);

	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category="Ability")
	float GetActiveGameplayEffectDurationFromClass(APawn* TargetPawn, const TSubclassOf<UGameplayEffect> EffectClass);
};
