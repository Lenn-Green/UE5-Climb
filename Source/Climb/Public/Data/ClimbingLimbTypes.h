#pragma once

#include "CoreMinimal.h"
#include "ClimbingLimbTypes.generated.h"

UENUM(BlueprintType)
enum class EClimbingLimb : uint8
{
	LeftHand UMETA(DisplayName = "Left Hand"),
	RightHand UMETA(DisplayName = "Right Hand"),
	LeftFoot UMETA(DisplayName = "Left Foot"),
	RightFoot UMETA(DisplayName = "Right Foot")
};

USTRUCT(BlueprintType)
struct CLIMB_API FLimbState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb")
	EClimbingLimb Limb = EClimbingLimb::LeftHand;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb")
	bool bHasValidContact = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb")
	bool bIsLocked = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb")
	FVector ContactLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb")
	FVector ContactNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb")
	FRotator ContactRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LoadPercent = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb")
	TObjectPtr<AActor> ContactActor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Limb")
	TObjectPtr<UPrimitiveComponent> ContactComponent = nullptr;
};
