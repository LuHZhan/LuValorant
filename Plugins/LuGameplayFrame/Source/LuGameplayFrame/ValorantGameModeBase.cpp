// Copyright 2024 Dan Kestranek.


#include "ValorantGameModeBase.h"
#include "Engine/World.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

#include "Characters/Heroes/VTHeroCharacter.h"
#include "Player/VTPlayerController.h"
// #include "Player/VTPlayerState.h"

AValorantGameModeBase::AValorantGameModeBase(): EnemySpawnPoint(nullptr)
{
	RespawnDelay = 5.0f;

	HeroClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/GASShooter/Characters/Hero/BP_HeroCharacter.BP_HeroCharacter_C"));
	if (!HeroClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Failed to find HeroClass. If it was moved, please update the reference location in C++."), *FString(__FUNCTION__));
	}
}

void AValorantGameModeBase::HeroDied(AController* Controller)
{
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	ASpectatorPawn* SpectatorPawn = GetWorld()->SpawnActor<ASpectatorPawn>(SpectatorClass, Controller->GetPawn()->GetActorTransform(), SpawnParameters);

	Controller->UnPossess();
	Controller->Possess(SpectatorPawn);

	FTimerHandle RespawnTimerHandle;
	FTimerDelegate RespawnDelegate;

	RespawnDelegate = FTimerDelegate::CreateUObject(this, &AValorantGameModeBase::RespawnHero, Controller);
	GetWorldTimerManager().SetTimer(RespawnTimerHandle, RespawnDelegate, RespawnDelay, false);

	AVTPlayerController* PC = Cast<AVTPlayerController>(Controller);
	if (PC)
	{
		PC->SetRespawnCountdown(RespawnDelay);
	}
}

void AValorantGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Get the enemy hero spawn point
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), Actors);
	for (AActor* Actor : Actors)
	{
		if (Actor->GetName() == FString("EnemyHeroSpawn"))
		{
			EnemySpawnPoint = Actor;
			break;
		}
	}

	if (!EnemySpawnPoint)
	{
		UE_LOG(LogTemp, Error, TEXT("%s EnemySpawnPoint is null."), *FString(__FUNCTION__));
	}
}

void AValorantGameModeBase::RespawnHero(AController* Controller)
{
	if (!IsValid(Controller))
	{
		return;
	}

	if (Controller->IsPlayerController())
	{
		// Respawn player hero
		AActor* PlayerStart = FindPlayerStart(Controller);

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AVTHeroCharacter* Hero = GetWorld()->SpawnActor<AVTHeroCharacter>(HeroClass, PlayerStart->GetActorLocation(), PlayerStart->GetActorRotation(), SpawnParameters);

		APawn* OldSpectatorPawn = Controller->GetPawn();
		Controller->UnPossess();
		OldSpectatorPawn->Destroy();
		Controller->Possess(Hero);
		
		AVTPlayerController* PC = Cast<AVTPlayerController>(Controller);
		if (PC)
		{
			PC->ClientSetControlRotation(PlayerStart->GetActorRotation());
		}
	}
	else
	{
		// Respawn AI hero
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		AVTHeroCharacter* Hero = GetWorld()->SpawnActor<AVTHeroCharacter>(HeroClass, EnemySpawnPoint->GetActorTransform(), SpawnParameters);

		APawn* OldSpectatorPawn = Controller->GetPawn();
		Controller->UnPossess();
		OldSpectatorPawn->Destroy();
		Controller->Possess(Hero);
	}
}