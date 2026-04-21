#pragma once

#include "CoreMinimal.h"
#include "ClimbingHoldTypes.generated.h"

USTRUCT(BlueprintType)
struct CLIMB_API FClimbingHoldCandidate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Hold")
	bool bIsValid = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Hold")
	FVector Location = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Hold")
	FVector Normal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Hold")
	TObjectPtr<AActor> Actor = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Hold")
	TObjectPtr<UPrimitiveComponent> Component = nullptr;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Hold")
	float Distance = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Hold")
	float Score = 0.0f;
};
