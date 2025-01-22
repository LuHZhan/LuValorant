// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "../Characters/Abilities/AttributeSets/VTAttributeSetBase.h"
#include "../Characters/Abilities/AttributeSets/VTAmmoAttributeSet.h"

#include "VTHUDWidget.generated.h"

class UPaperSprite;
class UTexture2D;

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CallUpdateEvent(FGameplayAttribute AS, float NewValue, float OldValue);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "VT|UI")
	TMap<FGameplayAttribute, FName> AttributeFuncNameMap;
	// TMap<FGameplayAttribute, UFunction> AttributeFuncMap;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowAbilityConfirmPrompt(bool bShowText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetRespawnCountdown(float RespawnTimeRemaining);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void ShowInteractionPrompt(float InteractionDuration);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void HideInteractionPrompt();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StartInteractionTimer(float InteractionDuration);

	// Interaction interrupted, cancel and hide HUD interact timer
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void StopInteractionTimer();


	/**
	* Weapon info
	*/

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEquippedWeaponSprite(UPaperSprite* Sprite);

	// Things like fire mode for rifle
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetEquippedWeaponStatusText(const FText& StatusText);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPrimaryClipAmmo(int32 ClipAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetPrimaryReserveAmmo(int32 ReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetSecondaryClipAmmo(int32 SecondaryClipAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetSecondaryReserveAmmo(int32 SecondaryReserveAmmo);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetReticle(TSubclassOf<class UVTHUDReticle> ReticleClass);


	/**
	* Attribute setters
	*/

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxHealth(float NewValue, float OldValue = -1);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentHealth(float NewValue, float OldValue = -1);

	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetHealthPercentage(float HealthPercentage);

	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetHealthRegenRate(float HealthRegenRate);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxStamina(float NewValue, float OldValue = -1);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentStamina(float NewValue, float OldValue = -1);

	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetStaminaPercentage(float StaminaPercentage);
	//
	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetStaminaRegenRate(float StaminaRegenRate);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxMana(float NewValue, float OldValue = -1);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentMana(float NewValue, float OldValue = -1);

	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetManaPercentage(float ManaPercentage);
	//
	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetManaRegenRate(float ManaRegenRate);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetMaxShield(float NewValue, float OldValue = -1);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCurrentShield(float NewValue, float OldValue = -1);

	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetShieldPercentage(float ShieldPercentage);
	//
	// UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	// void SetShieldRegenRate(float ShieldRegenRate);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetExperience(int32 Experience);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHeroLevel(int32 HeroLevel);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetGold(int32 Gold);
};
