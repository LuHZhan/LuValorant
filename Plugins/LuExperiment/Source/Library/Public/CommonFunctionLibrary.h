// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CommonFunctionLibrary.generated.h"


UENUM()
enum ENetConnectionType:uint8
{
	Local,
	Client,
	// OtherClient,
	Server
};

/**
 * 
 */
UCLASS()
class LIBRARY_API UCommonFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (DefaultToSelf = "Actor"))
	static ENetConnectionType IsServerConnectionActor(AActor* Actor, TArray<UNetConnection*>& OutputConnection, bool& bIsServer);
};
