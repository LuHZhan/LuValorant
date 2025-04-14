## UPrimitiveComponent

### UpdateOverlapsImpl

```cpp
// NewPendingOverlaps -- 碰撞查询中检测到的将要碰到的重叠
// bDoNotifies -- 是否触发重叠事件
// OverlapsAtEndLocation -- 存有Self组件当前位置（查询末端位置）的所有重叠，只有self组件是Actor的根组件时才使用这个数组
bool UPrimitiveComponent::UpdateOverlapsImpl(const TOverlapArrayView* NewPendingOverlaps, bool bDoNotifies, const TOverlapArrayView* OverlapsAtEndLocation)
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateOverlaps); 
	SCOPE_CYCLE_UOBJECT(ComponentScope, this);

	// Begin 检测
	const AActor* const MyActor = GetOwner();
	if (MyActor && !MyActor->HasActorBegunPlay() && !MyActor->IsActorBeginningPlay())
	{
		return false;
	}
    // ...

	bool bCanSkipUpdateOverlaps = true;

	// Collision和GenerateEvent 检测
	if (GetGenerateOverlapEvents() && IsQueryCollisionEnabled())	
	{
		bCanSkipUpdateOverlaps = false;
		if (MyActor)
		{
			const FTransform PrevTransform = GetComponentTransform();
			// 是否为根组件，根组件始终不会考虑子组件的影响
			const bool bIgnoreChildren = (MyActor->GetRootComponent() == this);
			// ...
			if (NewPendingOverlaps)
			{
                // 触发所有BeginComponentOverlap
				const int32 NumNewPendingOverlaps = NewPendingOverlaps->Num();
				for (int32 Idx=0; Idx < NumNewPendingOverlaps; ++Idx)
				{
					BeginComponentOverlap( (*NewPendingOverlaps)[Idx], bDoNotifies );
				}
                // ...
			}

			// OverlapMultiResult：保存在新的位置重新重叠检测的结果
            // NewOverlappingComponentPtrs：保存当前重叠的指针
			TInlineOverlapInfoArray OverlapMultiResult;
			TInlineOverlapPointerArray NewOverlappingComponentPtrs;
			// ...
            
            // 更新重叠
			if (IsValid(this) && GetGenerateOverlapEvents())
			{
                // 在Trans不变的情况下，使用现有的缓存重叠结果
				// PrimitiveComponentCVars::bAllowCachedOverlapsCVar 允许使用缓存的重叠结果
				if (OverlapsAtEndLocation != nullptr && PrimitiveComponentCVars::bAllowCachedOverlapsCVar && PrevTransform.Equals(GetComponentTransform()))
                // ...
				{
					UE_LOG(LogPrimitiveComponent, VeryVerbose, TEXT("%s->%s Skipping overlap test!"), *GetNameSafe(GetOwner()), *GetName());
                    /* true表示存在新的待处理重叠 */
					const bool bCheckForInvalid = (NewPendingOverlaps && NewPendingOverlaps->Num() > 0);
					if (bCheckForInvalid)
					{
                        // GetPointersToArrayDataByPredicate 是一个谓语函数，用于过滤每个重叠是否依旧有效
                        // 一个典型的场景是：
                        // - 组件A移动并与组件B产生新的重叠
                        // - 在处理这个重叠时，某些事件处理器改变了组件A或B的碰撞设置[ECollisionResponse]或者重叠时间生						 //   成设置[bGenerateOverlapEvents]
                        // - 由于设置变化，A和B实际上不应该再产生重叠事件
                        // -- 这时就需要过滤掉这种无效的重叠，避免错误的重叠事件
						// NewOverlappingComponentPtrs 将会作为输出存储过滤后的结果
						GetPointersToArrayDataByPredicate(NewOverlappingComponentPtrs, *OverlapsAtEndLocation, FPredicateFilterCanOverlap(*this));
                        // ...
					}
					else
					{
                        /* 直接使用缓存重叠结果 */
						GetPointersToArrayData(NewOverlappingComponentPtrs, *OverlapsAtEndLocation);
					}
				}
                // Self组件有移动的情况（或OverlapsAtEndLocation为空的情况）
				else
				{
					SCOPE_CYCLE_COUNTER(STAT_PerformOverlapQuery);
					UE_LOG(LogPrimitiveComponent, VeryVerbose, TEXT("%s->%s Performing overlaps!"), *GetNameSafe(GetOwner()), *GetName());
					UWorld* const MyWorld = GetWorld();
					TArray<FOverlapResult> Overlaps;
					/* 根据bIgnoreChildren选择性地包括Component Overlaps */
					FComponentQueryParams Params(SCENE_QUERY_STAT(UpdateOverlaps), bIgnoreChildren ? MyActor : nullptr);
					Params.bIgnoreBlocks = true;	
					FCollisionResponseParams ResponseParam;
					InitSweepCollisionParams(Params, ResponseParam);
                    /* 获取真实的重叠查询填充到Overlaps */
					ComponentOverlapMulti(Overlaps, MyWorld, GetComponentLocation(), GetComponentQuat(), GetCollisionObjectType(), Params);

                    // 从Overlaps中将结果填充到OverlapMultiResult
					for (int32 ResultIdx=0; ResultIdx < Overlaps.Num(); ResultIdx++)
					{
						const FOverlapResult& Result = Overlaps[ResultIdx];

						UPrimitiveComponent* const HitComp = Result.Component.Get();
						if (HitComp && (HitComp != this) && HitComp->GetGenerateOverlapEvents())
						{
							const bool bCheckOverlapFlags = false; 
                            /* 除去常规检测之外，会忽略对于WorldSettings和未完成初始化的Actor的重叠 */
							if (!ShouldIgnoreOverlapResult(MyWorld, MyActor, *this, Result.OverlapObjectHandle, *HitComp, bCheckOverlapFlags))
							{
                                /* 填充重叠 */
								OverlapMultiResult.Emplace(HitComp, Result.ItemIndex);		
							}
						}
					}
                    // ...

					/* 将结果填充到NewOverlappingComponentPtrs中 */
					GetPointersToArrayData(NewOverlappingComponentPtrs, OverlapMultiResult);
				}
			}
            // ...

			// 对比新旧缓存
			if (OverlappingComponents.Num() > 0)
			{
                // OverlappingComponents数组里都是旧重叠，将旧重叠存储到OldOverlappingComponentPtrs
				TInlineOverlapPointerArray OldOverlappingComponentPtrs;
				if (bIgnoreChildren)
				{
					GetPointersToArrayDataByPredicate(OldOverlappingComponentPtrs, OverlappingComponents, FPredicateOverlapHasDifferentActor(*MyActor));
				}
				else
				{
					GetPointersToArrayData(OldOverlappingComponentPtrs, OverlappingComponents);
				}
                // ...

				// 去除这两个数组重复的部分，我们就可以筛选出 过时的重叠 和 新的需要触发重叠事件的重叠
				for (int32 CompIdx=0; CompIdx < OldOverlappingComponentPtrs.Num() && NewOverlappingComponentPtrs.Num() > 0; ++CompIdx)
				{
					const FOverlapInfo* SearchItem = OldOverlappingComponentPtrs[CompIdx];
					const int32 NewElementIdx = IndexOfOverlapFast(NewOverlappingComponentPtrs, SearchItem);
					if (NewElementIdx != INDEX_NONE)
					{
						NewOverlappingComponentPtrs.RemoveAtSwap(NewElementIdx, 1, EAllowShrinking::No);
						OldOverlappingComponentPtrs.RemoveAtSwap(CompIdx, 1, EAllowShrinking::No);
						--CompIdx;
					}
				}
                // ...

                // 对所有过时的重叠执行EndComponentOverlap
				const int32 NumOldOverlaps = OldOverlappingComponentPtrs.Num();
				if (NumOldOverlaps > 0)
				{
                    // 将OldOverlappingComponentPtrs的结果转存到OldOverlappingComponents
					TInlineOverlapInfoArray OldOverlappingComponents;
					OldOverlappingComponents.SetNumUninitialized(NumOldOverlaps);
					for (int32 i=0; i < NumOldOverlaps; i++)
					{
						OldOverlappingComponents[i] = *(OldOverlappingComponentPtrs[i]);
					}
                    // ...
					
                    // EndComponentOverlap处理
					for (const FOverlapInfo& OtherOverlap : OldOverlappingComponents)
					{
						if (OtherOverlap.OverlapInfo.Component.IsValid())
						{
							EndComponentOverlap(OtherOverlap, bDoNotifies, false);
						}
                        /* 当内存变大时才回收内存，以尽量避免混乱，同时避免增加组件的内存使用 */
						else
						{
							const bool bAllowShrinking = (OverlappingComponents.Max() >= 24);
							const int32 StaleElementIndex = IndexOfOverlapFast(OverlappingComponents, OtherOverlap);
							if (StaleElementIndex != INDEX_NONE)
							{
                                /* 使用 RemoveAtSwap 方法移除元素，这是一个高效的移除方法，将最后一个元素移动到被移除元素									的位置，避免整个数组的移动 */
								OverlappingComponents.RemoveAtSwap(StaleElementIndex, 1, bAllowShrinking ? EAllowShrinking::Yes : EAllowShrinking::No);
							}
						}
					}
                    // ...
				}
			}

			/* 这段代码是一种防御性编程技术，确保在使用指向局部变量的指针时，这些局部变量仍然在有效的作用域内，从而避免潜在的内存			   错误 */
			static_assert(sizeof(OverlapMultiResult) != 0, "Variable must be in this scope");
			static_assert(sizeof(*OverlapsAtEndLocation) != 0, "Variable must be in this scope");

			// 针对新的重叠调用BeingComponentOverlap
			for (const FOverlapInfo* NewOverlap : NewOverlappingComponentPtrs)
			{
                /* 这里调用的基于ComponentOverlapMulti获得新重叠 */
				BeginComponentOverlap(*NewOverlap, bDoNotifies);
			}
            // ...
		}
	}
    // 结束所有存在的重叠
	else
	{
		if (OverlappingComponents.Num() > 0)
		{
			const bool bSkipNotifySelf = false;
            /* 需要将这些重叠全部处理掉，也就是一一调用EndComponentOverlap */
			ClearComponentOverlaps(bDoNotifies, bSkipNotifySelf);
		}
	}
    // ...

	// 在遍历子组件之前，先缓存了一份子组件
    // 是因为子组件更新重叠的过程中，可能会自己脱离父组件，导致循环出现BUG
	TInlineComponentArray<USceneComponent*> AttachedChildren;
	AttachedChildren.Append(GetAttachChildren());

	for (USceneComponent* const ChildComp : AttachedChildren)
	{
		if (ChildComp)
		{
			// 对所有的子组件调用UpdateOverlaps
			bCanSkipUpdateOverlaps &= ChildComp->UpdateOverlaps(nullptr, bDoNotifies, nullptr);
		}
	}

	// Update physics volume using most current overlaps
	if (GetShouldUpdatePhysicsVolume())
	{
		UpdatePhysicsVolume(bDoNotifies);
        /* 将 bCanSkipUpdateOverlaps 设置为 false ，表示不能跳过重叠更新 */
		bCanSkipUpdateOverlaps = false;
	}

	return bCanSkipUpdateOverlaps;
}
```



####  根组件跳过子组件的重叠查询

断点调试发现，根组件在调用UpdateOverlaps的NewPendingOverlaps数组中，并没有任何子组件，哪怕子组件碰撞全开。

往上追溯，才发现UPrimitiveComponent::MoveComponentImpl里在重叠检测时还藏了一手：

```c++
			FComponentQueryParams Params(SCENE_QUERY_STAT(MoveComponent), Actor);
			FCollisionResponseParams ResponseParam;
			InitSweepCollisionParams(Params, ResponseParam);
			Params.bIgnoreTouches |= !(GetGenerateOverlapEvents() || bForceGatherOverlaps);
			Params.TraceTag = TraceTagName;
			bool const bHadBlockingHit = MyWorld->ComponentSweepMulti(Hits, this, TraceStart, TraceEnd, InitialRotationQuat, Params);
```

FComponentQueryParams Params的第二个参数就是要忽略的Actor，这里的Actor指的就是本身，所以检测的结果自然就没有自己的子组件了。不过即便如此，如果子组件在碰撞上允许和根组件生成重叠事件时，在子组件的UpdateOverlaps还是不可避免地与根组件发生重叠关系。不过UE的注释里都提到了，这都是为了优化MovementCompoennt的移动流程。



### BeginComponentOverlap

```cpp
void UPrimitiveComponent::BeginComponentOverlap(const FOverlapInfo& OtherOverlap, bool bDoNotifies)
{
	SCOPE_CYCLE_COUNTER(STAT_BeginComponentOverlap);

	if (!IsValid(this))
	{
		return;
	}
    
	// 避免Component重复触发重叠事件，OverlappingComponents是与当前组件重叠的其他组件，IndexOfOverlapFast是一个查询函数
	const bool bComponentsAlreadyTouching = (IndexOfOverlapFast(OverlappingComponents, OtherOverlap) != INDEX_NONE);
	if (!bComponentsAlreadyTouching)
	{
		UPrimitiveComponent* OtherComp = OtherOverlap.OverlapInfo.Component.Get();
		if (CanComponentsGenerateOverlap(this, OtherComp))
		{
            // 记录当前是否是Actor内部的碰撞，如果不是，检测当前重叠的两个Actor是否之前已重叠
			GlobalOverlapEventsCounter++;			
			AActor* const OtherActor = OtherComp->GetOwner();
			AActor* const MyActor = GetOwner();

			const bool bSameActor = (MyActor == OtherActor);
			const bool bNotifyActorTouch = bDoNotifies && !bSameActor && !AreActorsOverlapping(*MyActor, *OtherActor);
			// ...
            
			// 将当前的OtherOverlap载入Overlapping，对OtherComp也执行相同的操作
			OverlappingComponents.Add(OtherOverlap);												 
			AddUniqueOverlapFast(OtherComp->OverlappingComponents, FOverlapInfo(this, INDEX_NONE));	
			// ...
            
            // 控制何时应该触发重叠事件通知
            // - 正常游戏过程中 ：当游戏世界已经开始运行时，正常触发重叠事件。
			// - 关卡流送过程中 ：即使游戏世界尚未完全开始，如果Actor配置为在关卡流送过程中生成重叠事件，也会触发重叠通知
			const UWorld* World = GetWorld();
			const bool bLevelStreamingOverlap = (bDoNotifies && MyActor->bGenerateOverlapEventsDuringLevelStreaming && MyActor->IsActorBeginningPlayFromLevelStreaming());
			if (bDoNotifies && ((World && World->HasBegunPlay()) || bLevelStreamingOverlap))
            // ...
			{
				// 先触发selfOnComponentBeginOverlap蓝图委托
				if (IsValid(this))
				{
					OnComponentBeginOverlap.Broadcast(this, OtherActor, OtherComp, OtherOverlap.GetBodyIndex(), OtherOverlap.bFromSweep, OtherOverlap.OverlapInfo);
				}

				if (IsValid(OtherComp))
				{
					// 因为在扫描碰撞中，碰撞信息（如法线方向）是相对于发起扫描的组件的。但对于被碰撞的组件来说，这些信息需要反转才能正确表示从它的角度看到的碰撞情况。所以当 OtherOverlap.bFromSweep 为真时，使用 FHitResult::GetReversedHit 反转碰撞信息；否则直接使用原始碰撞信息。
					OtherComp->OnComponentBeginOverlap.Broadcast(OtherComp, MyActor, this, INDEX_NONE, OtherOverlap.bFromSweep, OtherOverlap.bFromSweep ? FHitResult::GetReversedHit(OtherOverlap.OverlapInfo) : OtherOverlap.OverlapInfo);
				}

				// 对于两个不同且之前没有重叠的Actor执行委托
				if (bNotifyActorTouch)
				{
					// Cpp Notify
					if (IsActorValidToNotify(MyActor))
					{
						MyActor->NotifyActorBeginOverlap(OtherActor);
					}

					if (IsActorValidToNotify(OtherActor))
					{
						OtherActor->NotifyActorBeginOverlap(MyActor);
					}

					// Blueprint Deleagate
					if (IsActorValidToNotify(MyActor))
					{
						MyActor->OnActorBeginOverlap.Broadcast(MyActor, OtherActor);
					}

					if (IsActorValidToNotify(OtherActor))
					{
						OtherActor->OnActorBeginOverlap.Broadcast(OtherActor, MyActor);
					}
				}
                // ...
			}
		}
	}
}
```



### EndComponentOverlap

```Cpp
void UPrimitiveComponent::EndComponentOverlap(const FOverlapInfo& OtherOverlap, bool bDoNotifies, bool bSkipNotifySelf)
{
	SCOPE_CYCLE_COUNTER(STAT_EndComponentOverlap);

	UPrimitiveComponent* OtherComp = OtherOverlap.OverlapInfo.Component.Get();
	if (OtherComp == nullptr)
	{
		return;
	}

	const int32 OtherOverlapIdx = IndexOfOverlapFast(OtherComp->OverlappingComponents, FOverlapInfo(this, INDEX_NONE));
	if (OtherOverlapIdx != INDEX_NONE)
	{
		OtherComp->OverlappingComponents.RemoveAtSwap(OtherOverlapIdx, 1, EAllowShrinking::No);
	}

	const int32 OverlapIdx = IndexOfOverlapFast(OverlappingComponents, OtherOverlap);
	if (OverlapIdx != INDEX_NONE)
	{
		//UE_LOG(LogActor, Log, TEXT("END OVERLAP! Self=%s SelfComp=%s, Other=%s, OtherComp=%s"), *GetNameSafe(this), *GetNameSafe(MyComp), *GetNameSafe(OtherActor), *GetNameSafe(OtherComp));
		GlobalOverlapEventsCounter++;
		OverlappingComponents.RemoveAtSwap(OverlapIdx, 1, EAllowShrinking::No);

		AActor* const MyActor = GetOwner();
		const UWorld* World = GetWorld();
		const bool bLevelStreamingOverlap = (bDoNotifies && MyActor && MyActor->bGenerateOverlapEventsDuringLevelStreaming && MyActor->IsActorBeginningPlayFromLevelStreaming());
		if (bDoNotifies && ((World && World->HasBegunPlay()) || bLevelStreamingOverlap))
		{
			AActor* const OtherActor = OtherComp->GetOwner();
			if (OtherActor)
			{
				if (!bSkipNotifySelf && IsPrimCompValidAndAlive(this))
				{
					OnComponentEndOverlap.Broadcast(this, OtherActor, OtherComp, OtherOverlap.GetBodyIndex());
				}

				if (IsPrimCompValidAndAlive(OtherComp))
				{
					OtherComp->OnComponentEndOverlap.Broadcast(OtherComp, MyActor, this, INDEX_NONE);
				}
	
				// if this was the last touch on the other actor by this actor, notify that we've untouched the actor as well
				const bool bSameActor = (MyActor == OtherActor);
				if (MyActor && !bSameActor && !AreActorsOverlapping(*MyActor, *OtherActor))
				{			
					if (IsActorValidToNotify(MyActor))
					{
						MyActor->NotifyActorEndOverlap(OtherActor);
						MyActor->OnActorEndOverlap.Broadcast(MyActor, OtherActor);
					}

					if (IsActorValidToNotify(OtherActor))
					{
						OtherActor->NotifyActorEndOverlap(MyActor);
						OtherActor->OnActorEndOverlap.Broadcast(OtherActor, MyActor);
					}
				}
			}
		}
	}
}
```

