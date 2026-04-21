#include "Character/ClimbingCharacter.h"

#include "Components/ClimbingMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"

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
	AddClimbingInputMappingContext();
}

void AClimbingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	AddClimbingInputMappingContext();
	BindClimbingInputActions(PlayerInputComponent);
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

FVector2D AClimbingCharacter::GetClimbCenterOfMassInput() const
{
	return ClimbCenterOfMassInput;
}

FVector2D AClimbingCharacter::GetClimbLimbProbeInput() const
{
	return ClimbLimbProbeInput;
}

float AClimbingCharacter::GetLeftGripInput() const
{
	return LeftGripInput;
}

float AClimbingCharacter::GetRightGripInput() const
{
	return RightGripInput;
}

void AClimbingCharacter::AddClimbingInputMappingContext() const
{
	if (!ClimbingInputMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("Climbing input mapping context is not assigned on %s."), *GetName());
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		InputSubsystem->AddMappingContext(ClimbingInputMappingContext, ClimbingInputMappingPriority);
	}
}

void AClimbingCharacter::BindClimbingInputActions(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AClimbingCharacter requires UEnhancedInputComponent for climbing controls."));
		return;
	}

	if (ClimbCenterOfMassMoveAction)
	{
		EnhancedInputComponent->BindAction(ClimbCenterOfMassMoveAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbCenterOfMassMove);
		EnhancedInputComponent->BindAction(ClimbCenterOfMassMoveAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbCenterOfMassMoveCompleted);
	}

	if (ClimbLimbProbeAction)
	{
		EnhancedInputComponent->BindAction(ClimbLimbProbeAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbLimbProbe);
		EnhancedInputComponent->BindAction(ClimbLimbProbeAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbLimbProbeCompleted);
	}

	if (ClimbLeftGripAction)
	{
		EnhancedInputComponent->BindAction(ClimbLeftGripAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbLeftGrip);
		EnhancedInputComponent->BindAction(ClimbLeftGripAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbLeftGripCompleted);
	}

	if (ClimbRightGripAction)
	{
		EnhancedInputComponent->BindAction(ClimbRightGripAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbRightGrip);
		EnhancedInputComponent->BindAction(ClimbRightGripAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbRightGripCompleted);
	}
}

void AClimbingCharacter::HandleClimbCenterOfMassMove(const FInputActionValue& Value)
{
	ClimbCenterOfMassInput = Value.Get<FVector2D>();
}

void AClimbingCharacter::HandleClimbCenterOfMassMoveCompleted(const FInputActionValue& Value)
{
	ClimbCenterOfMassInput = FVector2D::ZeroVector;
}

void AClimbingCharacter::HandleClimbLimbProbe(const FInputActionValue& Value)
{
	ClimbLimbProbeInput = Value.Get<FVector2D>();
}

void AClimbingCharacter::HandleClimbLimbProbeCompleted(const FInputActionValue& Value)
{
	ClimbLimbProbeInput = FVector2D::ZeroVector;
}

void AClimbingCharacter::HandleClimbLeftGrip(const FInputActionValue& Value)
{
	LeftGripInput = Value.Get<float>();
}

void AClimbingCharacter::HandleClimbLeftGripCompleted(const FInputActionValue& Value)
{
	LeftGripInput = 0.0f;
}

void AClimbingCharacter::HandleClimbRightGrip(const FInputActionValue& Value)
{
	RightGripInput = Value.Get<float>();
}

void AClimbingCharacter::HandleClimbRightGripCompleted(const FInputActionValue& Value)
{
	RightGripInput = 0.0f;
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
