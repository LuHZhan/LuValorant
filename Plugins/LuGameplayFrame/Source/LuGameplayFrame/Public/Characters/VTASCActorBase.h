// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "VTASCActorBase.generated.h"

/**
* Base AActor class that has an AbilitySystemComponent but does not inherit from ACharacter.
*/
UCLASS()
class LUGAMEPLAYFRAME_API AVTASCActorBase : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVTASCActorBase();

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	UPROPERTY()
	class UVTAbilitySystemComponent* AbilitySystemComponent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
