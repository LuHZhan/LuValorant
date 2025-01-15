// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "VTAnimNotify_PlaySoundForPerspective.generated.h"

/**
 * AnimNotify to play a sound if the character is in the matching perspective (1st or 3rd).
 */
UCLASS(meta = (DisplayName = "Play Sound For Perspective"))
class GASSHOOTER_API UVTAnimNotify_PlaySoundForPerspective : public UAnimNotify_PlaySound
{
	GENERATED_BODY()

public:
	UVTAnimNotify_PlaySoundForPerspective();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AnimNotify", meta = (ExposeOnSpawn = true))
	bool bPlayForFirstPersonPerspective;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
