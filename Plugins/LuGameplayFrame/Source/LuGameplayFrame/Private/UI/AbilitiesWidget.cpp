// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/AbilitiesWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Kismet/GameplayStatics.h"

void UAbilitiesWidget::ReInitWidget_Implementation()
{
	LoadAbilitiesStateTag();
	LoadListenAbilityTags();

	AbilitiesTagWithState = {
		{CooldownTag, EAbilityState::Cooldown},
		{SilenceTag, EAbilityState::Silence},
		{ClearTag, EAbilityState::Clear},
	};

	if (WeakAbilitySystemComponentPtr.IsValid() && ListenAbilityTags.Num() > 0)
	{
		for (const FGameplayTag Tag : ListenAbilityTags)
		{
			WeakAbilitySystemComponentPtr.Get()->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved)
			                             .AddUObject(this, &UAbilitiesWidget::TagChanged);
		}
	}
}

bool UAbilitiesWidget::UpdateAbilityState_Implementation(EAbilityState NewState)
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

UAbilitySystemComponent* UAbilitiesWidget::SetAbilityComponentFromPawn(APawn* TargetPawn)
{
	if (TargetPawn->GetClass()->ImplementsInterface(UAbilitySystemInterface::StaticClass()))
	{
		if (const IAbilitySystemInterface* SystemInterface = Cast<IAbilitySystemInterface>(TargetPawn); SystemInterface != nullptr)
		{
			WeakAbilitySystemComponentPtr = SystemInterface->GetAbilitySystemComponent();
		}
	}
	return WeakAbilitySystemComponentPtr.Get();
}

UAbilitySystemComponent* UAbilitiesWidget::SetAbilityComponent(UAbilitySystemComponent* Ptr)
{
	if (Ptr != nullptr)
	{
		WeakAbilitySystemComponentPtr = Ptr;
	}
	return WeakAbilitySystemComponentPtr.Get();
}

TArray<float> UAbilitiesWidget::GetActiveGameplayEffectDurationFromTag(APawn* TargetPawn, FGameplayTagContainer Tags)
{
	TArray<float> Result = {};
	if (const UAbilitySystemComponent* ASC = SetAbilityComponentFromPawn(TargetPawn); ASC != nullptr)
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

float UAbilitiesWidget::GetActiveGameplayEffectDurationFromClass(APawn* TargetPawn, TSubclassOf<UGameplayEffect> EffectClass)
{
	if (const UAbilitySystemComponent* Asc = SetAbilityComponentFromPawn(TargetPawn); Asc != nullptr)
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
	for (const FGameplayTag Tag : ListenAbilityTags)
	{
		WeakAbilitySystemComponentPtr->RegisterGameplayTagEvent(Tag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);
	}
	Destruct();
}

void UAbilitiesWidget::TagChanged(const FGameplayTag Tag, int32 NewCount)
{
	OnTranspondToStateSwitch_Implementation(Tag, NewCount);
	if (NewCount > 0)
	{
		OnGameplayTagAdded.Broadcast(Tag);
	}
	else
	{
		OnGameplayTagAddedRemoved.Broadcast(Tag);
	}
}

void UAbilitiesWidget::OnTranspondToStateSwitch_Implementation(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount > 0)
	{
		if (AbilitiesTagWithState.Contains(Tag))
		{
			UpdateAbilityState(AbilitiesTagWithState[Tag]);
		}
	}
	else
	{
		if (AbilitiesTagWithState.Contains(Tag) && Tag != ClearTag)
		{
			UpdateAbilityState(EAbilityState::CanUse);
		}
	}
}


void UAbilitiesWidget::LoadAbilitiesStateTag()
{
	for (auto Info : AbilityInfo.AbilityStateTags)
	{
		if (Info.Key == EAbilityState::Cooldown)
		{
			CooldownTag = Info.Value;
		}
		else if (Info.Key == EAbilityState::Silence)
		{
			SilenceTag = Info.Value;
		}
		else if (Info.Key == EAbilityState::Clear)
		{
			ClearTag = Info.Value;
		}
	}
}

void UAbilitiesWidget::LoadListenAbilityTags()
{
	const TArray<FGameplayTag> CurTags = {AbilityTag, CooldownTag, SilenceTag, ClearTag};
	for (FGameplayTag Tag : CurTags)
	{
		if (Tag.IsValid())
		{
			if (!ListenAbilityTags.HasTag(Tag))
			{
				ListenAbilityTags.AddTag(Tag);
			}
		}
	}
}

UAbilitySystemComponent* UAbilitiesWidget::GetWeakAbilitySystemComponent()
{
	return WeakAbilitySystemComponentPtr.Get();
}
