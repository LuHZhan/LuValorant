// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "VTAbilityTypes.h"
#include "Input/VTInputAction.h"
#include "Weapons/Interface/VTGeneralInterface.h"
#include "Library/BFLCommon.h"

#include "VTGameplayAbility.generated.h"

class USkeletalMeshComponent;

USTRUCT()
struct LUGAMEPLAYFRAME_API FAbilityMeshMontage
{
	GENERATED_BODY()

public:
	UPROPERTY()
	class USkeletalMeshComponent* Mesh;

	UPROPERTY()
	class UAnimMontage* Montage;

	FAbilityMeshMontage() : Mesh(nullptr), Montage(nullptr)
	{
	}

	FAbilityMeshMontage(class USkeletalMeshComponent* InMesh, class UAnimMontage* InMontage)
		: Mesh(InMesh), Montage(InMontage)
	{
	}
};

/**
 * 
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTGameplayAbility : public UGameplayAbility, public IVTGeneralInterface
{
	GENERATED_BODY()

public:
	UVTGameplayAbility();

	// // Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EVTAbilityInputID AbilityInputID = EVTAbilityInputID::None;
	//
	// // Value to associate an ability with an slot without tying it to an automatically activated input.
	// // Passive abilities won't be tied to an input so we need a way to generically associate abilities with slots.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	EVTAbilityInputID AbilityID = EVTAbilityInputID::None;

	// Tells an ability to activate immediately when its granted
	// Used for passive abilities and abilites forced on others.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateAbilityOnGranted;

	/** 是否启用输入激活GA */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateOnInput;

	/** 限制玩家在Interacting时不能激活GA */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bCannotActivateWhileInteracting;

	// ---------------- Input ----------------

	/** 绑定IA */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Input", meta=(EditCondition="bIsOpenAction"))
	UVTInputAction* InputAction;

	/** IA触发方式 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Input", meta=(EditCondition="bIsOpenAction"))
	ETriggerEvent ActionTrigger;

	/** 是否开启IA触发 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability|Input")
	bool bIsOpenAction = true;

	/** 默认触发当前GA */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void IATriggerEvent(const FInputActionValue& Value);

	/** 接口 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void IACompletedEvent(const FInputActionValue& Value);

	/** 接口 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void IACanceledEvent(const FInputActionValue& Value);

	// ================ Input ================

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffects")
	TMap<FGameplayTag, FVTGameplayEffectContainer> EffectContainerMap;
	
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Ability")
	void OnPostOnAvatarSet(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpec& Spec);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Ability")
	void OnPostOnGiveAbility(FGameplayAbilityActorInfo ActorInfo, const FGameplayAbilitySpec& Spec);
	
	UFUNCTION(BlueprintCallable, Category = "Ability")
	FGameplayAbilityTargetDataHandle MakeGameplayAbilityTargetDataHandleFromActorArray(const TArray<AActor*> TargetActors);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	FGameplayAbilityTargetDataHandle MakeGameplayAbilityTargetDataHandleFromHitResults(const TArray<FHitResult> HitResults);


	/**
	 * Create FVTGameplayEffectContainerSpec from FGameplayTag
	 * @param ContainerTag 
	 * @param EventData 
	 * @param OverrideGameplayLevel 
	 * @return 
	 */
	UFUNCTION(BlueprintCallable, Category = Ability, meta = (AutoCreateRefTerm = "EventData"))
	virtual FVTGameplayEffectContainerSpec MakeEffectContainerSpec(FGameplayTag ContainerTag, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);

	/**
	 * Create FVTGameplayEffectContainerSpec 
	 */
	virtual FVTGameplayEffectContainerSpec MakeEffectContainerSpecFromContainer(const FVTGameplayEffectContainer& Container, const FGameplayEventData& EventData, int32 OverrideGameplayLevel = -1);


	// Applies a gameplay effect container spec that was previously created
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual TArray<FActiveGameplayEffectHandle> ApplyEffectContainerSpec(const FVTGameplayEffectContainerSpec& ContainerSpec);

	// Expose GetSourceObject to Blueprint. Retrieves the SourceObject associated with this ability. Callable on non instanced abilities.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability", meta = (DisplayName = "Get Source Object"))
	UObject* K2_GetSourceObject(FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	// Attempts to activate the given ability handle and batch all RPCs into one. This will only batch all RPCs that happen
	// in one frame. Best case scenario we batch ActivateAbility, SendTargetData, and EndAbility into one RPC instead of three.
	// 尝试激活给定的技能，将所有 RPC 处理并批处理为一个。这只会对一帧中发生的所有 RPC 进行批处理。最佳情况是将 ActivateAbility、SendTargetData 和 EndAbility 批处理到一个 RPC 中，而不是三个。
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool BatchRPCTryActivateAbility(FGameplayAbilitySpecHandle InAbilityHandle, bool EndAbilityImmediately);

	// Same as calling K2_EndAbility. Meant for use with batching system to end the ability externally.
	virtual void ExternalEndAbility();

	// Returns the current prediction key and if it's valid for more predicting. Used for debugging ability prediction windows.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual FString GetCurrentPredictionKeyStatus();

	// Returns if the current prediction key is valid for more predicting.
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability")
	virtual bool IsPredictionKeyValidForMorePrediction() const;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr,
	                                const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	virtual bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

	// Allows C++ and Blueprint abilities to override how cost is checked in case they don't use a GE like weapon ammo
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	bool CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;
	virtual bool CheckCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo) const;

	virtual void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const override;

	// Allows C++ and Blueprint abilities to override how cost is applied in case they don't use a GE like weapon ammo
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Ability")
	void ApplyCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const;

	virtual void ApplyCost_Implementation(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo& ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
	{
	};

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void SetHUDReticle(TSubclassOf<class UVTHUDReticle> ReticleClass);

	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void ResetHUDReticle();

	// Sends TargetData from the client to the Server and creates a new Prediction Window
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual void SendTargetDataToServer(const FGameplayAbilityTargetDataHandle& TargetData);

	// Is the player's input currently pressed? Only works if the ability is bound to input.
	UFUNCTION(BlueprintCallable, Category = "Ability")
	virtual bool IsInputPressed() const;


	// ----------------------------------------------------------------------------------------------------------------
	//	Animation Support for multiple USkeletalMeshComponents on the AvatarActor
	// ----------------------------------------------------------------------------------------------------------------

	/** Returns the currently playing montage for this ability, if any */
	UFUNCTION(BlueprintCallable, Category = Animation)
	UAnimMontage* GetCurrentMontageForMesh(USkeletalMeshComponent* InMesh);

	/** Call to set/get the current montage from a montage task. Set to allow hooking up montage events to ability events */
	virtual void SetCurrentMontageForMesh(USkeletalMeshComponent* InMesh, class UAnimMontage* InCurrentMontage);

	// ---------------- IVTGeneralInterface ----------------
	
	UFUNCTION(BlueprintCallable, Category="IVTGeneralInterface")
	virtual AVTHeroCharacter* GetPawn() const override;
	
	UFUNCTION(BlueprintCallable, Category="IVTGeneralInterface")
	virtual UVTGameplayAbility* GetAbilityInstanceFromHandle(FGameplayAbilitySpecHandle InHandle) const override;
	
	UFUNCTION(BlueprintCallable, Category="IVTGeneralInterface")
	virtual UVTGameplayAbility* GetAbilityInstanceFromClass(TSubclassOf<UGameplayAbility> InAbilityClass) const override;

	UFUNCTION(BlueprintCallable, Category="IVTGeneralInterface")
	virtual FGameplayAbilitySpecHandle GetAbilitySpecHandleFromClass(TSubclassOf<UGameplayAbility> InAbilityClass) const override;

	UFUNCTION(BlueprintCallable, Category="IVTGeneralInterface")
	virtual bool IsPrimaryAbilityInstanceActive(FGameplayAbilitySpecHandle Handle) const override;

	UFUNCTION(BlueprintCallable,BlueprintPure,Category="IVTGeneralInterface")
	virtual bool GASpecHandleIsValid(FGameplayAbilitySpecHandle SpecHandle) const override;

	UFUNCTION(BlueprintCallable,BlueprintPure,Category="IVTGeneralInterface")
	virtual bool IsAbilityActive() const override;

	// ================ IVTGeneralInterface ================
	
protected:
	FGameplayTag InteractingTag;
	FGameplayTag InteractingRemovalTag;


	// ----------------------------------------------------------------------------------------------------------------
	//	Animation Support for multiple USkeletalMeshComponents on the AvatarActor
	// ----------------------------------------------------------------------------------------------------------------

	/** Active montages being played by this ability */
	UPROPERTY()
	TArray<FAbilityMeshMontage> CurrentAbilityMeshMontages;

	bool FindAbilityMeshMontage(USkeletalMeshComponent* InMesh, FAbilityMeshMontage& InAbilityMontage);

	/** Immediately jumps the active montage to a section */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageJumpToSectionForMesh(USkeletalMeshComponent* InMesh, FName SectionName);

	/** Sets pending section on active montage */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation")
	void MontageSetNextSectionNameForMesh(USkeletalMeshComponent* InMesh, FName FromSectionName, FName ToSectionName);

	/**
	 * Stops the current animation montage.
	 *
	 * @param OverrideBlendOutTime If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation", Meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStopForMesh(USkeletalMeshComponent* InMesh, float OverrideBlendOutTime = -1.0f);

	/**
	* Stops all currently animating montages
	*
	* @param OverrideBlendOutTime If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
	 */
	UFUNCTION(BlueprintCallable, Category = "Ability|Animation", Meta = (AdvancedDisplay = "OverrideBlendOutTime"))
	void MontageStopForAllMeshes(float OverrideBlendOutTime = -1.0f);
};
