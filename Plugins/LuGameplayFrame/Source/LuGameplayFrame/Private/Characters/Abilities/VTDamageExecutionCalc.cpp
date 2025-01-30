// Copyright 2024 Dan Kestranek.


#include "Characters/Abilities/VTDamageExecutionCalc.h"
#include "..\..\..\Public\Characters\Abilities\AttributeSets\VTAttributeSetBase.h"
#include "Characters/Abilities/VTAbilitySystemComponent.h"

// Declare the attributes to capture and define how we want to capture them from the Source and Target.
struct VTDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	VTDamageStatics()
	{
		// 快照发生在创建GESpec时
		// 在这个例子中，我们没有从源中捕获任何东西，但是你可能会想要像AttackPower这样的属性。
		// 在ExecutionCalculation下的CalculationModifier中捕获可选的Damage
		DEFINE_ATTRIBUTE_CAPTUREDEF(UVTAttributeSetBase, Damage, Source, true);

		// 捕获目标的护甲。不要快照。
		DEFINE_ATTRIBUTE_CAPTUREDEF(UVTAttributeSetBase, Armor, Target, false);
	}
};

static const VTDamageStatics& DamageStatics()
{
	static VTDamageStatics DStatics;
	return DStatics;
}

UVTDamageExecutionCalc::UVTDamageExecutionCalc()
{
	HeadShotMultiplier = 1.5f;

	RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
}

void UVTDamageExecutionCalc::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	// 从源和目标收集标签
	FGameplayTagContainer AssetTags;
	Spec.GetAllAssetTags(AssetTags);
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// 载入Tags
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 计算护甲
	float Armor = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, Armor);
	Armor = FMath::Max<float>(Armor, 0.0f);

	// 计算伤害
	float Damage = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DamageDef, EvaluationParameters, Damage);
	Damage += FMath::Max<float>(Spec.GetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), false, -1.0f), 0.0f); // 如果SetByCaller存在，添加它的伤害

	float UnmitigatedDamage = Damage; 
	const FHitResult* Hit = Spec.GetContext().GetHitResult();
	if (AssetTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.CanHeadShot"))) && Hit && Hit->BoneName == "b_head")
	{
		UnmitigatedDamage *= HeadShotMultiplier;
		FGameplayEffectSpec* MutableSpec = ExecutionParams.GetOwningSpecForPreExecuteMod();
		MutableSpec->AddDynamicAssetTag(FGameplayTag::RequestGameplayTag(FName("Effect.Damage.HeadShot")));
	}

	float MitigatedDamage = (UnmitigatedDamage) * (100 / (100 + Armor));

	if (MitigatedDamage > 0.f)
	{
		// Set the Target's damage meta attribute
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(DamageStatics().DamageProperty, EGameplayModOp::Additive, MitigatedDamage));
	}
}
