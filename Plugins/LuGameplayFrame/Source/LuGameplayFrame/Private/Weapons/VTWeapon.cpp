// Copyright 2024 Dan Kestranek.


#include "Weapons/VTWeapon.h"
#include "Characters/Abilities/VTAbilitySystemComponent.h"
#include "Characters/Abilities/VTAbilitySystemGlobals.h"
#include "Characters/Abilities/VTGameplayAbility.h"
#include "Characters/Abilities/VTGATA_LineTrace.h"
#include "Characters/Abilities/VTGATA_SphereTrace.h"
#include "Characters/Heroes/VTHeroCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "VTBlueprintFunctionLibrary.h"
#include "Camera/CameraShakeSourceComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/VTPlayerController.h"
#include "Shakes/PerlinNoiseCameraShakePattern.h"
#include "Weapons/VTPerlinNoiseCameraShakePattern.h"

// Sets default values
AVTWeapon::AVTWeapon()
{
	// Set this actor to never tick
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	NetUpdateFrequency = 100.0f; // Set this to a value that's appropriate for your game
	bSpawnWithCollision = true;
	PrimaryClipAmmo = 0;
	MaxPrimaryClipAmmo = 0;
	SecondaryClipAmmo = 0;
	MaxSecondaryClipAmmo = 0;
	bInfiniteAmmo = false;
	PrimaryAmmoType = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));
	SecondaryAmmoType = FGameplayTag::RequestGameplayTag(FName("Weapon.Ammo.None"));

	CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionComponent"));
	CollisionComp->InitCapsuleSize(40.0f, 50.0f);
	CollisionComp->SetCollisionObjectType(COLLISION_PICKUP);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Manually enable when in pickup mode
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RootComponent = CollisionComp;

	WeaponMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh1P"));
	WeaponMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh1P->CastShadow = false;
	WeaponMesh1P->SetVisibility(true, true);
	WeaponMesh1P->SetupAttachment(CollisionComp);
	WeaponMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

	WeaponMesh3PickupRelativeLocation = FVector(0.0f, -25.0f, 0.0f);

	WeaponMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh3P"));
	WeaponMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh3P->SetupAttachment(CollisionComp);
	WeaponMesh3P->SetRelativeLocation(WeaponMesh3PickupRelativeLocation);
	WeaponMesh3P->CastShadow = true;
	WeaponMesh3P->SetVisibility(false, true);
	WeaponMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

	WeaponPrimaryInstantAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Weapon.Primary.Instant");
	WeaponSecondaryInstantAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Weapon.Secondary.Instant");
	WeaponAlternateInstantAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Weapon.Alternate.Instant");
	WeaponIsFiringTag = FGameplayTag::RequestGameplayTag("Weapon.IsFiring");

	FireMode = FGameplayTag::RequestGameplayTag("Weapon.FireMode.None");
	FireType = EFireMode::FullAuto;

	StatusText = DefaultStatusText;

	RestrictedPickupTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
	RestrictedPickupTags.AddTag(FGameplayTag::RequestGameplayTag("State.KnockedDown"));

	WeaponAttributeSet = CreateDefaultSubobject<UVTWeaponAttributeSetBase>(FName("AttributeSet"));

	WeaponGameplayAbilities.Empty();
	WeaponGameplayAbilities = {
		{FName("FirePrimary"), nullptr},
		{FName("FireSecondary"), nullptr},
		{FName("Reload"), nullptr},
		{FName("Aiming"), nullptr},
		{FName("SwitchPreCost"), nullptr},
	};
}

void AVTWeapon::StartCameraShake(float DeltaTime) const
{
	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			if (const UCameraShakeBase* ShakeInstance = PC->PlayerCameraManager->StartCameraShake(
				InCameraShakeClass,
				1.0f,
				ECameraShakePlaySpace::CameraLocal
			); ShakeInstance != nullptr)
			{
				UVTPerlinNoiseCameraShakePattern* Pattern = Cast<UVTPerlinNoiseCameraShakePattern>(ShakeInstance->GetRootShakePattern());
				Pattern->StartFiring(DeltaTime);
			}
		}
	}
}

UAbilitySystemComponent* AVTWeapon::GetAbilitySystemComponent() const
{
	if (OwningCharacter)
	{
		return OwningCharacter->GetAbilityComponent();
	}
	return nullptr;
}

UVTAbilitySystemComponent* AVTWeapon::GetAbilityComponent() const
{
	return Cast<UVTAbilitySystemComponent>(GetAbilitySystemComponent());
}

USkeletalMeshComponent* AVTWeapon::GetWeaponMesh1P() const
{
	return WeaponMesh1P;
}

USkeletalMeshComponent* AVTWeapon::GetWeaponMesh3P() const
{
	return WeaponMesh3P;
}

void AVTWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 指定一个属性在满足特定条件时才进行复制, COND_OwnerOnly：仅复制给该 Actor 的所有者
	DOREPLIFETIME_CONDITION(AVTWeapon, OwningCharacter, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AVTWeapon, PrimaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AVTWeapon, MaxPrimaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AVTWeapon, SecondaryClipAmmo, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AVTWeapon, MaxSecondaryClipAmmo, COND_OwnerOnly);
}

void AVTWeapon::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(AVTWeapon, PrimaryClipAmmo, (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(WeaponIsFiringTag)));
	DOREPLIFETIME_ACTIVE_OVERRIDE(AVTWeapon, SecondaryClipAmmo, (IsValid(AbilitySystemComponent) && !AbilitySystemComponent->HasMatchingGameplayTag(WeaponIsFiringTag)));
}

void AVTWeapon::SetOwningCharacter(AVTHeroCharacter* InOwningCharacter)
{
	OwningCharacter = InOwningCharacter;
	if (OwningCharacter)
	{
		// Called when added to inventory
		AbilitySystemComponent = Cast<UVTAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());
		SetOwner(InOwningCharacter);
		AttachToComponent(OwningCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (OwningCharacter->GetCurrentWeapon() != this)
		{
			WeaponMesh3P->CastShadow = false;
			WeaponMesh3P->SetVisibility(true, true);
			WeaponMesh3P->SetVisibility(false, true);
		}
	}
	else
	{
		AbilitySystemComponent = nullptr;
		SetOwner(nullptr);
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
}

void AVTWeapon::NotifyActorBeginOverlap(AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	if (IsValid(this) && !OwningCharacter)
	{
		PickUp(Cast<AVTHeroCharacter>(Other));
	}
}

void AVTWeapon::Equip_Implementation()
{
	if (!OwningCharacter)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s OwningCharacter is nullptr"), *FString(__FUNCTION__), *GetName());
		return;
	}

	const FName AttachPoint = OwningCharacter->GetWeaponAttachPoint();

	if (OwningCharacter->IsInFirstPersonPerspective())
	{
		WeaponMesh1P->AttachToComponent(OwningCharacter->GetFirstPersonMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, AttachPoint);
		OwningCharacter->UpdatePersonMeshLocation();
		// WeaponMesh1P->SetRelativeLocation(WeaponMesh1PEquippedRelativeLocation);
		// WeaponMesh1P->SetRelativeRotation(FRotator(0, 0, -90.0f));
		WeaponMesh1P->SetVisibility(true, true);
		WeaponMesh3P->SetVisibility(false, true);
	}
	else
	{
		WeaponMesh3P->AttachToComponent(OwningCharacter->GetThirdPersonMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, AttachPoint);
		WeaponMesh3P->SetRelativeLocation(WeaponMesh3PEquippedRelativeLocation);
		WeaponMesh3P->SetRelativeRotation(FRotator(0, 0, -90.0f));
		WeaponMesh3P->CastShadow = true;
		WeaponMesh3P->bCastHiddenShadow = true;
		WeaponMesh3P->SetVisibility(true, true);
		WeaponMesh1P->SetVisibility(false, true);
	}
	OwningCharacter->AddInstanceComponent(OwningCharacter->IsInFirstPersonPerspective() ? WeaponMesh1P : WeaponMesh3P);

	for (auto Ability : WeaponGameplayAbilities)
	{
		if (Ability.Value != nullptr)
		{
			FGameplayAbilitySpec AbilitySpecHandle = FGameplayAbilitySpec{
				Ability.Value, 1, -1, this
			};

			if (IsValid(AbilitySpecHandle.Ability))
			{
				if (const auto ASC = GetAbilityComponent(); ASC != nullptr)
				{
					GetAbilityComponent()->GiveAbility(AbilitySpecHandle);
				}
			}
		}
	}

	InitializeAttributes();
}

void AVTWeapon::UnEquip()
{
	if (OwningCharacter == nullptr)
	{
		return;
	}

	// Necessary to detach so that when toggling perspective all meshes attached won't become visible.

	WeaponMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh1P->SetVisibility(false, true);

	WeaponMesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	WeaponMesh3P->CastShadow = false;
	WeaponMesh3P->bCastHiddenShadow = false;
	WeaponMesh3P->SetVisibility(true, true); // Without this, the unequipped weapon's 3p shadow hangs around
	WeaponMesh3P->SetVisibility(false, true);
}

void AVTWeapon::AddAbilities()
{
	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UVTAbilitySystemComponent* ASC = Cast<UVTAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		UE_LOG(LogTemp, Error, TEXT("%s %s Role: %s ASC is null"), *FString(__FUNCTION__), *GetName(), GET_ACTOR_ROLE_FSTRING(OwningCharacter));
		return;
	}

	// Grant abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (TSubclassOf<UVTGameplayAbility>& Ability : Abilities)
	{
		AbilitySpecHandles.Add(ASC->GiveAbility(
			FGameplayAbilitySpec(Ability, 1, INDEX_NONE, this)));
	}
}

void AVTWeapon::RemoveAbilities()
{
	if (!IsValid(OwningCharacter) || !OwningCharacter->GetAbilitySystemComponent())
	{
		return;
	}

	UVTAbilitySystemComponent* ASC = Cast<UVTAbilitySystemComponent>(OwningCharacter->GetAbilitySystemComponent());

	if (!ASC)
	{
		return;
	}

	// Remove abilities, but only on the server	
	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	for (FGameplayAbilitySpecHandle& SpecHandle : AbilitySpecHandles)
	{
		ASC->ClearAbility(SpecHandle);
	}
}

int32 AVTWeapon::GetAbilityLevel(EVTAbilityInputID AbilityID)
{
	// All abilities for now are level 1
	return 1;
}

void AVTWeapon::ResetWeapon()
{
	FireMode = DefaultFireMode;
	StatusText = DefaultStatusText;
}

void AVTWeapon::OnDropped_Implementation(FVector NewLocation)
{
	SetOwningCharacter(nullptr);
	ResetWeapon();

	SetActorLocation(NewLocation);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	if (WeaponMesh1P)
	{
		WeaponMesh1P->AttachToComponent(CollisionComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
		WeaponMesh1P->SetVisibility(false, true);
	}

	if (WeaponMesh3P)
	{
		WeaponMesh3P->AttachToComponent(CollisionComp, FAttachmentTransformRules::SnapToTargetIncludingScale);
		WeaponMesh3P->SetRelativeLocation(WeaponMesh3PickupRelativeLocation);
		WeaponMesh3P->CastShadow = true;
		WeaponMesh3P->SetVisibility(true, true);
	}
}

bool AVTWeapon::OnDropped_Validate(FVector NewLocation)
{
	return true;
}

// void AVTWeapon::SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo)
// {
// 	int32 OldPrimaryClipAmmo = PrimaryClipAmmo;
// 	PrimaryClipAmmo = NewPrimaryClipAmmo;
// 	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
// }

TSubclassOf<UVTHUDReticle> AVTWeapon::GetPrimaryHUDReticleClass() const
{
	return PrimaryHUDReticleClass;
}

bool AVTWeapon::HasInfiniteAmmo() const
{
	return bInfiniteAmmo;
}

UAnimMontage* AVTWeapon::GetEquip1PMontage() const
{
	return Equip1PMontage;
}

UAnimMontage* AVTWeapon::GetEquip3PMontage() const
{
	return Equip3PMontage;
}

USoundCue* AVTWeapon::GetPickupSound() const
{
	return PickupSound;
}

FText AVTWeapon::GetDefaultStatusText() const
{
	return DefaultStatusText;
}

void AVTWeapon::InitializeAttributes() const
{
	GetAbilityComponent()->AddSpawnedAttribute(WeaponAttributeSet);

	if (!GetAbilityComponent()->GetSpawnedAttributes().Contains(WeaponAttributeSet))
	{
		UE_LOG(LogTemp, Error, TEXT("WeaponAttributeSet 注册失败！"));
		return;
	}

	if (!InitAttributeEffect)
	{
		UE_LOG(LogTemp, Error, TEXT("InitAttributeEffect 未配置！"));
		return;
	}

	if (UVTAbilitySystemComponent* ASC = GetAbilityComponent(); ASC != nullptr)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddInstigator(GetOwner(), GetOwner()); // 不指定Avatar和Owner会报错
		if (const FGameplayEffectSpecHandle Handle = ASC->MakeOutgoingSpec(InitAttributeEffect, 1, EffectContext); Handle.IsValid())
		{
			ASC->ApplyGameplayEffectSpecToSelf(*Handle.Data.Get());
		}
	}
}

AVTGATA_LineTrace* AVTWeapon::GetOrCreateLineTraceTargetActor()
{
	if (LineTraceTargetActor)
	{
		return LineTraceTargetActor;
	}

	LineTraceTargetActor = GetWorld()->SpawnActor<AVTGATA_LineTrace>();
	LineTraceTargetActor->SetOwner(this);
	return LineTraceTargetActor;
}

AVTGATA_SphereTrace* AVTWeapon::GetOrCreateSphereTraceTargetActor()
{
	if (SphereTraceTargetActor)
	{
		return SphereTraceTargetActor;
	}

	SphereTraceTargetActor = GetWorld()->SpawnActor<AVTGATA_SphereTrace>();
	SphereTraceTargetActor->SetOwner(this);
	return SphereTraceTargetActor;
}

float AVTWeapon::GetAttributeCurrentValue(const FGameplayAttribute Attribute) const
{
	float Value = 0.0f;
	bool bFound = false;
	if (const UAbilitySystemComponent* ASC = OwningCharacter->GetAbilitySystemComponent(); ASC != nullptr)
	{
		Value = ASC->GetGameplayAttributeValue(Attribute, bFound);
	}
	return bFound ? Value : 0.0f;
}

void AVTWeapon::BeginPlay()
{
	Super::BeginPlay();

	ResetWeapon();
	if (!OwningCharacter && bSpawnWithCollision)
	{
		// 拾取模式下启用碰撞
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void AVTWeapon::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (LineTraceTargetActor)
	{
		LineTraceTargetActor->Destroy();
	}

	if (SphereTraceTargetActor)
	{
		SphereTraceTargetActor->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

void AVTWeapon::PickUp(AVTHeroCharacter* InCharacter)
{
	if (!InCharacter || !InCharacter->IsAlive() || !InCharacter->GetAbilitySystemComponent() || InCharacter->GetAbilitySystemComponent()->HasAnyMatchingGameplayTags(RestrictedPickupTags))
	{
		return;
	}

	if (InCharacter->AddWeaponToInventory(this, true) && OwningCharacter->IsInFirstPersonPerspective())
	{
		WeaponMesh1P->CastShadow = false;
		WeaponMesh1P->SetVisibility(true, true);
		// WeaponMesh3P->SetVisibility(false, true);
	}
}

void AVTWeapon::OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo)
{
	OnPrimaryClipAmmoChanged.Broadcast(OldPrimaryClipAmmo, PrimaryClipAmmo);
}

void AVTWeapon::OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo)
{
	OnMaxPrimaryClipAmmoChanged.Broadcast(OldMaxPrimaryClipAmmo, MaxPrimaryClipAmmo);
}

void AVTWeapon::OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo)
{
	OnSecondaryClipAmmoChanged.Broadcast(OldSecondaryClipAmmo, SecondaryClipAmmo);
}

void AVTWeapon::OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo)
{
	OnMaxSecondaryClipAmmoChanged.Broadcast(OldMaxSecondaryClipAmmo, MaxSecondaryClipAmmo);
}
