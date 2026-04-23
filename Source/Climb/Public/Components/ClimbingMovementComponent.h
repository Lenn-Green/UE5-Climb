#pragma once

#include "CoreMinimal.h"
#include "Data/ClimbingMovementTypes.h"
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

	void StartClimbingMovement(const FClimbingAttachmentFrame& AttachmentFrame);

	UFUNCTION(BlueprintCallable, Category="Climbing|Movement")
	void StartClimbingMovementWithAttachment(const FClimbingAttachmentFrame& AttachmentFrame);

	UFUNCTION(BlueprintCallable, Category="Climbing|Movement")
	void StopClimbingMovement();

	UFUNCTION(BlueprintCallable, Category="Climbing|Movement")
	void UpdateClimbingAttachmentFrame(const FClimbingAttachmentFrame& AttachmentFrame);

	UFUNCTION(BlueprintPure, Category="Climbing|Movement")
	bool IsInClimbingMovementMode() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Movement")
	FClimbingAttachmentFrame GetClimbingAttachmentFrame() const;

protected:
	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climbing|Movement", meta=(ClampMin="0.0", Units="cm"))
	float TargetWallDistance = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Climbing|Movement", meta=(ClampMin="0.0"))
	float AttachInterpSpeed = 12.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Movement")
	FClimbingAttachmentFrame ClimbingAttachmentFrame;

private:
	void PhysClimbing(float DeltaTime, int32 Iterations);
	FVector GetClimbingTargetLocation() const;
};
