// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitiesWidget.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"

#include "Data/VTHeroDataAsset.h"
#include "Library/BFLCommon.h"

#include "HeroState.generated.h"

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UHeroState : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	virtual void InitWidget();

	void UpdateAbilitys();
	void ResetAbilitys();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	UAbilitiesWidget* CreateWidget(FGameplayTag AbilityTag, bool& IsCreate);

	UFUNCTION(BlueprintCallable,BlueprintNativeEvent)
	void LoadAbilitiesWidgetToPanel();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetAbilitiesPanel(UPanelWidget* Panel);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TMap<FGameplayTag, UAbilitiesWidget*> GetAbilityWidget(FGameplayTag Tag) { return CurAbilityWidgets; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UAbilitySystemComponent* GetAbilityComponent() const;

	UFUNCTION(BlueprintCallable)
	void SetAbilityComponent(UAbilitySystemComponent* Component)
	{
		WeakComponent = Component;
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Abilities")
	bool bIsUseDefaultDataAsset = true;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Abilities")
	UVTHeroDataAsset* HeroDataAsset;

	// --------------------------------
	// Cur Data
	// --------------------------------

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Abilities")
	FGameplayTag HeroTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Abilities")
	FGameplayTagContainer CurHeroAbilities;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Abilities")
	TMap<FGameplayTag, FAbilityBaseInfo> CurAbilityBaseInfos;

	UPROPERTY(BlueprintReadOnly)
	TMap<FGameplayTag, UAbilitiesWidget*> CurAbilityWidgets;

	UPROPERTY(BlueprintReadOnly)
	UPanelWidget* AbilitysPanel = nullptr;

	// ================================
	// Cur Data
	// ================================

private:
	TWeakObjectPtr<UAbilitySystemComponent> WeakComponent;
};
