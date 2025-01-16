// Copyright 2024 Dan Kestranek.


#include "Player/VTPlayerController.h"
#include "Characters/Abilities/AttributeSets/VTAmmoAttributeSet.h"
#include "..\..\Public\Characters\Abilities\AttributeSets\VTAttributeSetBase.h"
#include "Characters/Abilities/VTAbilitySystemComponent.h"
#include "Characters/Heroes/VTHeroCharacter.h"
#include "Player/VTPlayerState.h"
#include "UI/VTHUDWidget.h"
#include "Weapons/VTWeapon.h"

void AVTPlayerController::CreateHUD()
{
	// Only create once
	if (UIHUDWidget)
	{
		return;
	}

	if (!UIHUDWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Missing UIHUDWidgetClass. Please fill in on the Blueprint of the PlayerController."), *FString(__FUNCTION__));
		return;
	}

	// Only create a HUD for local player
	if (!IsLocalPlayerController())
	{
		return;
	}

	// Need a valid PlayerState to get attributes from
	AVTPlayerState* PS = GetPlayerState<AVTPlayerState>();
	if (!PS)
	{
		return;
	}

	UIHUDWidget = CreateWidget<UVTHUDWidget>(this, UIHUDWidgetClass);
	UIHUDWidget->AddToViewport();

	// Set attributes
	UIHUDWidget->SetCurrentHealth(PS->GetHealth());
	UIHUDWidget->SetMaxHealth(PS->GetMaxHealth());
	UIHUDWidget->SetHealthPercentage(PS->GetHealth() / PS->GetMaxHealth());
	UIHUDWidget->SetCurrentMana(PS->GetMana());
	UIHUDWidget->SetMaxMana(PS->GetMaxMana());
	UIHUDWidget->SetManaPercentage(PS->GetMana() / PS->GetMaxMana());
	UIHUDWidget->SetHealthRegenRate(PS->GetHealthRegenRate());
	UIHUDWidget->SetManaRegenRate(PS->GetManaRegenRate());
	UIHUDWidget->SetCurrentStamina(PS->GetStamina());
	UIHUDWidget->SetMaxStamina(PS->GetMaxStamina());
	UIHUDWidget->SetStaminaPercentage(PS->GetStamina() / PS->GetMaxStamina());
	UIHUDWidget->SetStaminaRegenRate(PS->GetStaminaRegenRate());
	UIHUDWidget->SetCurrentShield(PS->GetShield());
	UIHUDWidget->SetMaxShield(PS->GetMaxShield());
	UIHUDWidget->SetShieldRegenRate(PS->GetShieldRegenRate());
	UIHUDWidget->SetShieldPercentage(PS->GetShield() / PS->GetMaxShield());
	UIHUDWidget->SetExperience(PS->GetXP());
	UIHUDWidget->SetGold(PS->GetGold());
	UIHUDWidget->SetHeroLevel(PS->GetCharacterLevel());

	AVTHeroCharacter* Hero = GetPawn<AVTHeroCharacter>();
	if (Hero)
	{
		AVTWeapon* CurrentWeapon = Hero->GetCurrentWeapon();
		if (CurrentWeapon)
		{
			UIHUDWidget->SetEquippedWeaponSprite(CurrentWeapon->PrimaryIcon);
			UIHUDWidget->SetEquippedWeaponStatusText(CurrentWeapon->GetDefaultStatusText());
			UIHUDWidget->SetPrimaryClipAmmo(Hero->GetPrimaryClipAmmo());
			UIHUDWidget->SetReticle(CurrentWeapon->GetPrimaryHUDReticleClass());

			// PlayerState's Pawn isn't set up yet so we can't just call PS->GetPrimaryReserveAmmo()
			if (PS->GetAmmoAttributeSet())
			{
				FGameplayAttribute Attribute = PS->GetAmmoAttributeSet()->GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType);
				if (Attribute.IsValid())
				{
					UIHUDWidget->SetPrimaryReserveAmmo(PS->GetAbilitySystemComponent()->GetNumericAttribute(Attribute));
				}
			}
		}
	}
}

UVTHUDWidget* AVTPlayerController::GetGSHUD()
{
	return UIHUDWidget;
}

void AVTPlayerController::SetEquippedWeaponPrimaryIconFromSprite(UPaperSprite* InSprite)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetEquippedWeaponSprite(InSprite);
	}
}

void AVTPlayerController::SetEquippedWeaponStatusText(const FText& StatusText)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetEquippedWeaponStatusText(StatusText);
	}
}

void AVTPlayerController::SetPrimaryClipAmmo(int32 ClipAmmo)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetPrimaryClipAmmo(ClipAmmo);
	}
}

void AVTPlayerController::SetPrimaryReserveAmmo(int32 ReserveAmmo)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetPrimaryReserveAmmo(ReserveAmmo);
	}
}

void AVTPlayerController::SetSecondaryClipAmmo(int32 SecondaryClipAmmo)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetSecondaryClipAmmo(SecondaryClipAmmo);
	}
}

void AVTPlayerController::SetSecondaryReserveAmmo(int32 SecondaryReserveAmmo)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetSecondaryReserveAmmo(SecondaryReserveAmmo);
	}
}

void AVTPlayerController::SetHUDReticle(TSubclassOf<UVTHUDReticle> ReticleClass)
{
	// !GetWorld()->bIsTearingDown Stops an error when quitting PIE while targeting when the EndAbility resets the HUD reticle
	if (UIHUDWidget && GetWorld() && !GetWorld()->bIsTearingDown)
	{
		UIHUDWidget->SetReticle(ReticleClass);
	}
}

void AVTPlayerController::ShowDamageNumber_Implementation(float DamageAmount, AVTCharacterBase* TargetCharacter, FGameplayTagContainer DamageNumberTags)
{
	if (IsValid(TargetCharacter))
	{
		TargetCharacter->AddDamageNumber(DamageAmount, DamageNumberTags);
	}
}

bool AVTPlayerController::ShowDamageNumber_Validate(float DamageAmount, AVTCharacterBase* TargetCharacter, FGameplayTagContainer DamageNumberTags)
{
	return true;
}

void AVTPlayerController::SetRespawnCountdown_Implementation(float RespawnTimeRemaining)
{
	if (UIHUDWidget)
	{
		UIHUDWidget->SetRespawnCountdown(RespawnTimeRemaining);
	}
}

bool AVTPlayerController::SetRespawnCountdown_Validate(float RespawnTimeRemaining)
{
	return true;
}

void AVTPlayerController::ClientSetControlRotation_Implementation(FRotator NewRotation)
{
	SetControlRotation(NewRotation);
}

bool AVTPlayerController::ClientSetControlRotation_Validate(FRotator NewRotation)
{
	return true;
}

void AVTPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AVTPlayerState* PS = GetPlayerState<AVTPlayerState>();
	if (PS)
	{
		// Init ASC with PS (Owner) and our new Pawn (AvatarActor)
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, InPawn);
	}
}

void AVTPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// For edge cases where the PlayerState is repped before the Hero is possessed.
	CreateHUD();
}

void AVTPlayerController::Kill()
{
	ServerKill();
}

void AVTPlayerController::ServerKill_Implementation()
{
	AVTPlayerState* PS = GetPlayerState<AVTPlayerState>();
	if (PS)
	{
		PS->GetAttributeSetBase()->SetHealth(0.0f);
	}
}

bool AVTPlayerController::ServerKill_Validate()
{
	return true;
}
