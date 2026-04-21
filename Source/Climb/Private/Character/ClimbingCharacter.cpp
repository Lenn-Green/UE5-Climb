#include "Character/ClimbingCharacter.h"

#include "Components/ClimbingMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AClimbingCharacter::AClimbingCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UClimbingMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
}

void AClimbingCharacter::BeginPlay()
{
	Super::BeginPlay();

	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	SetClimbingState(Movement && Movement->IsMovingOnGround() ? EClimbingState::Grounded : EClimbingState::Falling);
}

void AClimbingCharacter::EnterClimbing()
{
	if (UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent())
	{
		ClimbingMovement->StartClimbingMovement();
	}

	SetClimbingState(EClimbingState::Climbing);
}

void AClimbingCharacter::ExitClimbing()
{
	if (UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent())
	{
		ClimbingMovement->StopClimbingMovement();
	}

	SetClimbingState(EClimbingState::Falling);
}

void AClimbingCharacter::PrepareClimbingJump()
{
	if (ClimbingState == EClimbingState::Climbing)
	{
		SetClimbingState(EClimbingState::PreparingJump);
	}
}

EClimbingState AClimbingCharacter::GetClimbingState() const
{
	return ClimbingState;
}

bool AClimbingCharacter::IsClimbing() const
{
	return ClimbingState == EClimbingState::Climbing || ClimbingState == EClimbingState::PreparingJump;
}

UClimbingMovementComponent* AClimbingCharacter::GetClimbingMovementComponent() const
{
	return Cast<UClimbingMovementComponent>(GetCharacterMovement());
}

void AClimbingCharacter::SetClimbingState(EClimbingState NewState)
{
	if (ClimbingState == NewState)
	{
		return;
	}

	const EClimbingState PreviousState = ClimbingState;
	ClimbingState = NewState;

	UE_LOG(LogTemp, Log, TEXT("Climbing state changed: %s -> %s"), *UEnum::GetValueAsString(PreviousState), *UEnum::GetValueAsString(ClimbingState));
}
