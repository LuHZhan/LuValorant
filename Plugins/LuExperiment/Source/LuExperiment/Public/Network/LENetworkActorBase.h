// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LENetworkActorBase.generated.h"

UCLASS()
class LUEXPERIMENT_API ALENetworkActorBase : public AActor
{
	GENERATED_BODY()

public:
	ALENetworkActorBase();
	ALENetworkActorBase(const FObjectInitializer& ObjectInitializer);

	// virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool BIsReplicated = true;

	UPROPERTY(Replicated)
	int IntProperty;

	UPROPERTY(Replicated)
	FRepAttachment StructProperty;

	UPROPERTY(Replicated)
	TArray<int> ArrProperty;

	UPROPERTY(Replicated)
	FVector VecProperty;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// UFUNCTION(Server, reliable, WithValidation)
	// void ServerSetReplicatedTargetData(FGameplayAbilitySpecHandle AbilityHandle, FPredictionKey AbilityOriginalPredictionKey, const FGameplayAbilityTargetDataHandle& ReplicatedTargetDataHandle,
	//                                    FGameplayTag ApplicationTag, FPredictionKey CurrentPredictionKey);
};
