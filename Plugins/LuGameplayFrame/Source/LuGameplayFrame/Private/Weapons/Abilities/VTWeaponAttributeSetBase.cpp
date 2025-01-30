// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Abilities/VTWeaponAttributeSetBase.h"

#include "GameplayEffectExtension.h"
#include "Characters/VTCharacterBase.h"

UVTWeaponAttributeSetBase::UVTWeaponAttributeSetBase()
{
}

// using PreAttributeChangeFunc = void(*)(const FGameplayAttribute&, float&);
// TMap<FGameplayAttribute, PreAttributeChangeFunc> UWeaponAttributeSet::MessageHandlerMap = {
//
// };
void UVTWeaponAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	// if (Attribute == GetBodyDamageAttribute())
	// {
	// 	SetBodyDamage(FMath::Clamp(NewValue, MinDamage, MaxDamage));
	// }
	// else if (Attribute == GetHeadDamageAttribute())
	// {
	// 	SetHeadDamage(FMath::Clamp(NewValue, MinDamage, MaxDamage));
	// }
}

void UVTWeaponAttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
}

bool UVTWeaponAttributeSetBase::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	return Super::PreGameplayEffectExecute(Data);
}

void UVTWeaponAttributeSetBase::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayEffectContextHandle Context = Data.EffectSpec.GetContext();
	UAbilitySystemComponent* Source = Context.GetOriginalInstigatorAbilitySystemComponent();
	const FGameplayTagContainer& SourceTags = *Data.EffectSpec.CapturedSourceTags.GetAggregatedTags();

	FGameplayTagContainer SpecAssetTags;
	Data.EffectSpec.GetAllAssetTags(SpecAssetTags);

	// 获取TargetInfo
	AActor* TargetActor = nullptr;
	const AController* TargetController = nullptr;
	AVTCharacterBase* TargetCharacter = nullptr;
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		TargetActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		TargetCharacter = Cast<AVTCharacterBase>(TargetActor);
	}

	// 获取SourceInfo
	AActor* SourceActor = nullptr;
	AController* SourceController = nullptr;
	if (Source && Source->AbilityActorInfo.IsValid() && Source->AbilityActorInfo->AvatarActor.IsValid())
	{
		// 可能是这两个都可能获取为空
		SourceActor = Source->AbilityActorInfo->AvatarActor.Get();
		SourceController = Source->AbilityActorInfo->PlayerController.Get();

		// 更新Controller为SourceActor
		if (SourceController == nullptr && SourceActor != nullptr)
		{
			if (const APawn* Pawn = Cast<APawn>(SourceActor))
			{
				SourceController = Pawn->GetController();
			}
		}

		if (Context.GetEffectCauser())
		{
			SourceActor = Context.GetEffectCauser();
		}
	}

	// if (Data.EvaluatedData.Attribute == GetCurrentDamageAttribute())
	// {
	// 	// 保留备份
	// 	const float LocalDamageDone = GetCurrentDamage();
	// 	SetCurrentDamage(0.f);
	//
	// 	if (LocalDamageDone > 0.0f)
	// 	{
	// 		const bool WasAlive = TargetCharacter ? TargetCharacter->IsAlive() : false;
	// 		
	// 		const float OldShield = GetShield();
	// 		float DamageAfterShield = LocalDamageDone - OldShield;
	// 		if (OldShield > 0)
	// 		{
	// 			float NewShield = OldShield - LocalDamageDone;
	// 			SetShield(FMath::Clamp<float>(NewShield, 0.0f, GetMaxShield()));
	// 		}
	//
	// 		if (DamageAfterShield > 0)
	// 		{
	// 			// Apply the health change and then clamp it
	// 			const float NewHealth = GetHealth() - DamageAfterShield;
	// 			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));
	// 		}
	//
	// 		if (TargetCharacter && WasAlive)
	// 		{
	// 			// This is the log statement for damage received. Turned off for live games.
	// 			//UE_LOG(LogTemp, Log, TEXT("%s() %s Damage Received: %f"), *FString(__FUNCTION__), *GetOwningActor()->GetName(), LocalDamageDone);
	//
	// 			// Show damage number for the Source player unless it was self damage
	// 			if (SourceActor != TargetActor)
	// 			{
	// 				AVTPlayerController* PC = Cast<AVTPlayerController>(SourceController);
	// 				if (PC)
	// 				{
	// 					FGameplayTagContainer DamageNumberTags;
	//
	// 					if (Data.EffectSpec.GetDynamicAssetTags().HasTag(HeadShotTag))
	// 					{
	// 						DamageNumberTags.AddTagFast(HeadShotTag);
	// 					}
	//
	// 					PC->ShowDamageNumber(LocalDamageDone, TargetCharacter, DamageNumberTags);
	// 				}
	// 			}
	//
	// 			if (!TargetCharacter->IsAlive())
	// 			{
	// 				// TargetCharacter was alive before this damage and now is not alive, give XP and Gold bounties to Source.
	// 				// Don't give bounty to self.
	// 				if (SourceController != TargetController)
	// 				{
	// 					// Create a dynamic instant Gameplay Effect to give the bounties
	// 					UGameplayEffect* GEBounty = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("Bounty")));
	// 					GEBounty->DurationPolicy = EGameplayEffectDurationType::Instant;
	//
	// 					int32 Idx = GEBounty->Modifiers.Num();
	// 					GEBounty->Modifiers.SetNum(Idx + 2);
	//
	// 					FGameplayModifierInfo& InfoXP = GEBounty->Modifiers[Idx];
	// 					InfoXP.ModifierMagnitude = FScalableFloat(GetXPBounty());
	// 					InfoXP.ModifierOp = EGameplayModOp::Additive;
	// 					InfoXP.Attribute = UVTAttributeSetBase::GetXPAttribute();
	//
	// 					FGameplayModifierInfo& InfoGold = GEBounty->Modifiers[Idx + 1];
	// 					InfoGold.ModifierMagnitude = FScalableFloat(GetGoldBounty());
	// 					InfoGold.ModifierOp = EGameplayModOp::Additive;
	// 					InfoGold.Attribute = UVTAttributeSetBase::GetGoldAttribute();
	//
	// 					Source->ApplyGameplayEffectToSelf(GEBounty, 1.0f, Source->MakeEffectContext());
	// 				}
	// 			}
	// 		}
	// 	}
	// } // Damage
	// else if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	// {
	// 	// Handle other health changes.
	// 	// Health loss should go through Damage.
	// 	SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	// } // Health
	// else if (Data.EvaluatedData.Attribute == GetManaAttribute())
	// {
	// 	// Handle mana changes.
	// 	SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
	// } // Mana
	// else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	// {
	// 	// Handle stamina changes.
	// 	SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	// }
	// else if (Data.EvaluatedData.Attribute == GetShieldAttribute())
	// {
	// 	// Handle shield changes.
	// 	SetShield(FMath::Clamp(GetShield(), 0.0f, GetMaxShield()));
	// }
}

void UVTWeaponAttributeSetBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void UVTWeaponAttributeSetBase::OnRep_CurrentAmmo(const FGameplayAttributeData& OldCurrentAmmo)
{
}

void UVTWeaponAttributeSetBase::OnRep_ReserveAmmo(const FGameplayAttributeData& OldReserveAmmo)
{
}
