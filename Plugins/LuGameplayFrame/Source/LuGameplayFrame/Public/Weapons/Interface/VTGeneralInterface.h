// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpecHandle.h"
#include "UObject/Interface.h"

#include "VTGeneralInterface.generated.h"

class UGameplayAbility;
class UVTGameplayAbility;
class AVTHeroCharacter;
class AVTWeapon;

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UVTGeneralInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LUGAMEPLAYFRAME_API IVTGeneralInterface
{
	GENERATED_IINTERFACE_BODY()

public:
	virtual AVTWeapon* GetWeapon() const { return nullptr; }
	virtual AVTHeroCharacter* GetPawn() const { return nullptr; }

	virtual UVTGameplayAbility* GetAbilityInstanceFromClass(TSubclassOf<UGameplayAbility> InAbilityClass) const =0;
	virtual UVTGameplayAbility* GetAbilityInstanceFromHandle(FGameplayAbilitySpecHandle InHandle) const =0;

	virtual FGameplayAbilitySpecHandle GetAbilitySpecHandleFromClass(TSubclassOf<UGameplayAbility> InAbilityClass) const =0;

	virtual bool IsPrimaryAbilityInstanceActive(FGameplayAbilitySpecHandle Handle) const =0;

	virtual bool GASpecHandleIsValid(FGameplayAbilitySpecHandle SpecHandle) const =0;

	/** 当前GA是否激活 */
	virtual bool IsAbilityActive() const =0;
};
