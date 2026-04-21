#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/ClimbingTypes.h"
#include "ClimbingCharacter.generated.h"

class UClimbingMovementComponent;

UCLASS()
class CLIMB_API AClimbingCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AClimbingCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Climbing|State")
	void EnterClimbing();

	UFUNCTION(BlueprintCallable, Category="Climbing|State")
	void ExitClimbing();

	UFUNCTION(BlueprintCallable, Category="Climbing|State")
	void PrepareClimbingJump();

	UFUNCTION(BlueprintPure, Category="Climbing|State")
	EClimbingState GetClimbingState() const;

	UFUNCTION(BlueprintPure, Category="Climbing|State")
	bool IsClimbing() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Movement")
	UClimbingMovementComponent* GetClimbingMovementComponent() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|State")
	EClimbingState ClimbingState = EClimbingState::Falling;

private:
	void SetClimbingState(EClimbingState NewState);
};
