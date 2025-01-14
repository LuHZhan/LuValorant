// Copyright 2024 Dan Kestranek.

#pragma once

#include "Abilities/GameplayAbilityTypes.h"
#include "Characters/Abilities/GSAbilityTypes.h"
#include "GSTargetType.generated.h"

class AVTCharacterBase;


class AActor;
struct FGameplayEventData;


/**
 * Target Class
 */
UCLASS(Blueprintable, meta = (ShowWorldContextPin))
class GASSHOOTER_API UGSTargetType : public UObject
{
	GENERATED_BODY()

public:
	UGSTargetType() {}

	// 确定GE的对象
	UFUNCTION(BlueprintNativeEvent)
	void GetTargets(AVTCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
	virtual void GetTargets_Implementation(AVTCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const;
};

/** Trivial Target：使用Owner */
UCLASS(NotBlueprintable)
class GASSHOOTER_API UGSTargetType_UseOwner : public UGSTargetType
{
	GENERATED_BODY()

public:
	UGSTargetType_UseOwner() {}
	
	virtual void GetTargets_Implementation(AVTCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};

/** Event Trivial Target：从Event Data获取Target */
UCLASS(NotBlueprintable)
class GASSHOOTER_API UGSTargetType_UseEventData : public UGSTargetType
{
	GENERATED_BODY()

public:
	UGSTargetType_UseEventData() {}

	virtual void GetTargets_Implementation(AVTCharacterBase* TargetingCharacter, AActor* TargetingActor, FGameplayEventData EventData, TArray<FGameplayAbilityTargetDataHandle>& OutTargetData, TArray<FHitResult>& OutHitResults, TArray<AActor*>& OutActors) const override;
};
