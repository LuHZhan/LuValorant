// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "GameplayTagContainer.h"
#include "VTAT_WaitTargetDataUsingActor.generated.h"

// 定义一个动态多播委托，用于传递目标数据引脚
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitTargetDataUsingActorDelegate, const FGameplayAbilityTargetDataHandle&, Data);

/**
 * 等待从已生成的 TargetActor 中获取目标数据，并且不会在接收到数据后销毁 TargetActor。
 *
 * 原始的 WaitTargetData 的注释建议我们对其进行大量子类化，但它的多数函数并不是虚函数。因此，这里是对它的完全重写，以添加 bCreateKeyIfNotValidForMorePredicting 功能。
 */
UCLASS()
class LUGAMEPLAYFRAME_API UVTAT_WaitTargetDataUsingActor : public UAbilityTask
{
	GENERATED_UCLASS_BODY()
	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataUsingActorDelegate ValidData;

	UPROPERTY(BlueprintAssignable)
	FWaitTargetDataUsingActorDelegate Cancelled;

	/**
	* 使用指定的衍生TargetActor并等待它返回有效数据或被取消。TargetActor不会被销毁
	*
	* @param bCreateKeyIfNotValidForMorePredicting 如果当前的作用域预测键（Scoped Prediction Key）对更多预测无效，则创建一个新的。
	* 如果为 false，则始终创建一个新的作用域预测键。如果我们希望使用可能存在的有效作用域预测键（例如批处理能力中的激活键），则应将此设置为 true。
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static UVTAT_WaitTargetDataUsingActor* WaitTargetDataWithReusableActor(
		UGameplayAbility* OwningAbility,
		FName TaskInstanceName,
		TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType,
		AGameplayAbilityTargetActor* InTargetActor,
		bool bCreateKeyIfNotValidForMorePredicting = false
	);

	virtual void Activate() override;

	// ---------------- Delegate ----------------

	UFUNCTION()
	virtual void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ActivationTag);

	UFUNCTION()
	virtual void OnTargetDataReplicatedCancelledCallback();

	UFUNCTION()
	virtual void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
	virtual void OnTargetDataCancelledCallback(const FGameplayAbilityTargetDataHandle& Data);

	// ================ Delegate ================

	/**
	 * 当从外部节点请求确认时调用
	 * @param bEndTask 
	 */
	virtual void ExternalConfirm(bool bEndTask) override;

	/**
	 * 当从外部节点请求取消时调用
	 */
	virtual void ExternalCancel() override;

protected:
	UPROPERTY()
	AGameplayAbilityTargetActor* TargetActor;

	// 是否在预测键无效时创建新的预测键
	bool bCreateKeyIfNotValidForMorePredicting;

	TEnumAsByte<EGameplayTargetingConfirmation::Type> ConfirmationType;

	// FDelegateHandle OnTargetDataReplicatedCallbackDelegateHandle;

	virtual void OnDestroy(bool AbilityEnded) override;

	/**
	 * 开始初始化 TargetActor
	 */
	virtual void InitializeTargetActor() const;
	/**
	 * 完成初始化 TargetActor
	 */
	virtual void FinalizeTargetActor() const;

	/**
	 * 注册Target到Delegate
	 */
	virtual void RegisterTargetDataCallbacks();

	/**
	 * 是否应将目标数据复制到服务器
	 * @return 
	 */
	virtual bool ShouldReplicateDataToServer() const;
};
