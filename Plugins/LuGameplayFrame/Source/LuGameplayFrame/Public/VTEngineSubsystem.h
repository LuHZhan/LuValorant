// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "VTEngineSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
