// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/LENetworkActorBase.h"

#include "Net/UnrealNetwork.h"


// Sets default values
ALENetworkActorBase::ALENetworkActorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

ALENetworkActorBase::ALENetworkActorBase(const FObjectInitializer& ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

// void ALENetworkActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
// 	DOREPLIFETIME(ALENetworkActorBase,IntProperty);
// 	DOREPLIFETIME(ALENetworkActorBase, VecProperty);
// 	DOREPLIFETIME_CONDITION(ALENetworkActorBase, StructProperty, COND_InitialOnly);
// 	DOREPLIFETIME_CONDITION(ALENetworkActorBase, ArrProperty, COND_OwnerOnly);
// }

// Called when the game starts or when spawned
void ALENetworkActorBase::BeginPlay()
{
	Super::BeginPlay();

	// Cast<AActor>(this)->SetActorTickEnabled(false);
}

// Called every frame
void ALENetworkActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
