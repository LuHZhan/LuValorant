// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/LENetworkActorBase.h"


// Sets default values
ALENetworkActorBase::ALENetworkActorBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

ALENetworkActorBase::ALENetworkActorBase(const FObjectInitializer& ObjectInitializer)
{
	SetReplicates(true);
}

// Called when the game starts or when spawned
void ALENetworkActorBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALENetworkActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
