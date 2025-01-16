// Copyright 2024 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"

#include "Characters/VTCharacterBase.h"
#include "..\Abilities\VTInteractable.h"
#include "VTHeroCharacter.generated.h"

class AVTWeapon;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EVTHeroWeaponState : uint8
{
	// 0
	Rifle					UMETA(DisplayName = "Rifle"),
	// 1
	RifleAiming				UMETA(DisplayName = "Rifle Aiming"),
	// 2
	RocketLauncher			UMETA(DisplayName = "Rocket Launcher"),
	// 3
	RocketLauncherAiming	UMETA(DisplayName = "Rocket Launcher Aiming")
};

USTRUCT()
struct LUGAMEPLAYFRAME_API FVTHeroInventory
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<AVTWeapon*> Weapons;

	// Consumable items

	// Passive items like armor

	// Door keys

	// Etc
};

/**
 * A player or AI controlled hero character.
 */
UCLASS()
class LUGAMEPLAYFRAME_API AVTHeroCharacter : public AVTCharacterBase, public IVTInteractable
{
	GENERATED_BODY()
	
public:
	AVTHeroCharacter(const class FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Valorant|GSHeroCharacter")
	bool bStartInFirstPersonPerspective;

	FGameplayTag CurrentWeaponTag;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Only called on the Server. Calls before Server's AcknowledgePossession.
	virtual void PossessedBy(AController* NewController) override;

	class UVTFloatingStatusBarWidget* GetFloatingStatusBar();

	// Server handles knockdown - cancel abilities, remove effects, activate knockdown ability
	virtual void KnockDown();

	// Plays knockdown effects for all clients from KnockedDown tag listener on PlayerState
	virtual void PlayKnockDownEffects();

	// Plays revive effects for all clients from KnockedDown tag listener on PlayerState
	virtual void PlayReviveEffects();

	virtual void FinishDying() override;

	UFUNCTION(BlueprintCallable, Category = "Valorant|GSHeroCharacter")
	virtual bool IsInFirstPersonPerspective() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Valorant|GSHeroCharacter")
	USkeletalMeshComponent* GetFirstPersonMesh() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Valorant|GSHeroCharacter")
	USkeletalMeshComponent* GetThirdPersonMesh() const;

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	AVTWeapon* GetCurrentWeapon() const;

	// Adds a new weapon to the inventory.
	// Returns false if the weapon already exists in the inventory, true if it's a new weapon.
	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	bool AddWeaponToInventory(AVTWeapon* NewWeapon, bool bEquipWeapon = false);

	// Removes a weapon from the inventory.
	// Returns true if the weapon exists and was removed. False if the weapon didn't exist in the inventory.
	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	bool RemoveWeaponFromInventory(AVTWeapon* WeaponToRemove);

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	void RemoveAllWeaponsFromInventory();

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	void EquipWeapon(AVTWeapon* NewWeapon);

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(AVTWeapon* NewWeapon);
	void ServerEquipWeapon_Implementation(AVTWeapon* NewWeapon);
	bool ServerEquipWeapon_Validate(AVTWeapon* NewWeapon);

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	virtual void NextWeapon();

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	virtual void PreviousWeapon();

	FName GetWeaponAttachPoint();

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	int32 GetPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	int32 GetMaxPrimaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	int32 GetPrimaryReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	int32 GetSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	int32 GetMaxSecondaryClipAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	int32 GetSecondaryReserveAmmo() const;

	UFUNCTION(BlueprintCallable, Category = "Valorant|Inventory")
	int32 GetNumWeapons() const;


	/**
	* Interactable interface
	*/

	/**
	* We can Interact with other heroes when:
	* Knocked Down - to revive them
	*/
	virtual bool IsAvailableForInteraction_Implementation(UPrimitiveComponent* InteractionComponent) const override;

	/**
	* How long to interact with this player:
	* Knocked Down - to revive them
	*/
	virtual float GetInteractionDuration_Implementation(UPrimitiveComponent* InteractionComponent) const override;

	/**
	* Interaction:
	* Knocked Down - activate revive GA (plays animation)
	*/
	virtual void PreInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent) override;

	/**
	* Interaction:
	* Knocked Down - apply revive GE
	*/
	virtual void PostInteract_Implementation(AActor* InteractingActor, UPrimitiveComponent* InteractionComponent) override;

	/**
	* Should we wait and who should wait to sync before calling PreInteract():
	* Knocked Down - Yes, client. This will sync the local player's Interact Duration Timer with the knocked down player's
	* revive animation. If we had a picking a player up animation, we could play it on the local player in PreInteract().
	*/
	virtual void GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type, UPrimitiveComponent* InteractionComponent) const override;

	/**
	* Cancel interaction:
	* Knocked Down - cancel revive ability
	*/
	virtual void CancelInteraction_Implementation(UPrimitiveComponent* InteractionComponent) override;

	/**
	* Get the delegate for this Actor canceling interaction:
	* Knocked Down - cancel being revived if killed
	*/
	FSimpleMulticastDelegate* GetTargetCancelInteractionDelegate(UPrimitiveComponent* InteractionComponent) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Valorant|GSHeroCharacter")
	FVector StartingThirdPersonMeshLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Valorant|GSHeroCharacter")
	FVector StartingFirstPersonMeshLocation;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Valorant|Abilities")
	float ReviveDuration;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Valorant|Camera")
	float BaseTurnRate;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Valorant|Camera")
	float BaseLookUpRate;

	UPROPERTY(BlueprintReadOnly, Category = "Valorant|Camera")
	float StartingThirdPersonCameraBoomArmLength;

	UPROPERTY(BlueprintReadOnly, Category = "Valorant|Camera")
	FVector StartingThirdPersonCameraBoomLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Valorant|Camera")
	bool bIsFirstPersonPerspective;

	UPROPERTY(BlueprintReadOnly, Category = "Valorant|GSHeroCharacter")
	bool bWasInFirstPersonPerspectiveWhenKnockedDown;

	bool bASCInputBound;

	// Set to true when we change the weapon predictively and flip it to false when the Server replicates to confirm.
	// We use this if the Server refused a weapon change ability's activation to ask the Server to sync the client back up
	// with the correct CurrentWeapon.
	bool bChangedWeaponLocally;

	UPROPERTY(BlueprintReadOnly, Category = "Valorant|Camera")
	float Default1PFOV;

	UPROPERTY(BlueprintReadOnly, Category = "Valorant|Camera")
	float Default3PFOV;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Valorant|GSHeroCharacter")
	FName WeaponAttachPoint;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Valorant|Camera")
	class USpringArmComponent* ThirdPersonCameraBoom;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Valorant|Camera")
	class UCameraComponent* ThirdPersonCamera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Valorant|Camera")
	class UCameraComponent* FirstPersonCamera;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	USkeletalMeshComponent* FirstPersonMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Valorant|UI")
	TSubclassOf<class UVTFloatingStatusBarWidget> UIFloatingStatusBarClass;

	UPROPERTY()
	class UVTFloatingStatusBarWidget* UIFloatingStatusBar;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Valorant|UI")
	class UWidgetComponent* UIFloatingStatusBarComponent;

	UPROPERTY(ReplicatedUsing = OnRep_Inventory)
	FVTHeroInventory Inventory;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Valorant|Inventory")
	TArray<TSubclassOf<AVTWeapon>> DefaultInventoryWeaponClasses;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	AVTWeapon* CurrentWeapon;

	UPROPERTY()
	class UVTAmmoAttributeSet* AmmoAttributeSet;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Valorant|GSHeroCharacter")
	TSubclassOf<UGameplayEffect> KnockDownEffect;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Valorant|GSHeroCharacter")
	TSubclassOf<UGameplayEffect> ReviveEffect;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Valorant|GSHeroCharacter")
	TSubclassOf<UGameplayEffect> DeathEffect;

	FSimpleMulticastDelegate InteractionCanceledDelegate;

	// Cache tags
	FGameplayTag NoWeaponTag;
	FGameplayTag WeaponChangingDelayReplicationTag;
	FGameplayTag WeaponAmmoTypeNoneTag;
	FGameplayTag WeaponAbilityTag;
	FGameplayTag KnockedDownTag;
	FGameplayTag InteractingTag;

	// Attribute changed delegate handles
	FDelegateHandle PrimaryReserveAmmoChangedDelegateHandle;
	FDelegateHandle SecondaryReserveAmmoChangedDelegateHandle;

	// Tag changed delegate handles
	FDelegateHandle WeaponChangingDelayReplicationTagChangedDelegateHandle;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PostInitializeComponents() override;

	// Mouse
	void LookUp(float Value);

	// Gamepad
	void LookUpRate(float Value);

	// Mouse
	void Turn(float Value);

	// Gamepad
	void TurnRate(float Value);

	// Mouse + Gamepad
	void MoveForward(float Value);

	// Mouse + Gamepad
	void MoveRight(float Value);

	// Toggles between perspectives
	void TogglePerspective();

	// Sets the perspective
	void SetPerspective(bool Is1PPerspective);

	// Creates and initializes the floating status bar for heroes.
	// Safe to call many times because it checks to make sure it only executes once.
	UFUNCTION()
	void InitializeFloatingStatusBar();

	// Client only
	virtual void OnRep_PlayerState() override;
	virtual void OnRep_Controller() override;

	// Called from both SetupPlayerInputComponent and OnRep_PlayerState because of a potential race condition where the PlayerController might
	// call ClientRestart which calls SetupPlayerInputComponent before the PlayerState is repped to the client so the PlayerState would be null in SetupPlayerInputComponent.
	// Conversely, the PlayerState might be repped before the PlayerController calls ClientRestart so the Actor's InputComponent would be null in OnRep_PlayerState.
	void BindASCInput();

	// Server spawns default inventory
	void SpawnDefaultInventory();

	void SetupStartupPerspective();

	bool DoesWeaponExistInInventory(AVTWeapon* InWeapon);

	void SetCurrentWeapon(AVTWeapon* NewWeapon, AVTWeapon* LastWeapon);

	// Unequips the specified weapon. Used when OnRep_CurrentWeapon fires.
	void UnEquipWeapon(AVTWeapon* WeaponToUnEquip);

	// Unequips the current weapon. Used if for example we drop the current weapon.
	void UnEquipCurrentWeapon();

	UFUNCTION()
	virtual void CurrentWeaponPrimaryClipAmmoChanged(int32 OldPrimaryClipAmmo, int32 NewPrimaryClipAmmo);

	UFUNCTION()
	virtual void CurrentWeaponSecondaryClipAmmoChanged(int32 OldSecondaryClipAmmo, int32 NewSecondaryClipAmmo);

	// Attribute changed callbacks
	virtual void CurrentWeaponPrimaryReserveAmmoChanged(const FOnAttributeChangeData& Data);
	virtual void CurrentWeaponSecondaryReserveAmmoChanged(const FOnAttributeChangeData& Data);

	// Tag changed callbacks
	virtual void WeaponChangingDelayReplicationTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UFUNCTION()
	void OnRep_CurrentWeapon(AVTWeapon* LastWeapon);

	UFUNCTION()
	void OnRep_Inventory();

	void OnAbilityActivationFailed(const UGameplayAbility* FailedAbility, const FGameplayTagContainer& FailTags);
	
	// The CurrentWeapon is only automatically replicated to simulated clients.
	// The autonomous client can use this to request the proper CurrentWeapon from the server when it knows it may be
	// out of sync with it from predictive client-side changes.
	UFUNCTION(Server, Reliable)
	void ServerSyncCurrentWeapon();
	void ServerSyncCurrentWeapon_Implementation();
	bool ServerSyncCurrentWeapon_Validate();
	
	// The CurrentWeapon is only automatically replicated to simulated clients.
	// Use this function to manually sync the autonomous client's CurrentWeapon when we're ready to.
	// This allows us to predict weapon changes (changing weapons fast multiple times in a row so that the server doesn't
	// replicate and clobber our CurrentWeapon).
	UFUNCTION(Client, Reliable)
	void ClientSyncCurrentWeapon(AVTWeapon* InWeapon);
	void ClientSyncCurrentWeapon_Implementation(AVTWeapon* InWeapon);
	bool ClientSyncCurrentWeapon_Validate(AVTWeapon* InWeapon);
};
