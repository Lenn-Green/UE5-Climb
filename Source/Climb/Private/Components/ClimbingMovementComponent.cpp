#include "Components/ClimbingMovementComponent.h"

UClimbingMovementComponent::UClimbingMovementComponent()
{
	GravityScale = 1.0f;
}

void UClimbingMovementComponent::StartClimbingMovement()
{
	SetMovementMode(MOVE_Custom, CustomMovementModeClimbing);
}

void UClimbingMovementComponent::StartClimbingMovement(const FClimbingAttachmentFrame& AttachmentFrame)
{
	UpdateClimbingAttachmentFrame(AttachmentFrame);
	StartClimbingMovement();
}

void UClimbingMovementComponent::StartClimbingMovementWithAttachment(const FClimbingAttachmentFrame& AttachmentFrame)
{
	StartClimbingMovement(AttachmentFrame);
}

void UClimbingMovementComponent::StopClimbingMovement()
{
	SetMovementMode(MOVE_Falling);
	ClimbingAttachmentFrame = FClimbingAttachmentFrame();
}

void UClimbingMovementComponent::UpdateClimbingAttachmentFrame(const FClimbingAttachmentFrame& AttachmentFrame)
{
	ClimbingAttachmentFrame = AttachmentFrame;
	ClimbingAttachmentFrame.TargetWallDistance = AttachmentFrame.TargetWallDistance > 0.0f
		? AttachmentFrame.TargetWallDistance
		: TargetWallDistance;
}

bool UClimbingMovementComponent::IsInClimbingMovementMode() const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == CustomMovementModeClimbing;
}

FClimbingAttachmentFrame UClimbingMovementComponent::GetClimbingAttachmentFrame() const
{
	return ClimbingAttachmentFrame;
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

	Velocity = FVector::ZeroVector;

	if (!UpdatedComponent || !ClimbingAttachmentFrame.bIsValid)
	{
		return;
	}

	const FVector CurrentLocation = UpdatedComponent->GetComponentLocation();
	const FVector TargetLocation = GetClimbingTargetLocation();
	const float Alpha = FMath::Clamp(AttachInterpSpeed * DeltaTime, 0.0f, 1.0f);
	const FVector MoveDelta = (TargetLocation - CurrentLocation) * Alpha;

	if (!MoveDelta.IsNearlyZero())
	{
		MoveUpdatedComponent(MoveDelta, UpdatedComponent->GetComponentQuat(), true);
	}
}

FVector UClimbingMovementComponent::GetClimbingTargetLocation() const
{
	return ClimbingAttachmentFrame.AnchorLocation + ClimbingAttachmentFrame.WallNormal.GetSafeNormal() * ClimbingAttachmentFrame.TargetWallDistance;
}
