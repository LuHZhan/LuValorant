// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/CollisionProfile.h"
#include "Kismet/KismetSystemLibrary.h"
#include "WorldCollision.h"
#include "VTGATA_Trace.generated.h"

/**
 * Reusable, configurable trace TargetActor. Subclass with your own trace shapes.
 * Meant to be used with GSAT_WaitTargetDataUsingActor instead of the default WaitTargetData AbilityTask as the default
 * one will destroy the TargetActor.
 * 
 * 可重用的、可配置的跟踪TargetActor。子类具有自己的跟踪形状。
 * 意味着与GSAT_WaitTargetDataUsingActor一起使用，而不是默认的WaitTargetData AbilityTask，因为默认的会破坏TargetActor。
 */
UCLASS()
class LUGAMEPLAYFRAME_API AVTGATA_Trace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	AVTGATA_Trace();

	// 基础瞄准散布（角度）
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float BaseSpread;

	// 瞄准时的散布修正值
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float AimingSpreadMod;

	// 连续瞄准时的散布增量
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float TargetingSpreadIncrement;

	// 连续瞄准时的最大散布值
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	float TargetingSpreadMax;

	// 当前连续瞄准的散布值
	float CurrentTargetingSpread;

	// 是否使用瞄准散布修正
	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	bool bUseAimingSpreadMod;

	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	FGameplayTag AimingTag;

	UPROPERTY(BlueprintReadWrite, Category = "Accuracy")
	FGameplayTag AimingRemovalTag;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	float MaxRange;

	// 碰撞检测的配置名称
	UPROPERTY(BlueprintReadWrite, EditAnywhere, config, meta = (ExposeOnSpawn = true), Category = "Trace")
	FCollisionProfileName TraceProfile;

	// 跟踪是否影响瞄准的俯仰角
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bTraceAffectsAimPitch;

	// 每次跟踪返回的最大命中结果数
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	int32 MaxHitResultsPerTrace;

	// 每次执行的跟踪次数（例如，霰弹枪可能需要多次跟踪）
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	int32 NumberOfTraces;

	// 是否忽略阻挡命中
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bIgnoreBlockingHits;

	// 是否从玩家视角进行跟踪
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bTraceFromPlayerViewPoint;

	// 是否使用持久化的命中结果
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true), Category = "Trace")
	bool bUsePersistentHitResults;

public:
	UFUNCTION(BlueprintCallable)
	virtual void ResetSpread();

	UFUNCTION(BlueprintCallable)
	virtual float GetCurrentSpread() const;

	UFUNCTION(BlueprintCallable)
	void SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation);

	UFUNCTION(BlueprintCallable)
	virtual void SetShouldProduceTargetDataOnServer(bool bInShouldProduceTargetDataOnServer);

	UFUNCTION(BlueprintCallable)
	void SetDestroyOnConfirmation(bool bInDestroyOnConfirmation = false);

	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual void ConfirmTargetingAndContinue() override;

	virtual void CancelTargeting() override;

	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End,
	                                 FName ProfileName, const FCollisionQueryParams Params);

	virtual void AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch = false);

	/** 将相机射线裁剪到技能范围内，计算射线与技能范围球体的交点 */
	virtual bool ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition);

	virtual void StopTargeting();

protected:
	FVector CurrentTraceEnd;
	TArray<TWeakObjectPtr<AGameplayAbilityWorldReticle>> ReticleActors;

	// 持久化的命中结果集合
	TArray<FHitResult> PersistentHitResults;

	/** 根据命中结果生成目标数据 */
	virtual FGameplayAbilityTargetDataHandle MakeTargetData(const TArray<FHitResult>& HitResults) const;
	
	/** 执行跟踪并返回命中结果 */
	virtual TArray<FHitResult> PerformTrace(AActor* InSourceActor);

	/** 开始追踪 */
	virtual void DoTrace(TArray<FHitResult>& HitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End, FName ProfileName,
	                     const FCollisionQueryParams Params) PURE_VIRTUAL(AGSGATA_Trace, return;);
	
	virtual void ShowDebugTrace(TArray<FHitResult>& HitResults, EDrawDebugTrace::Type DrawDebugType, float Duration = 2.0f) PURE_VIRTUAL(AGSGATA_Trace, return;);
	
	virtual AGameplayAbilityWorldReticle* SpawnReticleActor(FVector Location, FRotator Rotation);
	/** 应该有更好的清空的方式 */
	virtual void DestroyReticleActors();
};
