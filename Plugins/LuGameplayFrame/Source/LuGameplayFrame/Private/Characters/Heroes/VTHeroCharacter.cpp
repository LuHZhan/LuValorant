// Copyright 2024 Dan Kestranek.


#include "Characters/Heroes/VTHeroCharacter.h"
#include "Animation/AnimInstance.h"
#include "AI/VTHeroAIController.h"
#include "Camera/CameraComponent.h"
#include "Characters/Abilities/VTAbilitySystemComponent.h"
#include "Characters/Abilities/GSAbilitySystemGlobals.h"
#include "Characters/Abilities/AttributeSets/VTAmmoAttributeSet.h"
#include "..\..\..\Public\Characters\Abilities\AttributeSets\VTAttributeSetBase.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "../ValorantGameModeBase.h"

#include "VTBlueprintFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/VTPlayerController.h"
#include "Player/VTPlayerState.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"
#include "UI/VTFloatingStatusBarWidget.h"
#include "Weapons/VTWeapon.h"

AVTHeroCharacter::AVTHeroCharacter(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;
	bStartInFirstPersonPerspective = true;
	bIsFirstPersonPerspective = false;
	bWasInFirstPersonPerspectiveWhenKnockedDown = false;
	bASCInputBound = false;
	bChangedWeaponLocally = false;
	Default1PFOV = 90.0f;
	Default3PFOV = 80.0f;
	NoWeaponTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Equipped.None"));
	WeaponChangingDelayReplicationTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.IsChangingDelayReplication"));
	WeaponAmmoTypeNoneTag = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	WeaponAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Weapon"));
	CurrentWeaponTag = NoWeaponTag;
	Inventory = FVTHeroInventory();
	ReviveDuration = 4.0f;

	ThirdPersonCameraBoom = CreateDefaultSubobject<USpringArmComponent>(FName("CameraBoom"));
	ThirdPersonCameraBoom->SetupAttachment(RootComponent);
	ThirdPersonCameraBoom->bUsePawnControlRotation = true;
	ThirdPersonCameraBoom->SetRelativeLocation(FVector(0, 50, 68.492264));

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FollowCamera"));
	ThirdPersonCamera->SetupAttachment(ThirdPersonCameraBoom);
	ThirdPersonCamera->FieldOfView = Default3PFOV;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(FName("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(RootComponent);
	FirstPersonCamera->bUsePawnControlRotation = true;

	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(FName("FirstPersonMesh"));
	FirstPersonMesh->SetupAttachment(FirstPersonCamera);
	FirstPersonMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));
	FirstPersonMesh->bReceivesDecals = false;
	FirstPersonMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	FirstPersonMesh->CastShadow = false;
	FirstPersonMesh->SetVisibility(false, true);

	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionProfileName(FName("NoCollision"));
	GetMesh()->SetCollisionResponseToChannel(COLLISION_INTERACTABLE, ECollisionResponse::ECR_Overlap);
	GetMesh()->bCastHiddenShadow = true;
	GetMesh()->bReceivesDecals = false;

	UIFloatingStatusBarComponent = CreateDefaultSubobject<UWidgetComponent>(FName("UIFloatingStatusBarComponent"));
	UIFloatingStatusBarComponent->SetupAttachment(RootComponent);
	UIFloatingStatusBarComponent->SetRelativeLocation(FVector(0, 0, 120));
	UIFloatingStatusBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	UIFloatingStatusBarComponent->SetDrawSize(FVector2D(500, 500));

	UIFloatingStatusBarClass = StaticLoadClass(UObject::StaticClass(), nullptr, TEXT("/Game/GASShooter/UI/UI_FloatingStatusBar_Hero.UI_FloatingStatusBar_Hero_C"));
	if (!UIFloatingStatusBarClass)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() Failed to find UIFloatingStatusBarClass. If it was moved, please update the reference location in C++."), *FString(__FUNCTION__));
	}

	AutoPossessAI = EAutoPossessAI::PlacedInWorld;
	AIControllerClass = AVTHeroAIController::StaticClass();

	// Cache tags
	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
}

void AVTHeroCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AVTHeroCharacter, Inventory);
	// Only replicate CurrentWeapon to simulated clients and manually sync CurrentWeeapon with Owner when we're ready.
	// This allows us to predict weapon changing.
	DOREPLIFETIME_CONDITION(AVTHeroCharacter, CurrentWeapon, COND_SimulatedOnly);
}

// Called to bind functionality to input
void AVTHeroCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AVTHeroCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AVTHeroCharacter::MoveRight);

	PlayerInputComponent->BindAxis("LookUp", this, &AVTHeroCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AVTHeroCharacter::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &AVTHeroCharacter::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &AVTHeroCharacter::TurnRate);

	PlayerInputComponent->BindAction("TogglePerspective", IE_Pressed, this, &AVTHeroCharacter::TogglePerspective);

	// Bind player input to the AbilitySystemComponent. Also called in OnRep_PlayerState because of a potential race condition.
	BindASCInput();
}

// Server only
void AVTHeroCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	AVTPlayerState* PS = GetPlayerState<AVTPlayerState>();
	if (PS)
	{
		// Set the ASC on the Server. Clients do this in OnRep_PlayerState()
		AbilitySystemComponent = Cast<UVTAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// AI won't have PlayerControllers so we can init again here just to be sure. No harm in initing twice for heroes that have PlayerControllers.
		PS->GetAbilitySystemComponent()->InitAbilityActorInfo(PS, this);

		WeaponChangingDelayReplicationTagChangedDelegateHandle = AbilitySystemComponent->RegisterGameplayTagEvent(WeaponChangingDelayReplicationTag)
		                                                                               .AddUObject(this, &AVTHeroCharacter::WeaponChangingDelayReplicationTagChanged);

		// Set the AttributeSetBase for convenience attribute functions
		AttributeSetBase = PS->GetAttributeSetBase();

		AmmoAttributeSet = PS->GetAmmoAttributeSet();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that possession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		InitializeAttributes();

		AddStartupEffects();

		AddCharacterAbilities();

		AVTPlayerController* PC = Cast<AVTPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}

		if (AbilitySystemComponent->GetTagCount(DeadTag) > 0)
		{
			// Set Health/Mana/Stamina to their max. This is only necessary for *Respawn*.
			SetHealth(GetMaxHealth());
			SetMana(GetMaxMana());
			SetStamina(GetMaxStamina());
			SetShield(GetMaxShield());
		}

		// Remove Dead tag
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(DeadTag));

		InitializeFloatingStatusBar();

		// If player is host on listen server, the floating status bar would have been created for them from BeginPlay before player possession, hide it
		if (IsLocallyControlled() && IsPlayerControlled() && UIFloatingStatusBarComponent && UIFloatingStatusBar)
		{
			UIFloatingStatusBarComponent->SetVisibility(false, true);
		}
	}

	SetupStartupPerspective();
}

UVTFloatingStatusBarWidget* AVTHeroCharacter::GetFloatingStatusBar()
{
	return UIFloatingStatusBar;
}

void AVTHeroCharacter::KnockDown()
{
	if (!HasAuthority())
	{
		return;
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(KnockDownEffect->GetDefaultObject()), 1.0f, AbilitySystemComponent->MakeEffectContext());
	}

	SetHealth(GetMaxHealth());
	SetShield(0.0f);
}

void AVTHeroCharacter::PlayKnockDownEffects()
{
	// Store perspective to restore on Revive
	bWasInFirstPersonPerspectiveWhenKnockedDown = IsInFirstPersonPerspective();

	SetPerspective(false);

	// Play it here instead of in the ability to skip extra replication data
	if (DeathMontage)
	{
		PlayAnimMontage(DeathMontage);
	}

	if (AbilitySystemComponent)
	{
		FGameplayCueParameters GCParameters;
		GCParameters.Location = GetActorLocation();
		AbilitySystemComponent->ExecuteGameplayCueLocal(FGameplayTag::RequestGameplayTag("GameplayCue.Hero.KnockedDown"), GCParameters);
	}
}

void AVTHeroCharacter::PlayReviveEffects()
{
	// Restore perspective the player had when knocked down
	SetPerspective(bWasInFirstPersonPerspectiveWhenKnockedDown);

	// Play revive particles or sounds here (we don't have any)
	if (AbilitySystemComponent)
	{
		FGameplayCueParameters GCParameters;
		GCParameters.Location = GetActorLocation();
		AbilitySystemComponent->ExecuteGameplayCueLocal(FGameplayTag::RequestGameplayTag("GameplayCue.Hero.Revived"), GCParameters);
	}
}

void AVTHeroCharacter::FinishDying()
{
	// AGSHeroCharacter doesn't follow AGSCharacterBase's pattern of Die->Anim->FinishDying because AGSHeroCharacter can be knocked down
	// to either be revived, bleed out, or finished off by an enemy.

	if (!HasAuthority())
	{
		return;
	}

	RemoveAllWeaponsFromInventory();

	AbilitySystemComponent->RegisterGameplayTagEvent(WeaponChangingDelayReplicationTag).Remove(WeaponChangingDelayReplicationTagChangedDelegateHandle);

	AValorantGameModeBase* GM = Cast<AValorantGameModeBase>(GetWorld()->GetAuthGameMode());

	if (GM)
	{
		GM->HeroDied(GetController());
	}

	RemoveCharacterAbilities();

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAllAbilities();

		FGameplayTagContainer EffectTagsToRemove;
		EffectTagsToRemove.AddTag(EffectRemoveOnDeathTag);
		int32 NumEffectsRemoved = AbilitySystemComponent->RemoveActiveEffectsWithTags(EffectTagsToRemove);

		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(DeathEffect->GetDefaultObject()), 1.0f, AbilitySystemComponent->MakeEffectContext());
	}

	OnCharacterDied.Broadcast(this);

	Super::FinishDying();
}

bool AVTHeroCharacter::IsInFirstPersonPerspective() const
{
	return bIsFirstPersonPerspective;
}

USkeletalMeshComponent* AVTHeroCharacter::GetFirstPersonMesh() const
{
	return FirstPersonMesh;
}

USkeletalMeshComponent* AVTHeroCharacter::GetThirdPersonMesh() const
{
	return GetMesh();
}

AVTWeapon* AVTHeroCharacter::GetCurrentWeapon() const
{
	return CurrentWeapon;
}

bool AVTHeroCharacter::AddWeaponToInventory(AVTWeapon* NewWeapon, bool bEquipWeapon)
{
	if (DoesWeaponExistInInventory(NewWeapon))
	{
		USoundCue* PickupSound = NewWeapon->GetPickupSound();

		if (PickupSound && IsLocallyControlled())
		{
			UGameplayStatics::SpawnSoundAttached(PickupSound, GetRootComponent());
		}

		if (GetLocalRole() < ROLE_Authority)
		{
			return false;
		}

		// Create a dynamic instant Gameplay Effect to give the primary and secondary ammo
		UGameplayEffect* GEAmmo = NewObject<UGameplayEffect>(GetTransientPackage(), FName(TEXT("Ammo")));
		GEAmmo->DurationPolicy = EGameplayEffectDurationType::Instant;

		if (NewWeapon->PrimaryAmmoType != WeaponAmmoTypeNoneTag)
		{
			int32 Idx = GEAmmo->Modifiers.Num();
			GEAmmo->Modifiers.SetNum(Idx + 1);

			FGameplayModifierInfo& InfoPrimaryAmmo = GEAmmo->Modifiers[Idx];
			InfoPrimaryAmmo.ModifierMagnitude = FScalableFloat(NewWeapon->GetPrimaryClipAmmo());
			InfoPrimaryAmmo.ModifierOp = EGameplayModOp::Additive;
			InfoPrimaryAmmo.Attribute = UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(NewWeapon->PrimaryAmmoType);
		}

		if (NewWeapon->SecondaryAmmoType != WeaponAmmoTypeNoneTag)
		{
			int32 Idx = GEAmmo->Modifiers.Num();
			GEAmmo->Modifiers.SetNum(Idx + 1);

			FGameplayModifierInfo& InfoSecondaryAmmo = GEAmmo->Modifiers[Idx];
			InfoSecondaryAmmo.ModifierMagnitude = FScalableFloat(NewWeapon->GetSecondaryClipAmmo());
			InfoSecondaryAmmo.ModifierOp = EGameplayModOp::Additive;
			InfoSecondaryAmmo.Attribute = UGSAmmoAttributeSet::GetReserveAmmoAttributeFromTag(NewWeapon->SecondaryAmmoType);
		}

		if (GEAmmo->Modifiers.Num() > 0)
		{
			AbilitySystemComponent->ApplyGameplayEffectToSelf(GEAmmo, 1.0f, AbilitySystemComponent->MakeEffectContext());
		}

		NewWeapon->Destroy();

		return false;
	}

	if (GetLocalRole() < ROLE_Authority)
	{
		return false;
	}

	Inventory.Weapons.Add(NewWeapon);
	NewWeapon->SetOwningCharacter(this);
	NewWeapon->AddAbilities();

	if (bEquipWeapon)
	{
		EquipWeapon(NewWeapon);
		ClientSyncCurrentWeapon(CurrentWeapon);
	}

	return true;
}

bool AVTHeroCharacter::RemoveWeaponFromInventory(AVTWeapon* WeaponToRemove)
{
	if (DoesWeaponExistInInventory(WeaponToRemove))
	{
		if (WeaponToRemove == CurrentWeapon)
		{
			UnEquipCurrentWeapon();
		}

		Inventory.Weapons.Remove(WeaponToRemove);
		WeaponToRemove->RemoveAbilities();
		WeaponToRemove->SetOwningCharacter(nullptr);
		WeaponToRemove->ResetWeapon();

		// Add parameter to drop weapon?

		return true;
	}

	return false;
}

void AVTHeroCharacter::RemoveAllWeaponsFromInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	UnEquipCurrentWeapon();

	float radius = 50.0f;
	float NumWeapons = Inventory.Weapons.Num();

	for (int32 i = Inventory.Weapons.Num() - 1; i >= 0; i--)
	{
		AVTWeapon* Weapon = Inventory.Weapons[i];
		RemoveWeaponFromInventory(Weapon);

		// Set the weapon up as a pickup

		float OffsetX = radius * FMath::Cos((i / NumWeapons) * 2.0f * PI);
		float OffsetY = radius * FMath::Sin((i / NumWeapons) * 2.0f * PI);
		Weapon->OnDropped(GetActorLocation() + FVector(OffsetX, OffsetY, 0.0f));
	}
}

void AVTHeroCharacter::EquipWeapon(AVTWeapon* NewWeapon)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerEquipWeapon(NewWeapon);
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
		bChangedWeaponLocally = true;
	}
	else
	{
		SetCurrentWeapon(NewWeapon, CurrentWeapon);
	}
}

void AVTHeroCharacter::ServerEquipWeapon_Implementation(AVTWeapon* NewWeapon)
{
	EquipWeapon(NewWeapon);
}

bool AVTHeroCharacter::ServerEquipWeapon_Validate(AVTWeapon* NewWeapon)
{
	return true;
}

void AVTHeroCharacter::NextWeapon()
{
	if (Inventory.Weapons.Num() < 2)
	{
		return;
	}

	int32 CurrentWeaponIndex = Inventory.Weapons.Find(CurrentWeapon);
	UnEquipCurrentWeapon();

	if (CurrentWeaponIndex == INDEX_NONE)
	{
		EquipWeapon(Inventory.Weapons[0]);
	}
	else
	{
		EquipWeapon(Inventory.Weapons[(CurrentWeaponIndex + 1) % Inventory.Weapons.Num()]);
	}
}

void AVTHeroCharacter::PreviousWeapon()
{
	if (Inventory.Weapons.Num() < 2)
	{
		return;
	}

	int32 CurrentWeaponIndex = Inventory.Weapons.Find(CurrentWeapon);

	UnEquipCurrentWeapon();

	if (CurrentWeaponIndex == INDEX_NONE)
	{
		EquipWeapon(Inventory.Weapons[0]);
	}
	else
	{
		int32 IndexOfPrevWeapon = FMath::Abs(CurrentWeaponIndex - 1 + Inventory.Weapons.Num()) % Inventory.Weapons.Num();
		EquipWeapon(Inventory.Weapons[IndexOfPrevWeapon]);
	}
}

FName AVTHeroCharacter::GetWeaponAttachPoint()
{
	return WeaponAttachPoint;
}

int32 AVTHeroCharacter::GetPrimaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetPrimaryClipAmmo();
	}

	return 0;
}

int32 AVTHeroCharacter::GetMaxPrimaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetMaxPrimaryClipAmmo();
	}

	return 0;
}

int32 AVTHeroCharacter::GetPrimaryReserveAmmo() const
{
	if (CurrentWeapon && AmmoAttributeSet)
	{
		FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType);
		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

int32 AVTHeroCharacter::GetSecondaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetSecondaryClipAmmo();
	}

	return 0;
}

int32 AVTHeroCharacter::GetMaxSecondaryClipAmmo() const
{
	if (CurrentWeapon)
	{
		return CurrentWeapon->GetMaxSecondaryClipAmmo();
	}

	return 0;
}

int32 AVTHeroCharacter::GetSecondaryReserveAmmo() const
{
	if (CurrentWeapon)
	{
		FGameplayAttribute Attribute = AmmoAttributeSet->GetReserveAmmoAttributeFromTag(CurrentWeapon->SecondaryAmmoType);
		if (Attribute.IsValid())
		{
			return AbilitySystemComponent->GetNumericAttribute(Attribute);
		}
	}

	return 0;
}

int32 AVTHeroCharacter::GetNumWeapons() const
{
	return Inventory.Weapons.Num();
}

bool AVTHeroCharacter::IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	// Hero is available to be revived if knocked down and is not already being revived.
	// If you want multiple heroes reviving someone to speed it up, you would need to change GA_Interact
	// (outside the scope of this sample).
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag)
		&& !AbilitySystemComponent->HasMatchingGameplayTag(InteractingTag))
	{
		return true;
	}

	return IVTInteractable::IsAvailableForInteraction_Implementation(InteractionComponent);
}

float AVTHeroCharacter::GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
	{
		return ReviveDuration;
	}

	return IVTInteractable::GetInteractionDuration_Implementation(InteractionComponent);
}

void AVTHeroCharacter::PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) && HasAuthority())
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(FGameplayTagContainer(FGameplayTag::RequestGameplayTag("Ability.Revive")));
	}
}

void AVTHeroCharacter::PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) && HasAuthority())
	{
		AbilitySystemComponent->ApplyGameplayEffectToSelf(Cast<UGameplayEffect>(ReviveEffect->GetDefaultObject()), 1.0f, AbilitySystemComponent->MakeEffectContext());
	}
}

void AVTHeroCharacter::GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
	{
		bShouldSync = true;
		Type = EAbilityTaskNetSyncType::OnlyClientWait;
		return;
	}

	IVTInteractable::GetPreInteractSyncType_Implementation(bShouldSync, Type, InteractionComponent);
}

void AVTHeroCharacter::CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent)
{
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) && HasAuthority())
	{
		FGameplayTagContainer CancelTags(FGameplayTag::RequestGameplayTag("Ability.Revive"));
		AbilitySystemComponent->CancelAbilities(&CancelTags);
	}
}

FSimpleMulticastDelegate* AVTHeroCharacter::GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent)
{
	return &InteractionCanceledDelegate;
}

/**
* On the Server, Possession happens before BeginPlay.
* On the Client, BeginPlay happens before Possession.
* So we can't use BeginPlay to do anything with the AbilitySystemComponent because we don't have it until the PlayerState replicates from possession.
*/
void AVTHeroCharacter::BeginPlay()
{
	Super::BeginPlay();

	StartingFirstPersonMeshLocation = FirstPersonMesh->GetRelativeLocation();

	// Only needed for Heroes placed in world and when the player is the Server.
	// On respawn, they are set up in PossessedBy.
	// When the player a client, the floating status bars are all set up in OnRep_PlayerState.
	InitializeFloatingStatusBar();

	// CurrentWeapon is replicated only to Simulated clients so sync the current weapon manually
	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		ServerSyncCurrentWeapon();
	}
}

void AVTHeroCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Cancel being revived if killed
	//InteractionCanceledDelegate.Broadcast();
	Execute_InteractableCancelInteraction(this, GetThirdPersonMesh());

	// Clear CurrentWeaponTag on the ASC. This happens naturally in UnEquipCurrentWeapon() but
	// that is only called on the server from hero death (the OnRep_CurrentWeapon() would have
	// handled it on the client but that is never called due to the hero being marked pending
	// destroy). This makes sure the client has it cleared.
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}

	Super::EndPlay(EndPlayReason);
}

void AVTHeroCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	StartingThirdPersonCameraBoomArmLength = ThirdPersonCameraBoom->TargetArmLength;
	StartingThirdPersonCameraBoomLocation = ThirdPersonCameraBoom->GetRelativeLocation();
	StartingThirdPersonMeshLocation = GetMesh()->GetRelativeLocation();

	GetWorldTimerManager().SetTimerForNextTick(this, &AVTHeroCharacter::SpawnDefaultInventory);
}

void AVTHeroCharacter::LookUp(float Value)
{
	if (IsAlive())
	{
		AddControllerPitchInput(Value);
	}
}

void AVTHeroCharacter::LookUpRate(float Value)
{
	if (IsAlive())
	{
		AddControllerPitchInput(Value * BaseLookUpRate * GetWorld()->DeltaTimeSeconds);
	}
}

void AVTHeroCharacter::Turn(float Value)
{
	if (IsAlive())
	{
		AddControllerYawInput(Value);
	}
}

void AVTHeroCharacter::TurnRate(float Value)
{
	if (IsAlive())
	{
		AddControllerYawInput(Value * BaseTurnRate * GetWorld()->DeltaTimeSeconds);
	}
}

void AVTHeroCharacter::MoveForward(float Value)
{
	if (IsAlive())
	{
		AddMovementInput(UKismetMathLibrary::GetForwardVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
	}
}

void AVTHeroCharacter::MoveRight(float Value)
{
	if (IsAlive())
	{
		AddMovementInput(UKismetMathLibrary::GetRightVector(FRotator(0, GetControlRotation().Yaw, 0)), Value);
	}
}

void AVTHeroCharacter::TogglePerspective()
{
	// If knocked down, always be in 3rd person
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag))
	{
		return;
	}

	bIsFirstPersonPerspective = !bIsFirstPersonPerspective;
	SetPerspective(bIsFirstPersonPerspective);
}

void AVTHeroCharacter::SetPerspective(bool InIsFirstPersonPerspective)
{
	// If knocked down, always be in 3rd person
	if (IsValid(AbilitySystemComponent) && AbilitySystemComponent->HasMatchingGameplayTag(KnockedDownTag) && InIsFirstPersonPerspective)
	{
		return;
	}

	// Only change perspective for the locally controlled player. Simulated proxies should stay in third person.
	// To swap cameras, deactivate current camera (defaults to ThirdPersonCamera), activate desired camera, and call PlayerController->SetViewTarget() on self
	AVTPlayerController* PC = GetController<AVTPlayerController>();
	if (PC && PC->IsLocalPlayerController())
	{
		if (InIsFirstPersonPerspective)
		{
			ThirdPersonCamera->Deactivate();
			FirstPersonCamera->Activate();
			PC->SetViewTarget(this);

			GetMesh()->SetVisibility(false, true);
			FirstPersonMesh->SetVisibility(true, true);

			// Move third person mesh back so that the shadow doesn't look disconnected
			GetMesh()->SetRelativeLocation(StartingThirdPersonMeshLocation + FVector(-120.0f, 0.0f, 0.0f));
		}
		else
		{
			FirstPersonCamera->Deactivate();
			ThirdPersonCamera->Activate();
			PC->SetViewTarget(this);

			FirstPersonMesh->SetVisibility(false, true);
			GetMesh()->SetVisibility(true, true);

			// Reset the third person mesh
			GetMesh()->SetRelativeLocation(StartingThirdPersonMeshLocation);
		}
	}
}

void AVTHeroCharacter::InitializeFloatingStatusBar()
{
	// Only create once
	if (UIFloatingStatusBar || !IsValid(AbilitySystemComponent))
	{
		return;
	}

	// Don't create for locally controlled player. We could add a game setting to toggle this later.
	if (IsPlayerControlled() && IsLocallyControlled())
	{
		return;
	}

	// Need a valid PlayerState
	if (!GetPlayerState())
	{
		return;
	}

	// Setup UI for Locally Owned Players only, not AI or the server's copy of the PlayerControllers
	AVTPlayerController* PC = Cast<AVTPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
	if (PC && PC->IsLocalPlayerController())
	{
		if (UIFloatingStatusBarClass)
		{
			UIFloatingStatusBar = CreateWidget<UVTFloatingStatusBarWidget>(PC, UIFloatingStatusBarClass);
			if (UIFloatingStatusBar && UIFloatingStatusBarComponent)
			{
				UIFloatingStatusBarComponent->SetWidget(UIFloatingStatusBar);

				// Setup the floating status bar
				UIFloatingStatusBar->SetHealthPercentage(GetHealth() / GetMaxHealth());
				UIFloatingStatusBar->SetManaPercentage(GetMana() / GetMaxMana());
				UIFloatingStatusBar->SetShieldPercentage(GetShield() / GetMaxShield());
				UIFloatingStatusBar->OwningCharacter = this;
				UIFloatingStatusBar->SetCharacterName(CharacterName);
			}
		}
	}
}

// Client only
void AVTHeroCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AVTPlayerState* PS = GetPlayerState<AVTPlayerState>();
	if (PS)
	{
		// Set the ASC for clients. Server does this in PossessedBy.
		AbilitySystemComponent = Cast<UVTAbilitySystemComponent>(PS->GetAbilitySystemComponent());

		// Init ASC Actor Info for clients. Server will init its ASC when it possesses a new Actor.
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);

		// Bind player input to the AbilitySystemComponent. Also called in SetupPlayerInputComponent because of a potential race condition.
		BindASCInput();

		AbilitySystemComponent->AbilityFailedCallbacks.AddUObject(this, &AVTHeroCharacter::OnAbilityActivationFailed);

		// Set the AttributeSetBase for convenience attribute functions
		AttributeSetBase = PS->GetAttributeSetBase();

		AmmoAttributeSet = PS->GetAmmoAttributeSet();

		// If we handle players disconnecting and rejoining in the future, we'll have to change this so that posession from rejoining doesn't reset attributes.
		// For now assume possession = spawn/respawn.
		InitializeAttributes();

		AVTPlayerController* PC = Cast<AVTPlayerController>(GetController());
		if (PC)
		{
			PC->CreateHUD();
		}

		if (CurrentWeapon)
		{
			// If current weapon repped before PlayerState, set tag on ASC
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
			// Update owning character and ASC just in case it repped before PlayerState
			CurrentWeapon->SetOwningCharacter(this);

			if (!PrimaryReserveAmmoChangedDelegateHandle.IsValid())
			{
				PrimaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UVTAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType)).AddUObject(this, &AVTHeroCharacter::CurrentWeaponPrimaryReserveAmmoChanged);
			}
			if (!SecondaryReserveAmmoChangedDelegateHandle.IsValid())
			{
				SecondaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					UVTAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->SecondaryAmmoType)).AddUObject(this, &AVTHeroCharacter::CurrentWeaponSecondaryReserveAmmoChanged);
			}
		}

		if (AbilitySystemComponent->GetTagCount(DeadTag) > 0)
		{
			// Set Health/Mana/Stamina/Shield to their max. This is only for *Respawn*. It will be set (replicated) by the
			// Server, but we call it here just to be a little more responsive.
			SetHealth(GetMaxHealth());
			SetMana(GetMaxMana());
			SetStamina(GetMaxStamina());
			SetShield(GetMaxShield());
		}

		// Simulated on proxies don't have their PlayerStates yet when BeginPlay is called so we call it again here
		InitializeFloatingStatusBar();
	}
}

void AVTHeroCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();

	SetupStartupPerspective();
}

void AVTHeroCharacter::BindASCInput()
{
	if (!bASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		FTopLevelAssetPath AbilityEnumAssetPath = FTopLevelAssetPath(FName("/Script/GASShooter"), FName("EGSAbilityInputID"));
		AbilitySystemComponent->BindAbilityActivationToInputComponent(
			InputComponent,
			FGameplayAbilityInputBinds(FString("ConfirmTarget"),
			                           FString("CancelTarget"),
			                           AbilityEnumAssetPath,
			                           static_cast<int32>(EVTAbilityInputID::Confirm),
			                           static_cast<int32>(EVTAbilityInputID::Cancel)));

		bASCInputBound = true;
	}
}

void AVTHeroCharacter::SpawnDefaultInventory()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		return;
	}

	int32 NumWeaponClasses = DefaultInventoryWeaponClasses.Num();
	for (int32 i = 0; i < NumWeaponClasses; i++)
	{
		if (!DefaultInventoryWeaponClasses[i])
		{
			// An empty item was added to the Array in blueprint
			continue;
		}

		AVTWeapon* NewWeapon = GetWorld()->SpawnActorDeferred<AVTWeapon>(DefaultInventoryWeaponClasses[i],
		                                                                 FTransform::Identity, this, this, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		NewWeapon->bSpawnWithCollision = false;
		NewWeapon->FinishSpawning(FTransform::Identity);

		bool bEquipFirstWeapon = i == 0;
		AddWeaponToInventory(NewWeapon, bEquipFirstWeapon);
	}
}

void AVTHeroCharacter::SetupStartupPerspective()
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC && PC->IsLocalController())
	{
		bIsFirstPersonPerspective = bStartInFirstPersonPerspective;
		SetPerspective(bIsFirstPersonPerspective);
	}
}

bool AVTHeroCharacter::DoesWeaponExistInInventory(AVTWeapon* InWeapon)
{
	//UE_LOG(LogTemp, Log, TEXT("%s InWeapon class %s"), *FString(__FUNCTION__), *InWeapon->GetClass()->GetName());

	for (AVTWeapon* Weapon : Inventory.Weapons)
	{
		if (Weapon && InWeapon && Weapon->GetClass() == InWeapon->GetClass())
		{
			return true;
		}
	}

	return false;
}

void AVTHeroCharacter::SetCurrentWeapon(AVTWeapon* NewWeapon, AVTWeapon* LastWeapon)
{
	if (NewWeapon == LastWeapon)
	{
		return;
	}

	// Cancel active weapon abilities
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer AbilityTagsToCancel = FGameplayTagContainer(WeaponAbilityTag);
		AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);
	}

	UnEquipWeapon(LastWeapon);

	if (NewWeapon)
	{
		if (AbilitySystemComponent)
		{
			// Clear out potential NoWeaponTag
			AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		}

		// Weapons coming from OnRep_CurrentWeapon won't have the owner set
		CurrentWeapon = NewWeapon;
		CurrentWeapon->SetOwningCharacter(this);
		CurrentWeapon->Equip();
		CurrentWeaponTag = CurrentWeapon->WeaponTag;

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
		}

		AVTPlayerController* PC = GetController<AVTPlayerController>();
		if (PC && PC->IsLocalController())
		{
			PC->SetEquippedWeaponPrimaryIconFromSprite(CurrentWeapon->PrimaryIcon);
			PC->SetEquippedWeaponStatusText(CurrentWeapon->StatusText);
			PC->SetPrimaryClipAmmo(CurrentWeapon->GetPrimaryClipAmmo());
			PC->SetPrimaryReserveAmmo(GetPrimaryReserveAmmo());
			PC->SetHUDReticle(CurrentWeapon->GetPrimaryHUDReticleClass());
		}

		NewWeapon->OnPrimaryClipAmmoChanged.AddDynamic(this, &AVTHeroCharacter::CurrentWeaponPrimaryClipAmmoChanged);
		NewWeapon->OnSecondaryClipAmmoChanged.AddDynamic(this, &AVTHeroCharacter::CurrentWeaponSecondaryClipAmmoChanged);

		if (AbilitySystemComponent)
		{
			PrimaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UVTAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->PrimaryAmmoType)).AddUObject(this, &AVTHeroCharacter::CurrentWeaponPrimaryReserveAmmoChanged);
			SecondaryReserveAmmoChangedDelegateHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
				UVTAmmoAttributeSet::GetReserveAmmoAttributeFromTag(CurrentWeapon->SecondaryAmmoType)).AddUObject(this, &AVTHeroCharacter::CurrentWeaponSecondaryReserveAmmoChanged);
		}

		UAnimMontage* Equip1PMontage = CurrentWeapon->GetEquip1PMontage();
		if (Equip1PMontage && GetFirstPersonMesh())
		{
			GetFirstPersonMesh()->GetAnimInstance()->Montage_Play(Equip1PMontage);
		}

		UAnimMontage* Equip3PMontage = CurrentWeapon->GetEquip3PMontage();
		if (Equip3PMontage && GetThirdPersonMesh())
		{
			GetThirdPersonMesh()->GetAnimInstance()->Montage_Play(Equip3PMontage);
		}
	}
	else
	{
		// This will clear HUD, tags etc
		UnEquipCurrentWeapon();
	}
}

void AVTHeroCharacter::UnEquipWeapon(AVTWeapon* WeaponToUnEquip)
{
	//TODO this will run into issues when calling UnEquipWeapon explicitly and the WeaponToUnEquip == CurrentWeapon

	if (WeaponToUnEquip)
	{
		WeaponToUnEquip->OnPrimaryClipAmmoChanged.RemoveDynamic(this, &AVTHeroCharacter::CurrentWeaponPrimaryClipAmmoChanged);
		WeaponToUnEquip->OnSecondaryClipAmmoChanged.RemoveDynamic(this, &AVTHeroCharacter::CurrentWeaponSecondaryClipAmmoChanged);

		if (AbilitySystemComponent)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UVTAmmoAttributeSet::GetReserveAmmoAttributeFromTag(WeaponToUnEquip->PrimaryAmmoType)).Remove(
				PrimaryReserveAmmoChangedDelegateHandle);
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UVTAmmoAttributeSet::GetReserveAmmoAttributeFromTag(WeaponToUnEquip->SecondaryAmmoType)).Remove(
				SecondaryReserveAmmoChangedDelegateHandle);
		}

		WeaponToUnEquip->UnEquip();
	}
}

void AVTHeroCharacter::UnEquipCurrentWeapon()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(CurrentWeaponTag);
		CurrentWeaponTag = NoWeaponTag;
		AbilitySystemComponent->AddLooseGameplayTag(CurrentWeaponTag);
	}

	UnEquipWeapon(CurrentWeapon);
	CurrentWeapon = nullptr;

	AVTPlayerController* PC = GetController<AVTPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetEquippedWeaponPrimaryIconFromSprite(nullptr);
		PC->SetEquippedWeaponStatusText(FText());
		PC->SetPrimaryClipAmmo(0);
		PC->SetPrimaryReserveAmmo(0);
		PC->SetHUDReticle(nullptr);
	}
}

void AVTHeroCharacter::CurrentWeaponPrimaryClipAmmoChanged(int32 OldPrimaryClipAmmo, int32 NewPrimaryClipAmmo)
{
	AVTPlayerController* PC = GetController<AVTPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetPrimaryClipAmmo(NewPrimaryClipAmmo);
	}
}

void AVTHeroCharacter::CurrentWeaponSecondaryClipAmmoChanged(int32 OldSecondaryClipAmmo, int32 NewSecondaryClipAmmo)
{
	AVTPlayerController* PC = GetController<AVTPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetSecondaryClipAmmo(NewSecondaryClipAmmo);
	}
}

void AVTHeroCharacter::CurrentWeaponPrimaryReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	AVTPlayerController* PC = GetController<AVTPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetPrimaryReserveAmmo(Data.NewValue);
	}
}

void AVTHeroCharacter::CurrentWeaponSecondaryReserveAmmoChanged(const FOnAttributeChangeData& Data)
{
	AVTPlayerController* PC = GetController<AVTPlayerController>();
	if (PC && PC->IsLocalController())
	{
		PC->SetSecondaryReserveAmmo(Data.NewValue);
	}
}

void AVTHeroCharacter::WeaponChangingDelayReplicationTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (CallbackTag == WeaponChangingDelayReplicationTag)
	{
		if (NewCount < 1)
		{
			// We only replicate the current weapon to simulated proxies so manually sync it when the weapon changing delay replication
			// tag is removed. We keep the weapon changing tag on for ~1s after the equip montage to allow for activating changing weapon
			// again without the server trying to clobber the next locally predicted weapon.
			ClientSyncCurrentWeapon(CurrentWeapon);
		}
	}
}

void AVTHeroCharacter::OnRep_CurrentWeapon(AVTWeapon* LastWeapon)
{
	bChangedWeaponLocally = false;
	SetCurrentWeapon(CurrentWeapon, LastWeapon);
}

void AVTHeroCharacter::OnRep_Inventory()
{
	if (GetLocalRole() == ROLE_AutonomousProxy && Inventory.Weapons.Num() > 0 && !CurrentWeapon)
	{
		// Since we don't replicate the CurrentWeapon to the owning client, this is a way to ask the Server to sync
		// the CurrentWeapon after it's been spawned via replication from the Server.
		// The weapon spawning is replicated but the variable CurrentWeapon is not on the owning client.
		ServerSyncCurrentWeapon();
	}
}

void AVTHeroCharacter::OnAbilityActivationFailed(const UGameplayAbility* FailedAbility, const FGameplayTagContainer& FailTags)
{
	if (FailedAbility && FailedAbility->AbilityTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Ability.Weapon.IsChanging"))))
	{
		if (bChangedWeaponLocally)
		{
			// Ask the Server to resync the CurrentWeapon that we predictively changed
			UE_LOG(LogTemp, Warning, TEXT("%s Weapon Changing ability activation failed. Syncing CurrentWeapon. %s. %s"), *FString(__FUNCTION__),
			       *UVTBlueprintFunctionLibrary::GetPlayerEditorWindowRole(GetWorld()), *FailTags.ToString());

			ServerSyncCurrentWeapon();
		}
	}
}

void AVTHeroCharacter::ServerSyncCurrentWeapon_Implementation()
{
	ClientSyncCurrentWeapon(CurrentWeapon);
}

bool AVTHeroCharacter::ServerSyncCurrentWeapon_Validate()
{
	return true;
}

void AVTHeroCharacter::ClientSyncCurrentWeapon_Implementation(AVTWeapon* InWeapon)
{
	AVTWeapon* LastWeapon = CurrentWeapon;
	CurrentWeapon = InWeapon;
	OnRep_CurrentWeapon(LastWeapon);
}

bool AVTHeroCharacter::ClientSyncCurrentWeapon_Validate(AVTWeapon* InWeapon)
{
	return true;
}
