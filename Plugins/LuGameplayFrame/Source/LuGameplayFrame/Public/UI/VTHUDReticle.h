// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VTHUDReticle.generated.h"

/**
 * 十字准心
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTHUDReticle : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetColor(FLinearColor Color);
};
