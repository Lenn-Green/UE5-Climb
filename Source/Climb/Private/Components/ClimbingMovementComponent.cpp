#include "Components/ClimbingMovementComponent.h"

UClimbingMovementComponent::UClimbingMovementComponent()
{
	GravityScale = 1.0f;
}

void UClimbingMovementComponent::StartClimbingMovement()
{
	SetMovementMode(MOVE_Custom, CustomMovementModeClimbing);
}

void UClimbingMovementComponent::StopClimbingMovement()
{
	SetMovementMode(MOVE_Falling);
}

bool UClimbingMovementComponent::IsInClimbingMovementMode() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == CustomMovementModeClimbing;
}

void UClimbingMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	if (CustomMovementMode == CustomMovementModeClimbing)
	{
		PhysClimbing(DeltaTime, Iterations);
		return;
	}

	Super::PhysCustom(DeltaTime, Iterations);
}

void UClimbingMovementComponent::PhysClimbing(float DeltaTime, int32 Iterations)
{
	if (DeltaTime < MIN_TICK_TIME)
	{
		return;
	}

	// Phase 1 keeps climbing motion locked in place until CoM and hold solvers exist.
	Velocity = FVector::ZeroVector;
	MoveUpdatedComponent(FVector::ZeroVector, UpdatedComponent ? UpdatedComponent->GetComponentQuat() : FQuat::Identity, true);
}
