// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "GameplayEffectTypes.h"
#include "VTPlayerState.generated.h"

class UGameplayEffect;
struct FHeroAbilityData;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVTOnGameplayAttributeValueChangedDelegate, FGameplayAttribute, Attribute, float, NewValue, float, OldValue);

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API AVTPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AVTPlayerState();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	class UVTAttributeSetBase* GetAttributeSetBase() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	class UVTAmmoAttributeSet* GetAmmoAttributeSet() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TMap<FGameplayTag, class UVTAbilityAttributeSet*> GetAbilityAttributeSet() const;

	UFUNCTION(BlueprintCallable,BlueprintPure)
	FGameplayTag K2_GetCurrentHeroTag() const;
	const FGameplayTag* GetCurrentHeroTag() const;
	
	UFUNCTION(BlueprintCallable,BlueprintPure)
	FHeroAbilityData GetHeroAbilityData() const;

	UFUNCTION(BlueprintCallable,BlueprintPure)
	const TMap<FGameplayTag, TSubclassOf<UGameplayEffect>>& GetAbilityStartupInitAttributesGE() const;
	
	UFUNCTION(BlueprintCallable)
	bool LoadHeroAbilityAttributeEmptyKey();

	UFUNCTION(BlueprintCallable)
	bool LoadHeroAbilityStartupAttributesGEs();

	UFUNCTION(BlueprintCallable, Category = "PlayerState")
	bool IsAlive() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|UI")
	void ShowAbilityConfirmPrompt(bool bShowPrompt);

	UFUNCTION(BlueprintCallable, Category = "PlayerState|UI")
	void ShowInteractionPrompt(float InteractionDuration);

	UFUNCTION(BlueprintCallable, Category = "PlayerState|UI")
	void HideInteractionPrompt();

	UFUNCTION(BlueprintCallable, Category = "PlayerState|UI")
	void StartInteractionTimer(float InteractionDuration);

	UFUNCTION(BlueprintCallable, Category = "PlayerState|UI")
	void StopInteractionTimer();

	// UFUNCTION(BlueprintCallable,BlueprintPure, Category = "PlayerState|Attributes")
	// TMap<FGameplayAttributeData, float> GetAttribute(const TArray<FGameplayAttributeData>& TargetAttributeData) const;

	// ----------------------------------------------------------------------------------------------------------------
	//	Attributes Get and Set
	// ----------------------------------------------------------------------------------------------------------------

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetMaxShield() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetArmor() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	float GetMoveSpeed() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	int32 GetXP() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	int32 GetXPBounty() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	int32 GetGold() const;

	UFUNCTION(BlueprintCallable, Category = "PlayerState|Attributes")
	int32 GetGoldBounty() const;

	// ================================================================================================================
	//	Attributes Get and Set
	// ================================================================================================================

protected:
	FGameplayTag DeadTag;
	FGameplayTag KnockedDownTag;

	UPROPERTY(BlueprintReadOnly)
	FGameplayTag HeroTag;

	UPROPERTY()
	class UVTAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY()
	class UVTAttributeSetBase* AttributeSetBase;

	UPROPERTY()
	class UVTAmmoAttributeSet* AmmoAttributeSet;

	UPROPERTY()
	TMap<FGameplayTag, class UVTAbilityAttributeSet*> AbilityAttributeMap;

	UPROPERTY()
	TMap<FGameplayTag, TSubclassOf<UGameplayEffect>> StartupInitAbilityAttributesGE;
	
	FDelegateHandle HealthChangedDelegateHandle;
	

	virtual void BeginPlay() override;

	virtual void HealthChanged(const FOnAttributeChangeData& Data);
	virtual void KnockDownTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
};
