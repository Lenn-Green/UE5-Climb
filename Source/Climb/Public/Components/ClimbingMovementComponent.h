#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ClimbingMovementComponent.generated.h"

UCLASS(ClassGroup=(Movement), meta=(BlueprintSpawnableComponent))
class CLIMB_API UClimbingMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	static constexpr uint8 CustomMovementModeClimbing = 1;

	UClimbingMovementComponent();

	UFUNCTION(BlueprintCallable, Category="Climbing|Movement")
	void StartClimbingMovement();

	UFUNCTION(BlueprintCallable, Category="Climbing|Movement")
	void StopClimbingMovement();

	UFUNCTION(BlueprintPure, Category="Climbing|Movement")
	bool IsInClimbingMovementMode() const;

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

private:
	void PhysClimbing(float DeltaTime, int32 Iterations);
};
