// Copyright 2024 Dan Kestranek.


#include "Player/VTPlayerState.h"
#include "Characters/Abilities/AttributeSets/VTAmmoAttributeSet.h"
#include "Characters/Abilities/AttributeSets/VTAttributeSetBase.h"
#include "Characters/Abilities/VTAbilitySystemComponent.h"
#include "Characters/Heroes/VTHeroCharacter.h"
#include "Characters/Heroes/Abilities/VTAbilityAttributeSet.h"
#include "Player/VTPlayerController.h"
#include "UI/VTHUDWidget.h"
#include "Weapons/VTWeapon.h"

AVTPlayerState::AVTPlayerState()
{
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<UVTAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// Create the attribute set, this replicates by default
	// Adding it as a subobject of the owning actor of an AbilitySystemComponent
	// automatically registers the AttributeSet with the AbilitySystemComponent
	AttributeSetBase = CreateDefaultSubobject<UVTAttributeSetBase>(TEXT("AttributeSetBase"));
	AmmoAttributeSet = CreateDefaultSubobject<UVTAmmoAttributeSet>(TEXT("AmmoAttributeSet"));
	
	LoadHeroAbilityAttributeKeyMap();
	TArray<FGameplayTag> ASTags;
	AbilityAttributeMap.GetKeys(ASTags);
	for (FGameplayTag Tag : ASTags)
	{
		FString Suffix{Tag.ToString()};
		Suffix.Append(TEXT("_AbilityAttributeSet"));
		AbilityAttributeMap[Tag] = CreateDefaultSubobject<UVTAbilityAttributeSet>(*Tag.ToString());
	}

	// Set PlayerState's NetUpdateFrequency to the same as the Character.
	// Default is very low for PlayerStates and introduces perceived lag in the ability system.
	// 100 is probably way too high for a shipping game, you can adjust to fit your needs.
	NetUpdateFrequency = 100.0f;

	DeadTag = FGameplayTag::RequestGameplayTag("State.Dead");
	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");

	HeroTag = FGameplayTag::RequestGameplayTag("Hero.Jett");
}

UAbilitySystemComponent* AVTPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UVTAttributeSetBase* AVTPlayerState::GetAttributeSetBase() const
{
	return AttributeSetBase;
}

UVTAmmoAttributeSet* AVTPlayerState::GetAmmoAttributeSet() const
{
	return AmmoAttributeSet;
}

TMap<FGameplayTag, UVTAbilityAttributeSet*> AVTPlayerState::GetAbilityAttributeSet() const
{
	return AbilityAttributeMap;
}

FHeroAbilityData AVTPlayerState::GetHeroAbilityData() const
{
	UVTHeroDataAsset* Asset = UBFLCommon::GetDefaultHeroDataAssetSync();
	if (Asset != nullptr)
	{
		for (FHeroAbilityData Data : Asset->HeroAbilityData)
		{
			if (Data.HeroTag == HeroTag)
			{
				return Data;
			}
		}
	}
	return FHeroAbilityData{};
}

bool AVTPlayerState::LoadHeroAbilityAttributeKeyMap()
{
	for (FHeroAbilityData HeroAbilityData = GetHeroAbilityData(); FGameplayTag AbilityTag : HeroAbilityData.HeroAbilities)
	{
		if (HeroAbilityData.AbilityInfoMap.Contains(AbilityTag))
		{
			AbilityAttributeMap.Add(AbilityTag, nullptr);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("LoadHeroAbilityAttributeMap: AbilityInfoMap does not contain AbilityTag: %s"), *AbilityTag.ToString());
			return false;
		}
	}
	return true;
}

bool AVTPlayerState::IsAlive() const
{
	return GetHealth() > 0.0f;
}

void AVTPlayerState::ShowAbilityConfirmPrompt(bool bShowPrompt)
{
	AVTPlayerController* PC = Cast<AVTPlayerController>(GetOwner());
	if (PC)
	{
		UVTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowAbilityConfirmPrompt(bShowPrompt);
		}
	}
}

void AVTPlayerState::ShowInteractionPrompt(float InteractionDuration)
{
	AVTPlayerController* PC = Cast<AVTPlayerController>(GetOwner());
	if (PC)
	{
		UVTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->ShowInteractionPrompt(InteractionDuration);
		}
	}
}

void AVTPlayerState::HideInteractionPrompt()
{
	AVTPlayerController* PC = Cast<AVTPlayerController>(GetOwner());
	if (PC)
	{
		UVTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->HideInteractionPrompt();
		}
	}
}

void AVTPlayerState::StartInteractionTimer(float InteractionDuration)
{
	AVTPlayerController* PC = Cast<AVTPlayerController>(GetOwner());
	if (PC)
	{
		UVTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->StartInteractionTimer(InteractionDuration);
		}
	}
}

void AVTPlayerState::StopInteractionTimer()
{
	AVTPlayerController* PC = Cast<AVTPlayerController>(GetOwner());
	if (PC)
	{
		UVTHUDWidget* HUD = PC->GetGSHUD();
		if (HUD)
		{
			HUD->StopInteractionTimer();
		}
	}
}

TMap<FGameplayAttributeData, float> AVTPlayerState::GetAttribute(const TArray<FGameplayAttributeData>& TargetAttributeData) const
{
	return {};
}

float AVTPlayerState::GetHealth() const
{
	return AttributeSetBase->GetHealth();
}

float AVTPlayerState::GetMaxHealth() const
{
	return AttributeSetBase->GetMaxHealth();
}

float AVTPlayerState::GetMana() const
{
	return AttributeSetBase->GetMana();
}

float AVTPlayerState::GetMaxMana() const
{
	return AttributeSetBase->GetMaxMana();
}

float AVTPlayerState::GetStamina() const
{
	return AttributeSetBase->GetStamina();
}

float AVTPlayerState::GetMaxStamina() const
{
	return AttributeSetBase->GetMaxStamina();
}

float AVTPlayerState::GetShield() const
{
	return AttributeSetBase->GetShield();
}

float AVTPlayerState::GetMaxShield() const
{
	return AttributeSetBase->GetMaxShield();
}

float AVTPlayerState::GetArmor() const
{
	return AttributeSetBase->GetArmor();
}

float AVTPlayerState::GetMoveSpeed() const
{
	return AttributeSetBase->GetMoveSpeed();
}

int32 AVTPlayerState::GetCharacterLevel() const
{
	return AttributeSetBase->GetCharacterLevel();
}

int32 AVTPlayerState::GetXP() const
{
	return AttributeSetBase->GetXP();
}

int32 AVTPlayerState::GetXPBounty() const
{
	return AttributeSetBase->GetXPBounty();
}

int32 AVTPlayerState::GetGold() const
{
	return AttributeSetBase->GetGold();
}

int32 AVTPlayerState::GetGoldBounty() const
{
	return AttributeSetBase->GetGoldBounty();
}

void AVTPlayerState::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent)
	{
		// Attribute change callbacks
		HealthChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSetBase->GetHealthAttribute()).AddUObject(this, &AVTPlayerState::HealthChanged);
		// Tag change callbacks
		AbilitySystemComponent->RegisterGameplayTagEvent(KnockedDownTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AVTPlayerState::KnockDownTagChanged);
	}
}

void AVTPlayerState::HealthChanged(const FOnAttributeChangeData& Data)
{
	// Check for and handle knockdown and death
	AVTHeroCharacter* Hero = Cast<AVTHeroCharacter>(GetPawn());
	if (IsValid(Hero) && !IsAlive() && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		if (Hero)
		{
			if (!AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
			{
				Hero->KnockDown();
			}
			else
			{
				Hero->FinishDying();
			}
		}
	}
}

void AVTPlayerState::KnockDownTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	AVTHeroCharacter* Hero = Cast<AVTHeroCharacter>(GetPawn());
	if (!IsValid(Hero))
	{
		return;
	}

	if (NewCount > 0)
	{
		Hero->PlayKnockDownEffects();
	}
	else if (NewCount < 1 && !AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		Hero->PlayReviveEffects();
	}
}
