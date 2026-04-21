#pragma once

#include "CoreMinimal.h"
#include "ClimbingTypes.generated.h"

UENUM(BlueprintType)
enum class EClimbingState : uint8
{
	Grounded UMETA(DisplayName = "Grounded"),
	Falling UMETA(DisplayName = "Falling"),
	Climbing UMETA(DisplayName = "Climbing"),
	PreparingJump UMETA(DisplayName = "Preparing Jump")
};
