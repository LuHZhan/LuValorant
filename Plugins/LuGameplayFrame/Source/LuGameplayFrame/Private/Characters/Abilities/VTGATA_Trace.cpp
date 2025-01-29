// Copyright 2024 Dan Kestranek.


#include "Characters/Abilities/VTGATA_Trace.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemLog.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerController.h"
#include "GameplayAbilitySpec.h"

AVTGATA_Trace::AVTGATA_Trace()
{
	bDestroyOnConfirmation = false;
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	MaxHitResultsPerTrace = 1;
	NumberOfTraces = 1;
	bIgnoreBlockingHits = false;
	bTraceAffectsAimPitch = true;
	bTraceFromPlayerViewPoint = false;
	MaxRange = 999999.0f;
	bUseAimingSpreadMod = false;
	BaseSpread = 0.0f;
	AimingSpreadMod = 0.0f;
	TargetingSpreadIncrement = 0.0f;
	TargetingSpreadMax = 0.0f;
	CurrentTargetingSpread = 0.0f;
	bUsePersistentHitResults = false;
}

void AVTGATA_Trace::ResetSpread()
{
	bUseAimingSpreadMod = false;
	BaseSpread = 0.0f;
	AimingSpreadMod = 0.0f;
	TargetingSpreadIncrement = 0.0f;
	TargetingSpreadMax = 0.0f;
	CurrentTargetingSpread = 0.0f;
}

float AVTGATA_Trace::GetCurrentSpread() const
{
	float FinalSpread = BaseSpread + CurrentTargetingSpread;

	if (bUseAimingSpreadMod && AimingTag.IsValid() && AimingRemovalTag.IsValid())
	{
		UAbilitySystemComponent* ASC = OwningAbility->GetCurrentActorInfo()->AbilitySystemComponent.Get();
		if (ASC && (ASC->GetTagCount(AimingTag) > ASC->GetTagCount(AimingRemovalTag)))
		{
			FinalSpread *= AimingSpreadMod;
		}
	}

	return FinalSpread;
}

void AVTGATA_Trace::SetStartLocation(const FGameplayAbilityTargetingLocationInfo& InStartLocation)
{
	StartLocation = InStartLocation;
}

void AVTGATA_Trace::SetShouldProduceTargetDataOnServer(bool bInShouldProduceTargetDataOnServer)
{
	ShouldProduceTargetDataOnServer = bInShouldProduceTargetDataOnServer;
}

void AVTGATA_Trace::SetDestroyOnConfirmation(bool bInDestroyOnConfirmation)
{
	bDestroyOnConfirmation = bInDestroyOnConfirmation;
}

void AVTGATA_Trace::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);

	SetActorTickEnabled(true);

	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

	DestroyReticleActors();

	if (ReticleClass)
	{
		for (int32 i = 0; i < MaxHitResultsPerTrace * NumberOfTraces; i++)
		{
			SpawnReticleActor(GetActorLocation(), GetActorRotation());
		}
	}

	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AVTGATA_Trace::ConfirmTargetingAndContinue()
{
	check(ShouldProduceTargetData());
	if (SourceActor)
	{
		TArray<FHitResult> HitResults = PerformTrace(SourceActor);
		const FGameplayAbilityTargetDataHandle Handle = MakeTargetData(HitResults);
		TargetDataReadyDelegate.Broadcast(Handle);

#if ENABLE_DRAW_DEBUG
		if (bDebug)
		{
			ShowDebugTrace(HitResults, EDrawDebugTrace::Type::ForDuration, 2.0f);
		}
#endif
	}

	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AVTGATA_Trace::CancelTargeting()
{
	const FGameplayAbilityActorInfo* ActorInfo = (OwningAbility ? OwningAbility->GetCurrentActorInfo() : nullptr);
	if (UAbilitySystemComponent* ASC = (ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr))
	{
		// Remove Delegate
		ASC->AbilityReplicatedEventDelegate(EAbilityGenericReplicatedEvent::GenericCancel, OwningAbility->GetCurrentAbilitySpecHandle(),
		                                    OwningAbility->GetCurrentActivationInfo().GetActivationPredictionKey()).Remove(GenericCancelHandle);
	}
	else
	{
		ABILITY_LOG(Warning, TEXT("AGameplayAbilityTargetActor::CancelTargeting called with null ASC! Actor %s"), *GetName());
	}

	CanceledDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
	SetActorTickEnabled(false);
	if (bUsePersistentHitResults)
	{
		PersistentHitResults.Empty();
	}
}

void AVTGATA_Trace::BeginPlay()
{
	Super::BeginPlay();

	// Start with Tick disabled. We'll enable it in StartTargeting() and disable it again in StopTargeting().
	// For instant confirmations, tick will never happen because we StartTargeting(), ConfirmTargeting(), and immediately StopTargeting().
	SetActorTickEnabled(false);
}

void AVTGATA_Trace::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DestroyReticleActors();

	Super::EndPlay(EndPlayReason);
}

void AVTGATA_Trace::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TArray<FHitResult> HitResults;
	if (bDebug || bUsePersistentHitResults)
	{
		HitResults = PerformTrace(SourceActor);
	}

#if ENABLE_DRAW_DEBUG
	if (SourceActor && bDebug)
	{
		ShowDebugTrace(HitResults, EDrawDebugTrace::Type::ForOneFrame);
	}
#endif
}

void AVTGATA_Trace::LineTraceWithFilter(TArray<FHitResult>& OutHitResults, const UWorld* World, const FGameplayTargetDataFilterHandle FilterHandle, const FVector& Start, const FVector& End,
                                        FName ProfileName, const FCollisionQueryParams Params)
{
	check(World);

	TArray<FHitResult> HitResults;
	World->LineTraceMultiByProfile(HitResults, Start, End, ProfileName, Params);

	TArray<FHitResult> FilteredHitResults;
	const FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();

	for (FHitResult HitResult : HitResults)
	{
		if (!HitResult.GetActor() || FilterHandle.FilterPassesForActor(HitResult.GetActor()))
		{
			HitResult.TraceStart = TraceStart;
			HitResult.TraceEnd = End;
			FilteredHitResults.Add(HitResult);
		}
	}

	OutHitResults = FilteredHitResults;
}

void AVTGATA_Trace::AimWithPlayerController(const AActor* InSourceActor, FCollisionQueryParams Params, const FVector& TraceStart, FVector& OutTraceEnd, bool bIgnorePitch)
{
	if (!OwningAbility)
	{
		return;
	}

	FVector ViewStart = TraceStart;
	FRotator ViewRot = StartLocation.GetTargetingTransform().GetRotation().Rotator();
	if (PrimaryPC)
	{
		PrimaryPC->GetPlayerViewPoint(ViewStart, ViewRot);
	}

	const FVector ViewDir = ViewRot.Vector();
	FVector ViewEnd = ViewStart + (ViewDir * MaxRange);

	// 计算出最终的ClippedPosition
	ClipCameraRayToAbilityRange(ViewStart, ViewDir, TraceStart, MaxRange, ViewEnd);

	// 过滤掉掉不需要的HitResult
	TArray<FHitResult> HitResults;
	LineTraceWithFilter(HitResults, InSourceActor->GetWorld(), Filter, ViewStart, ViewEnd, TraceProfile.Name, Params);

	// 更新当前散布值
	CurrentTargetingSpread = FMath::Min(TargetingSpreadMax, CurrentTargetingSpread + TargetingSpreadIncrement);

	// 获取最终HitResult的最大落点
	const bool bUseTraceResult = HitResults.Num() > 0 && (FVector::DistSquared(TraceStart, HitResults[0].Location) <= (MaxRange * MaxRange));
	const FVector AdjustedEnd = (bUseTraceResult) ? HitResults[0].Location : ViewEnd;

	// 基于修正后最终HitResult落点的方向
	FVector AdjustedAimDir = (AdjustedEnd - TraceStart).GetSafeNormal().IsZero() ? ViewDir : (AdjustedEnd - TraceStart).GetSafeNormal();

	if (!bTraceAffectsAimPitch && bUseTraceResult)
	{
		// 基于视野落点的方向 
		if (FVector OriginalAimDir = (ViewEnd - TraceStart).GetSafeNormal(); !OriginalAimDir.IsZero())
		{
			// 转换为角度并使用原始Pitch
			FRotator OriginalAimRot = OriginalAimDir.Rotation();
			FRotator AdjustedAimRot = AdjustedAimDir.Rotation();

			// 使用View的Pitch
			AdjustedAimRot.Pitch = OriginalAimRot.Pitch;
			// 更新修正后的方向
			AdjustedAimDir = AdjustedAimRot.Vector();
		}
	}

	const float CurrentSpread = GetCurrentSpread();
	// 散布锥体的一半角度
	const float ConeHalfAngle = FMath::DegreesToRadians(CurrentSpread * 0.5f);

	// 获取最终的增加了随机值的方向
	const int32 RandomSeed = FMath::Rand();
	FRandomStream WeaponRandomStream(RandomSeed);
	const FVector ShootDir = WeaponRandomStream.VRandCone(AdjustedAimDir, ConeHalfAngle, ConeHalfAngle);

	OutTraceEnd = TraceStart + (ShootDir * MaxRange);
}

bool AVTGATA_Trace::ClipCameraRayToAbilityRange(FVector CameraLocation, FVector CameraDirection, FVector AbilityCenter, float AbilityRange, FVector& ClippedPosition)
{
	// Relative ViewStart Vector
	const FVector CameraToCenter = AbilityCenter - CameraLocation;
	// 判断Camera是否朝向AbilityCenter,true表示Camera朝向AbilityCenter
	if (const float DotToCenter = FVector::DotProduct(CameraToCenter, CameraDirection); DotToCenter >= 0)
	{
		// Camera到AbilityCenter的垂直距离平方
		const float DistanceSquared = CameraToCenter.SizeSquared() - (DotToCenter * DotToCenter);
		// 射线与技能范围球体相交
		if (const float RadiusSquared = (AbilityRange * AbilityRange); DistanceSquared <= RadiusSquared)
		{
			// 计算从Camera到交点的距离
			const float DistanceFromCamera = FMath::Sqrt(RadiusSquared - DistanceSquared);
			// 计算沿射线的总距离,减法而不是加法会得到另一个交点
			const float DistanceAlongRay = DotToCenter + DistanceFromCamera;
			// 计算裁剪后的交点位置,凸轮瞄准点裁剪到范围球体
			ClippedPosition = CameraLocation + (DistanceAlongRay * CameraDirection);
			return true;
		}
	}

	//如果这个失败了，我们就偏离了中心，但我们可能在球体内部，并且能够找到一个好的退出点。
	return false;
}

void AVTGATA_Trace::StopTargeting()
{
	SetActorTickEnabled(false);

	DestroyReticleActors();

	// 清除Delegate
	TargetDataReadyDelegate.Clear();
	CanceledDelegate.Clear();

	if (GenericDelegateBoundASC)
	{
		GenericDelegateBoundASC->GenericLocalConfirmCallbacks.RemoveDynamic(this, &AGameplayAbilityTargetActor::ConfirmTargeting);
		GenericDelegateBoundASC->GenericLocalCancelCallbacks.RemoveDynamic(this, &AGameplayAbilityTargetActor::CancelTargeting);
		GenericDelegateBoundASC = nullptr;
	}
}

FGameplayAbilityTargetDataHandle AVTGATA_Trace::MakeTargetData(const TArray<FHitResult>& HitResults) const
{
	FGameplayAbilityTargetDataHandle ReturnDataHandle;

	for (int32 i = 0; i < HitResults.Num(); i++)
	{
		// 由FGameplayAbilityTargetDataHandle（通过内部的TSharedPtr）清理的
		FGameplayAbilityTargetData_SingleTargetHit* ReturnData = new FGameplayAbilityTargetData_SingleTargetHit();
		ReturnData->HitResult = HitResults[i];
		ReturnDataHandle.Add(ReturnData);
	}

	return ReturnDataHandle;
}

TArray<FHitResult> AVTGATA_Trace::PerformTrace(AActor* InSourceActor)
{
	bool bTraceComplex = false;
	TArray<AActor*> ActorsToIgnore;

	ActorsToIgnore.Add(InSourceActor);

	// 配置碰撞参数
	FCollisionQueryParams Params(SCENE_QUERY_STAT(AGSGATA_LineTrace), bTraceComplex);
	Params.bReturnPhysicalMaterial = true;
	Params.AddIgnoredActors(ActorsToIgnore);
	Params.bIgnoreBlocks = bIgnoreBlockingHits;

	FVector TraceStart = StartLocation.GetTargetingTransform().GetLocation();
	FVector TraceEnd;

	// 如果使用玩家视点进行追踪，则使用Camera作为起点
	if (PrimaryPC && bTraceFromPlayerViewPoint)
	{
		FVector ViewStart;
		FRotator ViewRot;
		PrimaryPC->GetPlayerViewPoint(ViewStart, ViewRot);
		TraceStart = ViewStart;
	}

	if (bUsePersistentHitResults)
	{
		// 清除任何阻挡命中结果、无效Actor,超出范围的Actor
		for (int32 i = PersistentHitResults.Num() - 1; i >= 0; i--)
		{
			FHitResult& HitResult = PersistentHitResults[i];

			if (HitResult.bBlockingHit || !HitResult.GetActor() || FVector::DistSquared(TraceStart, HitResult.GetActor()->GetActorLocation()) > (MaxRange * MaxRange))
			{
				PersistentHitResults.RemoveAt(i);
			}
		}
	}

	TArray<FHitResult> ReturnHitResults;
	for (int32 TraceIndex = 0; TraceIndex < NumberOfTraces; TraceIndex++)
	{
		// 只在Server或者Launching Client上运行
		AimWithPlayerController(InSourceActor, Params, TraceStart, TraceEnd);

		// ------------------------------------------------------

		SetActorLocationAndRotation(TraceEnd, SourceActor->GetActorRotation());
		CurrentTraceEnd = TraceEnd;

		TArray<FHitResult> TraceHitResults;
		// 追踪并获取数据TraceHitResults
		DoTrace(TraceHitResults, InSourceActor->GetWorld(), Filter, TraceStart, TraceEnd, TraceProfile.Name, Params);

		// 从后开始往前遍历
		for (int32 j = TraceHitResults.Num() - 1; j >= 0; j--)
		{
			// 丢弃超出追踪最大命数[MaxHitResultsPerTrace]的命中结果
			if (MaxHitResultsPerTrace >= 0 && j + 1 > MaxHitResultsPerTrace)
			{
				TraceHitResults.RemoveAt(j);
				continue;
			}

			FHitResult& HitResult = TraceHitResults[j];

			// Reminder: if bUsePersistentHitResults, Number of Traces = 1
			if (bUsePersistentHitResults)
			{
				// 将命中结果持久化[PersistentHitResults]容器中
				if (HitResult.GetActor() && (!HitResult.bBlockingHit || PersistentHitResults.IsEmpty()))
				{
					// 遍历是否有重复持久化的命中结果，有则跳过
					bool bActorAlreadyInPersistentHits = false;
					for (int32 k = 0; k < PersistentHitResults.Num(); k++)
					{
						FHitResult& PersistentHitResult = PersistentHitResults[k];

						if (PersistentHitResult.GetActor() == HitResult.GetActor())
						{
							bActorAlreadyInPersistentHits = true;
							break;
						}
					}

					if (bActorAlreadyInPersistentHits)
					{
						continue;
					}

					// 会随着遍历将更近的命中结果持久化到容器中
					if (PersistentHitResults.Num() >= MaxHitResultsPerTrace)
					{
						// Treat PersistentHitResults like a queue, remove first element
						PersistentHitResults.RemoveAt(0);
					}
					PersistentHitResults.Add(HitResult);
				}
			}
			else
			{
				// 根据当前的索引获取对应的Reticle
				int32 ReticleIndex = TraceIndex * MaxHitResultsPerTrace + j;
				if (ReticleIndex < ReticleActors.Num())
				{
					if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[ReticleIndex].Get())
					{
						if (const bool bHitActor = HitResult.GetActor() != nullptr; bHitActor && !HitResult.bBlockingHit)
						{
							LocalReticleActor->SetActorHiddenInGame(false);
							// 如果射线命中了一个Actor，并且Reticle的bSnapToTargetedActor为true，则将Reticle吸附到目标Actor的位置。
							const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? HitResult.GetActor()->GetActorLocation() : HitResult.Location;
							LocalReticleActor->SetActorLocation(ReticleLocation);
							LocalReticleActor->SetIsTargetAnActor(bHitActor);
						}
						else
						{
							LocalReticleActor->SetActorHiddenInGame(true);
						}
					}
				}
			}
		}

		if (!bUsePersistentHitResults)
		{
			// 隐藏超出命中结果的Reticle
			if (TraceHitResults.Num() < ReticleActors.Num())
			{
				for (int32 j = TraceHitResults.Num(); j < ReticleActors.Num(); j++)
				{
					if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[j].Get())
					{
						LocalReticleActor->SetIsTargetAnActor(false);
						LocalReticleActor->SetActorHiddenInGame(true);
					}
				}
			}
		}

		// 如果没有命中结果，则添加一个默认的命中结果
		if (TraceHitResults.Num() < 1)
		{
			FHitResult HitResult;
			HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();
			HitResult.TraceEnd = TraceEnd;
			HitResult.Location = TraceEnd;
			HitResult.ImpactPoint = TraceEnd;
			TraceHitResults.Add(HitResult);

			if (bUsePersistentHitResults && PersistentHitResults.Num() < 1)
			{
				PersistentHitResults.Add(HitResult);
			}
		}

		ReturnHitResults.Append(TraceHitResults);
	}

	if (bUsePersistentHitResults && MaxHitResultsPerTrace > 0)
	{
		for (int32 PersistentHitResultIndex = 0; PersistentHitResultIndex < PersistentHitResults.Num(); PersistentHitResultIndex++)
		{
			FHitResult& HitResult = PersistentHitResults[PersistentHitResultIndex];

			// 更新TraceStart，因为旧的持久化HitResults将具有原始的TraceStart，并且玩家可能从那时起就移动了
			HitResult.TraceStart = StartLocation.GetTargetingTransform().GetLocation();
			if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[PersistentHitResultIndex].Get())
			{
				// 类同上面的逻辑
				if (const bool bHitActor = HitResult.GetActor() != nullptr; bHitActor && !HitResult.bBlockingHit)
				{
					LocalReticleActor->SetActorHiddenInGame(false);
					// 如果射线命中了一个Actor，并且Reticle的bSnapToTargetedActor为true，则将Reticle吸附到目标Actor的位置。
					const FVector ReticleLocation = (bHitActor && LocalReticleActor->bSnapToTargetedActor) ? HitResult.GetActor()->GetActorLocation() : HitResult.Location;
					LocalReticleActor->SetActorLocation(ReticleLocation);
					LocalReticleActor->SetIsTargetAnActor(bHitActor);
				}
				else
				{
					LocalReticleActor->SetActorHiddenInGame(true);
				}
			}
		}

		if (PersistentHitResults.Num() < ReticleActors.Num())
		{
			// 隐藏超出命中结果的Reticle
			for (int32 PersistentHitResultIndex = PersistentHitResults.Num(); PersistentHitResultIndex < ReticleActors.Num(); PersistentHitResultIndex++)
			{
				if (AGameplayAbilityWorldReticle* LocalReticleActor = ReticleActors[PersistentHitResultIndex].Get())
				{
					LocalReticleActor->SetIsTargetAnActor(false);
					LocalReticleActor->SetActorHiddenInGame(true);
				}
			}
		}

		return PersistentHitResults;
	}
	return ReturnHitResults;
}

AGameplayAbilityWorldReticle* AVTGATA_Trace::SpawnReticleActor(FVector Location, FRotator Rotation)
{
	if (ReticleClass)
	{
		if (AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(ReticleClass, Location, Rotation))
		{
			SpawnedReticleActor->InitializeReticle(this, PrimaryPC, ReticleParams);
			SpawnedReticleActor->SetActorHiddenInGame(true);
			ReticleActors.Add(SpawnedReticleActor);

			// This is to catch cases of playing on a listen server where we are using a replicated reticle actor.
			// (In a client controlled player, this would only run on the client and therefor never replicate. If it runs
			// on a listen server, the reticle actor may replicate. We want consistancy between client/listen server players.
			// Just saying 'make the reticle actor non replicated' isnt a good answer, since we want to mix and match reticle
			// actors and there may be other targeting types that want to replicate the same reticle actor class).
			// 
			// 这是为了处理在监听服务器上使用复制的指示器的情况。
			// （在客户端控制的玩家中，这只会运行在客户端上，因此永远不会复制。如果运行在监听服务器上，指示器可能会被复制。我们希望客户端和监听服务器玩家之间保持一致。
			// 仅仅说“让指示器不复制”并不是一个好的解决方案，因为我们希望能够混合使用指示器，并且可能还有其他目标类型希望复制相同的指示器类）。
			if (!ShouldProduceTargetDataOnServer)
			{
				SpawnedReticleActor->SetReplicates(false);
			}
			return SpawnedReticleActor;
		}
	}
	return nullptr;
}

void AVTGATA_Trace::DestroyReticleActors()
{
	for (int32 i = ReticleActors.Num() - 1; i >= 0; i--)
	{
		if (ReticleActors[i].IsValid())
		{
			ReticleActors[i].Get()->Destroy();
		}
	}

	ReticleActors.Empty();
}
