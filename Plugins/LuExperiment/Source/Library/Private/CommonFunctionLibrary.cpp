// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonFunctionLibrary.h"

ENetConnectionType UCommonFunctionLibrary::IsServerConnectionActor(AActor* Actor, TArray<UNetConnection*>& OutputConnection, bool& bIsServer)
{
	const UWorld* World = Actor->GetWorld();
	if (!World)
	{
		bIsServer = false;
		return ENetConnectionType::Local;
	}

	UNetDriver* NetDriver = World->GetNetDriver();
	if (!NetDriver)
	{
		bIsServer = false;
		return ENetConnectionType::Local;
	}

	if (NetDriver->ServerConnection)
	{
		OutputConnection.Add(NetDriver->ServerConnection);
		// return Actor->HasLocalNetOwner() ? ENetConnectionType::Client : ENetConnectionType::OtherClient;
		bIsServer = false;
		return ENetConnectionType::Client;
	}

	for (UNetConnection* ClientConnection : NetDriver->ClientConnections)
	{
		OutputConnection.Add(ClientConnection);
	}
	bIsServer = true;
	return ENetConnectionType::Server;
}
