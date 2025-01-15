// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "TimerManager.h"

#include "VTCharacterBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVTCharacterDiedDelegate, AVTCharacterBase*, Character);

USTRUCT(BlueprintType)
struct LUGAMEPLAYFRAME_API FVTDamageNumber
{
	GENERATED_USTRUCT_BODY()

	float DamageAmount;

	FGameplayTagContainer Tags;

	FVTDamageNumber(): DamageAmount(0), Tags(FGameplayTagContainer{})
	{
	}

	FVTDamageNumber(float InDamageAmount, FGameplayTagContainer InTags) : DamageAmount(InDamageAmount)
	{
		Tags.AppendTags(InTags);
	}
};


UCLASS()
class LUGAMEPLAYFRAME_API AVTCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AVTCharacterBase(const class FObjectInitializer& ObjectInitializer);

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/**
	 * 是否还活着
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter")
	virtual bool IsAlive() const;

	/**
	 * Can only be called by the Server. 清除所有GA
	 */
	virtual void RemoveCharacterAbilities();

	/**
	 * 死亡处理
	 */
	virtual void Die();

	/**
	 * 死亡处理完成
	 */
	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter")
	virtual void FinishDying();

	/**
	 * 
	 * @param Damage 
	 * @param DamageNumberTags 
	 */
	virtual void AddDamageNumber(float Damage, FGameplayTagContainer DamageNumberTags);

	// Switch on AbilityID to return individual ability levels.
	// TODO:
	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter")
	virtual int32 GetAbilityLevel(EVTAbilityInputID AbilityID) const;

	UPROPERTY(BlueprintAssignable, Category = "VT|GSCharacter")
	FVTCharacterDiedDelegate OnCharacterDied;

	/**
	* Getters for attributes from GSAttributeSetBase
	**/

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	int32 GetCharacterLevel() const;

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetMana() const;

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetMaxMana() const;

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetShield() const;

	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetMaxShield() const;

	// Current MoveSpeed
	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetMoveSpeed() const;

	// Base MoveSpeed
	UFUNCTION(BlueprintCallable, Category = "VT|GSCharacter|Attributes")
	float GetMoveSpeedBaseValue() const;

protected:
	FGameplayTag DeadTag;
	FGameplayTag EffectRemoveOnDeathTag;
	TArray<FVTDamageNumber> DamageNumberQueue;
	FTimerHandle DamageNumberTimer;
	
	
	/**
	 *  初始AS
	 */
	UPROPERTY()
	class UVTAttributeSetBase* AttributeSetBase;

	/**
	 *  初始化的GA List
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "VT|Abilities")
	TArray<TSubclassOf<class UVTGameplayAbility>> CharacterAbilities;

	/**
	 *  初始化AS的GE
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "VT|Abilities")
	TSubclassOf<class UGameplayEffect> DefaultAttributes;

	/**
	 *  初始化的GE List
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "VT|Abilities")
	TArray<TSubclassOf<class UGameplayEffect>> StartupEffects;


	UPROPERTY()
	class UVTAbilitySystemComponent* AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "VT|GSCharacter")
	FText CharacterName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "VT|Animation")
	UAnimMontage* DeathMontage;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "VT|Audio")
	class USoundCue* DeathSound;
	
	UPROPERTY(EditAnywhere, Category = "VT|UI")
	TSubclassOf<class UVTDamageTextWidgetComponent> DamageNumberClass;
	
	
	virtual void BeginPlay() override;

	// CS
	virtual void AddCharacterAbilities();
	// CS
	virtual void InitializeAttributes();
	virtual void AddStartupEffects();
	virtual void ShowDamageNumber();


	/**
	*  会改变BaseValue
	*/

	virtual void SetHealth(float Health);
	virtual void SetMana(float Mana);
	virtual void SetStamina(float Stamina);
	virtual void SetShield(float Shield);
};
