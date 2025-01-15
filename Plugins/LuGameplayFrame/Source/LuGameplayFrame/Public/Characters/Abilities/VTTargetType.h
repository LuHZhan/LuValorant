// Copyright 2024 Dan Kestranek.

#pragma once

#include "Abilities/GameplayAbilityTypes.h"
#include "VTAbilityTypes.h"
#include "VTTargetType.generated.h"

class AVTCharacterBase;


class AActor;
struct FGameplayEventData;


/**
 * Target Class
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class GASSHOOTER_API UVTTargetType : public UObject
{
	GENERATED_BODY()

public:
	UVTTargetType() {}

	// 确定GE的对象
	UFUNCTION(BlueprintNativeEvent)
	void GetTargets(AVTCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
	virtual void GetTargets_Implementation(AVTCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};

/** Trivial Target：使用Owner */
UCLASS(NotBlueprintable)
class GASSHOOTER_API UVTTargetType_UseOwner : public UVTTargetType
{
	GENERATED_BODY()

public:
	UVTTargetType_UseOwner() {}
	
	virtual void GetTargets_Implementation(AVTCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};

/** Event Trivial Target：从Event Data获取Target */
UCLASS(NotBlueprintable)
class GASSHOOTER_API UVTTargetType_UseEventData : public UVTTargetType
{
	GENERATED_BODY()

public:
	UVTTargetType_UseEventData() {}

	virtual void GetTargets_Implementation(AVTCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};
