#pragma once
#include "VTSettingType.generated.h"

USTRUCT(BlueprintType)
struct FVTHeroSettingType
{
	GENERATED_BODY()

	FVTHeroSettingType()
	{
	}

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HeroSetting")
	float LookScale = 1.0f;
};
