// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VTProjectile.generated.h"

UCLASS()
class LUGAMEPLAYFRAME_API AVTProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AVTProjectile();

protected:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "PBProjectile")
	class UProjectileMovementComponent* ProjectileMovement;
};
