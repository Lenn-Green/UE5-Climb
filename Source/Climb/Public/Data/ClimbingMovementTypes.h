#pragma once

#include "CoreMinimal.h"
#include "ClimbingMovementTypes.generated.h"

USTRUCT(BlueprintType)
struct CLIMB_API FClimbingAttachmentFrame
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Movement")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Movement")
	FVector ContactLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Movement")
	FVector WallNormal = FVector::ForwardVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Movement")
	FVector AnchorLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Movement")
	FVector WallRight = FVector::RightVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Movement")
	FVector WallUp = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Movement", meta=(ClampMin="0.0", Units="cm"))
	float TargetWallDistance = 45.0f;
};
