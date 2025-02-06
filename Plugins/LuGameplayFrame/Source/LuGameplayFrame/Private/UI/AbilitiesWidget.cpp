// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitiesWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"


bool UAbilitiesWidget::UpdateAbilityCount_Implementation(EAbilityState NewState)
{
	if (NewState == EAbilityState::CanUse)
	{
		AbilityStateSwitchToCanUse(GetCurAbilityState());
		return true;
	}
	else if (NewState == EAbilityState::Clear)
	{
		AbilityStateSwitchToClear(GetCurAbilityState());
		return true;
	}
	else if (NewState == EAbilityState::Cooldown)
	{
		AbilityStateSwitchToCooldown(GetCurAbilityState());
		return true;
	}
	else if (NewState == EAbilityState::Silence)
	{
		AbilityStateSwitchToSilence(GetCurAbilityState());
		return true;
	}
	else if (NewState == EAbilityState::Start)
	{
		AbilityStateSwitchToStart(GetCurAbilityState());
		return true;
	}
	return false;
}

UAbilitySystemComponent* UAbilitiesWidget::GetAbilityComponent_Implementation(APawn* TargetPawn)
{
	if (TargetPawn->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		if (const IAbilitySystemInterface* SystemInterface = Cast<IAbilitySystemInterface>(TargetPawn); SystemInterface != nullptr)
		{
			return SystemInterface->GetAbilitySystemComponent();
		}
	}
	return nullptr;
}

TArray<float> UAbilitiesWidget::GetActiveGameplayEffectDurationFromTag_Implementation(APawn* TargetPawn, FGameplayTagContainer Tags)
{
	TArray<float> Result = {};
	if (const UAbilitySystemComponent* ASC = GetAbilityComponent(TargetPawn); ASC != nullptr)
	{
		TArray<FActiveGameplayEffectHandle> Effects = ASC->GetActiveEffects(FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(Tags));
		if (Effects.Num() > 0)
		{
			for (int i = 0; i < Effects.Num(); i++)
			{
				const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(Effects[i]);
				if (ActiveEffect && ActiveEffect->GetDuration() > 0)
				{
					Result.Add(ActiveEffect->GetTimeRemaining(ASC->GetWorld()->GetTimeSeconds()));
				}
			}
		}
	}
	return Result;
}

float UAbilitiesWidget::GetActiveGameplayEffectDurationFromClass_Implementation(APawn* TargetPawn, TSubclassOf<UGameplayEffect> EffectClass)
{
	if (const UAbilitySystemComponent* Asc = GetAbilityComponent(TargetPawn); Asc != nullptr)
	{
		for (FActiveGameplayEffectHandle ActiveHandle : Asc->GetActiveGameplayEffects().GetAllActiveEffectHandles())
		{
			const FActiveGameplayEffect* Effect = Asc->GetActiveGameplayEffect(ActiveHandle);
			if (Effect->Spec.Def.GetClass() == EffectClass->GetClass())
			{
				if (Effect->GetDuration() > 0)
				{
					return Effect->GetTimeRemaining(Asc->GetWorld()->GetTimeSeconds());
				}
			}
		}
	}
	return 0.0f;
}

void UAbilitiesWidget::Reset_Implementation()
{
	Destruct();
}


void UAbilitiesWidget::LoadRelatedAbilityTags()
{
	const TArray<FGameplayTag> CurTags = {AbilityTag, CooldownTag, SilenceTag, ClearTag};
	for (FGameplayTag Tag : CurTags)
	{
		if (Tag.IsValid())
		{
			if (!RelatedAbilityTags.HasTag(Tag))
			{
				RelatedAbilityTags.AddTag(Tag);
			}
		}
	}
}
