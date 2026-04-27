#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Data/ClimbingLimbTypes.h"
#include "Data/ClimbingTypes.h"
#include "ClimbingAnimInstance.generated.h"

class AClimbingCharacter;
class USkeletalMeshComponent;

USTRUCT(BlueprintType)
struct CLIMB_API FClimbingLimbAnimTarget
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimb Limb = EClimbingLimb::LeftHand;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bHasTarget = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bIsLocked = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FVector TargetLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FVector TargetNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FRotator TargetRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation", meta=(ClampMin="0.0", ClampMax="1.0"))
	float LoadPercent = 0.0f;
};

USTRUCT(BlueprintType)
struct CLIMB_API FClimbingControlRigTargets
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bIsClimbing = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimb ActiveProbeLimb = EClimbingLimb::RightHand;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FVector PelvisOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget LeftHandTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightHandTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget LeftFootTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightFootTarget;
};

UCLASS()
class CLIMB_API UClimbingAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category="Climbing|Animation")
	AClimbingCharacter* GetClimbingCharacter() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Animation")
	FClimbingControlRigTargets GetControlRigTargets() const;

protected:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	TObjectPtr<AClimbingCharacter> ClimbingCharacter;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingState ClimbingState = EClimbingState::Falling;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bIsClimbing = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimb ActiveProbeLimb = EClimbingLimb::RightHand;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FVector2D CenterOfMassInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FVector2D LimbProbeInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FVector PelvisOffset = FVector::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget LeftHandTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightHandTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget LeftFootTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightFootTarget;

	// Suggested Control Rig variable contract:
	// PelvisOffset, LeftHandTarget, RightHandTarget, LeftFootTarget, RightFootTarget, ActiveProbeLimb.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingControlRigTargets ControlRigTargets;

private:
	void ResolveClimbingCharacter();
	void SnapshotClimbingData();
	static FClimbingLimbAnimTarget MakeAnimTarget(const FLimbState& LimbState, const USkeletalMeshComponent* SkeletalMeshComponent);
	void UpdateControlRigTargets();
	void ResetClimbingData();
};
