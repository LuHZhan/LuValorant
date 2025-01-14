// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VTFloatingStatusBarWidget.generated.h"

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTFloatingStatusBarWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// UPROPERTY(BlueprintReadOnly)
	// class AGSCharacterBase* OwningCharacter;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetHealthPercentage(float HealthPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetManaPercentage(float ManaPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetShieldPercentage(float ShieldPercentage);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetCharacterName(const FText& NewName);
};
