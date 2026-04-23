#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ClimbingSolver.generated.h"

USTRUCT(BlueprintType)
struct CLIMB_API FClimbingStabilityResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Solver")
	bool bHasSupport = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Solver")
	bool bIsStable = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Solver")
	FVector ProjectedCenterOfMass = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Solver")
	FVector NearestSupportPoint = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Solver")
	float SignedDistanceToSupportLine = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Solver", meta=(ClampMin="0.0", ClampMax="1.0"))
	float StabilityPercent = 0.0f;
};

UCLASS()
class CLIMB_API UClimbingSolver : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	static constexpr float DefaultStableOffsetCm = 75.0f;
	static constexpr float DefaultMaxBodyTensionOffsetCm = 120.0f;
	static constexpr float DefaultDynoTimeToTargetSeconds = 0.55f;
	static constexpr float DefaultDynoGravityZ = -980.0f;
	static constexpr float DefaultMaxDynoLaunchSpeed = 1800.0f;

	UFUNCTION(BlueprintPure, Category="Climbing|Solver")
	static FVector ProjectCenterOfMassToWallPlane(const FVector& CenterOfMass, const FVector& PlaneOrigin, const FVector& PlaneNormal);

	UFUNCTION(BlueprintPure, Category="Climbing|Solver")
	static FClimbingStabilityResult EstimateTwoPointStability(const FVector& CenterOfMass, const FVector& FirstContact, const FVector& SecondContact, const FVector& WallNormal, float StableOffsetCm = 75.0f);

	UFUNCTION(BlueprintPure, Category="Climbing|Solver")
	static float EstimateBodyTension(const FVector& CenterOfMass, const FVector& FirstContact, const FVector& SecondContact, float MaxBodyTensionOffsetCm = 120.0f);

	UFUNCTION(BlueprintPure, Category="Climbing|Solver")
	static FVector CalculateDynoLaunchVelocity(const FVector& StartLocation, const FVector& TargetLocation, float TimeToTargetSeconds = 0.55f, float GravityZ = -980.0f, float MaxLaunchSpeed = 1800.0f);
};
