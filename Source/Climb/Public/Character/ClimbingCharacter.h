#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/ClimbingTypes.h"
#include "ClimbingCharacter.generated.h"

struct FInputActionValue;
class UClimbingMovementComponent;
class UInputAction;
class UInputMappingContext;

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

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	FVector2D GetClimbCenterOfMassInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	FVector2D GetClimbLimbProbeInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	float GetLeftGripInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	float GetRightGripInput() const;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|State")
	EClimbingState ClimbingState = EClimbingState::Falling;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputMappingContext> ClimbingInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	int32 ClimbingInputMappingPriority = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbCenterOfMassMoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbLimbProbeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbLeftGripAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbRightGripAction;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	FVector2D ClimbCenterOfMassInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	FVector2D ClimbLimbProbeInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	float LeftGripInput = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	float RightGripInput = 0.0f;

private:
	void AddClimbingInputMappingContext() const;
	void BindClimbingInputActions(UInputComponent* PlayerInputComponent);
	void HandleClimbCenterOfMassMove(const FInputActionValue& Value);
	void HandleClimbCenterOfMassMoveCompleted(const FInputActionValue& Value);
	void HandleClimbLimbProbe(const FInputActionValue& Value);
	void HandleClimbLimbProbeCompleted(const FInputActionValue& Value);
	void HandleClimbLeftGrip(const FInputActionValue& Value);
	void HandleClimbLeftGripCompleted(const FInputActionValue& Value);
	void HandleClimbRightGrip(const FInputActionValue& Value);
	void HandleClimbRightGripCompleted(const FInputActionValue& Value);
	void SetClimbingState(EClimbingState NewState);
};
