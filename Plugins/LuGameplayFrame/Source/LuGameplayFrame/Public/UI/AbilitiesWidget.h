// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Characters/Abilities/VTAbilityTypes.h"
#include "Data/VTHeroDataAsset.h"
#include "AbilitiesWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityTagAddedRemoved, FGameplayTag, Tag);

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UAbilitiesWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void ReInitWidget();

	UFUNCTION(BlueprintCallable, Category="Notify")
	void TagChanged(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Notify")
	void OnTranspondToStateSwitch(const FGameplayTag Tag, int32 NewCount);

	UFUNCTION(BlueprintCallable)
	void LoadAbilitiesStateTag();
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void LoadListenAbilityTags();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Notify")
	bool UpdateAbilityState(EAbilityState NewState);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Dispose")
	void Reset();

	UPROPERTY(BlueprintAssignable)
	FOnAbilityTagAddedRemoved OnGameplayTagAdded;

	UPROPERTY(BlueprintAssignable)
	FOnAbilityTagAddedRemoved OnGameplayTagAddedRemoved;

	// --------------------------------
	// EAbilityState Functions
	// --------------------------------

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Notify")
	EAbilityState GetCurAbilityState();

	// TODO: 这里可以优化成状态机模式
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

	// ================================
	// EAbilityState Functions
	// ================================

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ability")
	UAbilitySystemComponent* SetAbilityComponentFromPawn(APawn* TargetPawn);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ability")
	UAbilitySystemComponent* SetAbilityComponent(UAbilitySystemComponent* Ptr);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ability")
	UAbilitySystemComponent* GetWeakAbilitySystemComponent();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ability")
	TArray<float> GetActiveGameplayEffectDurationFromTag(APawn* TargetPawn, FGameplayTagContainer Tags);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Ability")
	float GetActiveGameplayEffectDurationFromClass(APawn* TargetPawn, TSubclassOf<UGameplayEffect> EffectClass);

	// --------------------------------
	// Ability Variables
	// --------------------------------

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Ability")
	TMap<FGameplayTag, TEnumAsByte<EAbilityState>> AbilitiesTagWithState;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Ability", meta=(ExposeOnSpawn="true"))
	FAbilityBaseInfo AbilityInfo;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Ability")
	FGameplayTagContainer ListenAbilityTags;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Ability", meta=(ExposeOnSpawn="true"))
	FGameplayTag AbilityTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Ability")
	FGameplayTag CooldownTag /** = FGameplayTag::RequestGameplayTag(FName("Cooldown")) */;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Ability")
	FGameplayTag SilenceTag /** = FGameplayTag::RequestGameplayTag(FName("DeBuff.Silence")) */;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Ability")
	FGameplayTag ClearTag /** = FGameplayTag::RequestGameplayTag(FName("Clear")) */;

	// ================================
	// Ability Variables
	// ================================

protected:
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UAbilitySystemComponent> WeakAbilitySystemComponentPtr;
};
