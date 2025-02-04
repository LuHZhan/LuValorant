// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/VTPerlinNoiseCameraShakePattern.h"

#include "Kismet/KismetSystemLibrary.h"
#include "Shakes/PerlinNoiseCameraShakePattern.h"

//
// UVTPerlinNoiseCameraShakePattern::UVTPerlinNoiseCameraShakePattern()
// {
// 	Pitch.Amplitude = 1.0f;
// 	Pitch.Frequency = 10.0f;
//
// 	Yaw.Amplitude = 0.25f;
// 	Yaw.Frequency = 10.0f;
//
// 	BlendInTime = 0.1f;
// 	BlendOutTime = .2f;
// 	Duration = 0.25f;
// }

void UVTPerlinNoiseCameraShakePattern::StartFiring(float DeltaTime)
{
	bIsFiring = true;
	DurationSinceFiring += DeltaTime;
	CurrentRecoilIntensity = FMath::Min(DurationSinceFiring * RecoilGrowthRate, MaxRecoilIntensity);
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("CurrentRecoilIntensity: %f"), CurrentRecoilIntensity));
}

void UVTPerlinNoiseCameraShakePattern::StopFiring()
{
	bIsFiring = false;
	DurationSinceFiring = 0.0f;
	CurrentRecoilIntensity = 0.0f;
}

void UVTPerlinNoiseCameraShakePattern::StopShakePatternImpl(const FCameraShakePatternStopParams& Params)
{
	Super::StopShakePatternImpl(Params);
}
