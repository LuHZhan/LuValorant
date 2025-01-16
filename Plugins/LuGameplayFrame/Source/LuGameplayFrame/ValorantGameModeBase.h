// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ValorantGameModeBase.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class LUGAMEPLAYFRAME_API AValorantGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AValorantGameModeBase();

	void HeroDied(AController* Controller);

protected:
	float RespawnDelay;

	TSubclassOf<class AVTHeroCharacter> HeroClass;

	AActor* EnemySpawnPoint;

	virtual void BeginPlay() override;

	void RespawnHero(AController* Controller);
};
