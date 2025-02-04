// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
// #include "Camera/CameraShakeBase.h"
#include "Shakes/PerlinNoiseCameraShakePattern.h"
#include "VTPerlinNoiseCameraShakePattern.generated.h"

/**
 * 
 */
UCLASS(BlueprintType,Blueprintable)
class LUGAMEPLAYFRAME_API UVTPerlinNoiseCameraShakePattern : public UPerlinNoiseCameraShakePattern
{
	GENERATED_BODY()

public:
	/** 开始累积后坐力 */
	UFUNCTION(BlueprintCallable, Category = "Recoil")
	void StartFiring(float DeltaTime);

	/** 停止并重置后坐力 */
	UFUNCTION(BlueprintCallable, Category = "Recoil")
	void StopFiring();

	/** 后坐力增长速率（每秒强度增量） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float RecoilGrowthRate = 1.0f;

	/** 最大后坐力强度 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float MaxRecoilIntensity = 5.0f;

	/** 后坐力曲线 */
	UPROPERTY(BlueprintReadWrite,EditAnywhere,Category="RecoilCurveLine")
	UCurveFloat* RecoilCurveLine;
	
	// virtual void TeardownShakePatternImpl() override;
	// virtual void StopShakePatternImpl(const FCameraShakePatternStopParams& Params) override;
	// virtual void ScrubShakePatternImpl(const FCameraShakePatternScrubParams& Params, FCameraShakePatternUpdateResult& OutResult) override;

private:

	virtual void StopShakePatternImpl(const FCameraShakePatternStopParams& Params) override;
	
	bool bIsFiring = false;
	float DurationSinceFiring = 0.0f;
	float CurrentRecoilIntensity = 0.0f;
};
