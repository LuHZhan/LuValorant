// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "VTDamageExecutionCalc.generated.h"

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTDamageExecutionCalc : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UVTDamageExecutionCalc();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

protected:
	float HeadShotMultiplier;
};
