// Copyright Epic Games, Inc. All Rights Reserved.

#include "ReplicatesThreeGameMode.h"
#include "ReplicatesThreeCharacter.h"
#include "UObject/ConstructorHelpers.h"

AReplicatesThreeGameMode::AReplicatesThreeGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
