// Copyright 2024 Dan Kestranek.


#include "Characters/VTCharacterMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Characters/Abilities/GSAbilitySystemGlobals.h"
#include "Characters/VTCharacterBase.h"
#include "GameplayTagContainer.h"

UVTCharacterMovementComponent::UVTCharacterMovementComponent()
{
	SprintSpeedMultiplier = 1.4f;
	ADSSpeedMultiplier = 0.8f;
	KnockedDownSpeedMultiplier = 0.4f;

	KnockedDownTag = FGameplayTag::RequestGameplayTag("State.KnockedDown");
	InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
	InteractingRemovalTag = FGameplayTag::RequestGameplayTag("State.InteractingRemoval");
}

float UVTCharacterMovementComponent::GetMaxSpeed() const
{
	AVTCharacterBase* Owner = Cast<AVTCharacterBase>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Error, TEXT("%s() No Owner"), *FString(__FUNCTION__));
		return Super::GetMaxSpeed();
	}

	if (!Owner->IsAlive())
	{
		return 0.0f;
	}

	// Don't move while interacting or being interacted on (revived)
	if (Owner->GetAbilitySystemComponent() && Owner->GetAbilitySystemComponent()->GetTagCount(InteractingTag)
		> Owner->GetAbilitySystemComponent()->GetTagCount(InteractingRemovalTag))
	{
		return 0.0f;
	}

	if (Owner->GetAbilitySystemComponent() && Owner->GetAbilitySystemComponent()->HasMatchingGameplayTag(KnockedDownTag))
	{
		return Owner->GetMoveSpeed() * KnockedDownSpeedMultiplier;
	}

	if (RequestToStartSprinting)
	{
		return Owner->GetMoveSpeed() * SprintSpeedMultiplier;
	}

	if (RequestToStartADS)
	{
		return Owner->GetMoveSpeed() * ADSSpeedMultiplier;
	}

	return Owner->GetMoveSpeed();
}

void UVTCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	//The Flags parameter contains the compressed input flags that are stored in the saved move.
	//UpdateFromCompressed flags simply copies the flags from the saved move into the movement component.
	//It basically just resets the movement component to the state when the move was made so it can simulate from there.
	RequestToStartSprinting = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;

	RequestToStartADS = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

FNetworkPredictionData_Client* UVTCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != NULL);

	if (!ClientPredictionData)
	{
		UVTCharacterMovementComponent* MutableThis = const_cast<UVTCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FVTNetworkPredictionData_Client(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UVTCharacterMovementComponent::StartSprinting()
{
	RequestToStartSprinting = true;
}

void UVTCharacterMovementComponent::StopSprinting()
{
	RequestToStartSprinting = false;
}

void UVTCharacterMovementComponent::StartAimDownSights()
{
	RequestToStartADS = true;
}

void UVTCharacterMovementComponent::StopAimDownSights()
{
	RequestToStartADS = false;
}

void UVTCharacterMovementComponent::FVTSavedMove::Clear()
{
	Super::Clear();

	SavedRequestToStartSprinting = false;
	SavedRequestToStartADS = false;
}

uint8 UVTCharacterMovementComponent::FVTSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (SavedRequestToStartSprinting)
	{
		Result |= FLAG_Custom_0;
	}

	if (SavedRequestToStartADS)
	{
		Result |= FLAG_Custom_1;
	}

	return Result;
}

bool UVTCharacterMovementComponent::FVTSavedMove::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* Character, float MaxDelta) const
{
	//Set which moves can be combined together. This will depend on the bit flags that are used.
	if (SavedRequestToStartSprinting != ((FVTSavedMove*)NewMove.Get())->SavedRequestToStartSprinting)
	{
		return false;
	}

	if (SavedRequestToStartADS != ((FVTSavedMove*)NewMove.Get())->SavedRequestToStartADS)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void UVTCharacterMovementComponent::FVTSavedMove::SetMoveFor(ACharacter* Character, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	UVTCharacterMovementComponent* CharacterMovement = Cast<UVTCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
		SavedRequestToStartSprinting = CharacterMovement->RequestToStartSprinting;
		SavedRequestToStartADS = CharacterMovement->RequestToStartADS;
	}
}

void UVTCharacterMovementComponent::FVTSavedMove::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	UVTCharacterMovementComponent* CharacterMovement = Cast<UVTCharacterMovementComponent>(Character->GetCharacterMovement());
	if (CharacterMovement)
	{
	}
}

UVTCharacterMovementComponent::FVTNetworkPredictionData_Client::FVTNetworkPredictionData_Client(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UVTCharacterMovementComponent::FVTNetworkPredictionData_Client::AllocateNewMove()
{
	return FSavedMovePtr(new FVTSavedMove());
}

