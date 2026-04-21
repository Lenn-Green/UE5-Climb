#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/ClimbingHoldTypes.h"
#include "ClimbingHoldQueryComponent.generated.h"

UCLASS(ClassGroup=(Climbing), meta=(BlueprintSpawnableComponent))
class CLIMB_API UClimbingHoldQueryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UClimbingHoldQueryComponent();

	UFUNCTION(BlueprintCallable, Category="Climbing|Hold Query")
	bool QueryBestHoldFromViewpoint(FClimbingHoldCandidate& OutCandidate) const;

	UFUNCTION(BlueprintCallable, Category="Climbing|Hold Query")
	bool QueryBestHold(const FVector& Start, const FVector& Direction, FClimbingHoldCandidate& OutCandidate) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climbing|Hold Query", meta=(ClampMin="1.0", Units="cm"))
	float TraceDistance = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climbing|Hold Query", meta=(ClampMin="1.0", Units="cm"))
	float TraceRadius = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climbing|Hold Query")
	TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climbing|Hold Query")
	bool bTraceComplex = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climbing|Hold Query")
	bool bDebugDraw = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climbing|Hold Query", meta=(ClampMin="0.0"))
	float DebugDrawTime = 1.5f;

private:
	FClimbingHoldCandidate MakeCandidate(const FHitResult& Hit, const FVector& Start) const;
	bool IsExplicitHold(const FHitResult& Hit) const;
};
