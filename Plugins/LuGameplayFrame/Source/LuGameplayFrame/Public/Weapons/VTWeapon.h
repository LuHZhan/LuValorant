// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "Characters/Abilities/VTAbilityTypes.h"
#include "VTWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWeaponAmmoChangedDelegate, int32, OldValue, int32, NewValue);

class AVTGATA_LineTrace;
class AVTGATA_SphereTrace;
class AVTHeroCharacter;
class UAnimMontage;
class UVTAbilitySystemComponent;
class UVTGameplayAbility;
class UPaperSprite;
class USkeletalMeshComponent;

UCLASS(Blueprintable, BlueprintType)
class LUGAMEPLAYFRAME_API AVTWeapon : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AVTWeapon();

	// Whether or not to spawn this weapon with collision enabled (pickup mode).
	// Set to false when spawning directly into a player's inventory or true when spawning into the world in pickup mode.
	UPROPERTY(BlueprintReadWrite)
	bool bSpawnWithCollision;

	// This tag is primarily used by the first person Animation Blueprint to determine which animations to play
	// (Rifle vs Rocket Launcher)
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|Weapon")
	FGameplayTag WeaponTag;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|Weapon")
	FGameplayTagContainer RestrictedPickupTags;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* PrimaryIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* SecondaryIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* PrimaryClipIcon;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	UPaperSprite* SecondaryClipIcon;

	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|Weapon")
	FGameplayTag FireMode;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|Weapon")
	FGameplayTag PrimaryAmmoType;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "GASShooter|Weapon")
	FGameplayTag SecondaryAmmoType;

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadWrite, VisibleInstanceOnly, Category = "GASShooter|Weapon")
	FText StatusText;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|Weapon")
	FWeaponAmmoChangedDelegate OnPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|Weapon")
	FWeaponAmmoChangedDelegate OnMaxPrimaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|Weapon")
	FWeaponAmmoChangedDelegate OnSecondaryClipAmmoChanged;

	UPROPERTY(BlueprintAssignable, Category = "GASShooter|Weapon")
	FWeaponAmmoChangedDelegate OnMaxSecondaryClipAmmoChanged;

	// Implement IAbilitySystemInterface
	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASShooter|Weapon")
	virtual USkeletalMeshComponent* GetWeaponMesh1P() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GASShooter|Weapon")
	virtual USkeletalMeshComponent* GetWeaponMesh3P() const;

	/**
	 * 返回用于网络复制的属性
	 * @param OutLifetimeProps 
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	void SetOwningCharacter(AVTHeroCharacter* InOwningCharacter);

	// Collision Overlap
	virtual void NotifyActorBeginOverlap(class AActor* Other) override;

	virtual int32 GetAbilityLevel(EVTAbilityInputID AbilityID);
	virtual void AddAbilities();
	virtual void RemoveAbilities();

	// ---------------- Function ----------------


	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void PickUp(AVTHeroCharacter* InCharacter);

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void Equip();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void UnEquip();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void ResetWeapon();

	UFUNCTION(NetMulticast, Reliable)
	void OnDropped(FVector NewLocation);
	virtual void OnDropped_Implementation(FVector NewLocation);
	virtual bool OnDropped_Validate(FVector NewLocation);

	// Getter for LineTraceTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Targeting")
	AVTGATA_LineTrace* GetLineTraceTargetActor();

	// Getter for SphereTraceTargetActor. Spawns it if it doesn't exist yet.
	UFUNCTION(BlueprintCallable, Category = "GASShooter|Targeting")
	AVTGATA_SphereTrace* GetSphereTraceTargetActor();

	// ================ Function ================

	// ---------------- Getting and Setting ----------------

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual int32 GetMaxPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual int32 GetMaxSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual void SetPrimaryClipAmmo(int32 NewPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual void SetMaxPrimaryClipAmmo(int32 NewMaxPrimaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual void SetSecondaryClipAmmo(int32 NewSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual void SetMaxSecondaryClipAmmo(int32 NewMaxSecondaryClipAmmo);

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	TSubclassOf<class UVTHUDReticle> GetPrimaryHUDReticleClass() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	virtual bool HasInfiniteAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Animation")
	UAnimMontage* GetEquip1PMontage() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Animation")
	UAnimMontage* GetEquip3PMontage() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Audio")
	class USoundCue* GetPickupSound() const;

	UFUNCTION(BlueprintCallable, Category = "GASShooter|Weapon")
	FText GetDefaultStatusText() const;

	// ========= Getting and Setting =========

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="ASC")
	UVTAbilitySystemComponent* AbilitySystemComponent;

	// How much ammo in the clip the gun starts with
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_PrimaryClipAmmo, Category = "GASShooter|Weapon|Ammo")
	int32 PrimaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxPrimaryClipAmmo, Category = "GASShooter|Weapon|Ammo")
	int32 MaxPrimaryClipAmmo;

	// How much ammo in the clip the gun starts with. Used for things like rifle grenades.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_SecondaryClipAmmo, Category = "GASShooter|Weapon|Ammo")
	int32 SecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, ReplicatedUsing = OnRep_MaxSecondaryClipAmmo, Category = "GASShooter|Weapon|Ammo")
	int32 MaxSecondaryClipAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|Weapon|Ammo")
	bool bInfiniteAmmo;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|UI")
	TSubclassOf<class UVTHUDReticle> PrimaryHUDReticleClass;

	UPROPERTY()
	AVTGATA_LineTrace* LineTraceTargetActor;

	UPROPERTY()
	AVTGATA_SphereTrace* SphereTraceTargetActor;

	// Collision capsule for when weapon is in pickup mode
	UPROPERTY(VisibleAnywhere)
	class UCapsuleComponent* CollisionComp;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|Weapon")
	USkeletalMeshComponent* WeaponMesh1P;

	UPROPERTY(VisibleAnywhere, Category = "GASShooter|Weapon")
	USkeletalMeshComponent* WeaponMesh3P;

	// Relative Location of weapon 3P Mesh when in pickup mode
	// 1P weapon mesh is invisible so it doesn't need one
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|Weapon")
	FVector WeaponMesh3PickupRelativeLocation;

	// Relative Location of weapon 1P Mesh when equipped
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|Weapon")
	FVector WeaponMesh1PEquippedRelativeLocation;

	// Relative Location of weapon 3P Mesh when equipped
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|Weapon")
	FVector WeaponMesh3PEquippedRelativeLocation;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "GASShooter|Weapon")
	AVTHeroCharacter* OwningCharacter;

	UPROPERTY(EditAnywhere, Category = "GASShooter|Weapon")
	TArray<TSubclassOf<UVTGameplayAbility>> Abilities;

	UPROPERTY(BlueprintReadOnly, Category = "GASShooter|Weapon")
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GASShooter|Weapon")
	FGameplayTag DefaultFireMode;

	// Things like fire mode for rifle
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|Weapon")
	FText DefaultStatusText;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "GASShooter|Animation")
	UAnimMontage* Equip1PMontage;

	UPROPERTY(BlueprintReadonly, EditAnywhere, Category = "GASShooter|Animation")
	UAnimMontage* Equip3PMontage;

	// Sound played when player picks it up
	UPROPERTY(EditDefaultsOnly, Category = "GASShooter|Audio")
	class USoundCue* PickupSound;

	// Cache tags
	FGameplayTag WeaponPrimaryInstantAbilityTag;
	FGameplayTag WeaponSecondaryInstantAbilityTag;
	FGameplayTag WeaponAlternateInstantAbilityTag;
	FGameplayTag WeaponIsFiringTag;

	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;


	UFUNCTION()
	virtual void OnRep_PrimaryClipAmmo(int32 OldPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxPrimaryClipAmmo(int32 OldMaxPrimaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_SecondaryClipAmmo(int32 OldSecondaryClipAmmo);

	UFUNCTION()
	virtual void OnRep_MaxSecondaryClipAmmo(int32 OldMaxSecondaryClipAmmo);
};
