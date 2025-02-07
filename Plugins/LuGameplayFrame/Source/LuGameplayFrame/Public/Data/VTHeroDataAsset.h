// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Characters/Abilities/VTAbilityTypes.h"
#include "Engine/DataAsset.h"
#include "VTHeroDataAsset.generated.h"

class UGameplayEffect;
class UVTGameplayAbility;
// struct FGameplayTag;

UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	None UMETA(DisplayName = "None"),
	Buff UMETA(DisplayName = "Buff"),
	Displacement UMETA(DisplayName = "Displacement"),
	Damage UMETA(DisplayName = "Damage"),
	Special UMETA(DisplayName = "Special")
};


USTRUCT(BlueprintType)
struct FAbilityBaseInfo
{
	GENERATED_BODY()

	FAbilityBaseInfo(): AbilityStartCount(0), bIsCooldownActive(false), CooldownDuration(0), AbilityType(EAbilityType::None), IconTexture(nullptr)
	{
		AbilityStateTags = {
			{EAbilityState::Cooldown, FGameplayTag{}},
			{EAbilityState::Silence, FGameplayTag::RequestGameplayTag(FName("DeBuff.Silence"))},
			{EAbilityState::Clear, FGameplayTag::RequestGameplayTag(FName("Clear"))},
		};
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int AbilityStartCount;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsCooldownActive;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bIsCooldownActive"))
	float CooldownDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAbilityType AbilityType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconTexture;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<TEnumAsByte<EAbilityState>, FGameplayTag> AbilityStateTags;
};

USTRUCT(BlueprintType)
struct FAbilityPerformanceInfo
{
	GENERATED_BODY()

	UPROPERTY()
	bool EditConditionTag = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag AbilityTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="EditConditionTag"))
	TSubclassOf<UVTGameplayAbility> Ability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="EditConditionTag"))
	TArray<TSubclassOf<UGameplayEffect>> Effects;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="EditConditionTag"))
	TSubclassOf<UGameplayEffect> CostEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="EditConditionTag"))
	TSubclassOf<UGameplayEffect> CooldownEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="EditConditionTag"))
	FGameplayTagContainer CueGameplayTagsTags;

	void IsAdvancedSettingsEnabled()
	{
		EditConditionTag = AbilityTag.IsValid();
	}
};

USTRUCT(BlueprintType)
struct FAbilityInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbilityPerformanceInfo PerformanceInfo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FAbilityBaseInfo BaseInfo;
};

USTRUCT(BlueprintType)
struct FHeroAbilityData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag HeroTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer HeroAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FAbilityInfo> AbilityInfoMap;

	FAbilityInfo GetFAbilityInfo(const FGameplayTag TargetAbilityTag);
	FAbilityBaseInfo GetFAbilityBaseInfo(const FGameplayTag TargetHeroTag, const FGameplayTag TargetAbilityTag);
	FAbilityPerformanceInfo GetFAbilityPerformanceInfo(const FGameplayTag TargetHeroTag, const FGameplayTag TargetAbilityTag);
	bool IsValid() const;
};

/**
 * 
 */
UCLASS(BlueprintType)
class LUGAMEPLAYFRAME_API UVTHeroDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FHeroAbilityData> HeroAbilityData;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Function")
	FHeroAbilityData GetHeroAbilityData(const FGameplayTag HeroTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Function")
	FAbilityBaseInfo GetBaseInfo(FGameplayTag HeroTag, FGameplayTag AbilityTag);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Function")
	FAbilityPerformanceInfo GetPerformanceInfo(FGameplayTag HeroTag, FGameplayTag AbilityTag);

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
};
