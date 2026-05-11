#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Data/ClimbingLimbTypes.h"
#include "Data/ClimbingTypes.h"
#include "ClimbingAnimInstance.generated.h"

class AClimbingCharacter;
class USkeletalMeshComponent;

UENUM(BlueprintType)
enum class EClimbingLimbPresentationState : uint8
{
	Inactive UMETA(DisplayName="Inactive"),
	Exploration UMETA(DisplayName="Exploration"),
	Locked UMETA(DisplayName="Locked"),
	Releasing UMETA(DisplayName="Releasing")
};

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
	FClimbingLimbAnimTarget LeftHandExplorationTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightHandExplorationTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimbPresentationState LeftHandPresentationState = EClimbingLimbPresentationState::Inactive;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimbPresentationState RightHandPresentationState = EClimbingLimbPresentationState::Inactive;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftHandIsExploration = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftHandIsLocked = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftHandIsReleasing = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightHandIsExploration = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightHandIsLocked = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightHandIsReleasing = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget LeftFootTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightFootTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget LeftFootExplorationTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightFootExplorationTarget;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimbPresentationState LeftFootPresentationState = EClimbingLimbPresentationState::Inactive;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimbPresentationState RightFootPresentationState = EClimbingLimbPresentationState::Inactive;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftFootIsExploration = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftFootIsLocked = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftFootIsReleasing = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightFootIsExploration = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightFootIsLocked = false;

	UPROPERTY(BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightFootIsReleasing = false;
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
	FClimbingLimbAnimTarget LeftHandExplorationTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightHandExplorationTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimbPresentationState LeftHandPresentationState = EClimbingLimbPresentationState::Inactive;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimbPresentationState RightHandPresentationState = EClimbingLimbPresentationState::Inactive;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftHandIsExploration = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftHandIsLocked = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftHandIsReleasing = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightHandIsExploration = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightHandIsLocked = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightHandIsReleasing = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget LeftFootTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightFootTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget LeftFootExplorationTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingLimbAnimTarget RightFootExplorationTarget;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimbPresentationState LeftFootPresentationState = EClimbingLimbPresentationState::Inactive;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	EClimbingLimbPresentationState RightFootPresentationState = EClimbingLimbPresentationState::Inactive;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftFootIsExploration = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftFootIsLocked = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bLeftFootIsReleasing = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightFootIsExploration = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightFootIsLocked = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	bool bRightFootIsReleasing = false;

	// Suggested Control Rig variable contract:
	// PelvisOffset, LeftHandTarget, RightHandTarget, LeftHandExplorationTarget,
	// RightHandExplorationTarget, LeftFootTarget, RightFootTarget, LeftFootExplorationTarget,
	// RightFootExplorationTarget, ActiveProbeLimb.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Animation")
	FClimbingControlRigTargets ControlRigTargets;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Smoothing", meta=(ClampMin="0.0"))
	float PelvisOffsetInterpSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Smoothing", meta=(ClampMin="0.0"))
	float LockedTargetInterpSpeed = 16.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Smoothing", meta=(ClampMin="0.0"))
	float ExplorationTargetInterpSpeed = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Smoothing", meta=(ClampMin="0.0"))
	float TargetRotationInterpSpeed = 14.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Smoothing", meta=(ClampMin="0.0"))
	float ReleaseTargetInterpSpeed = 14.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Smoothing", meta=(ClampMin="0.0", Units="cm"))
	float ReleaseTargetCompletionDistance = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Pose", meta=(ClampMin="0.0", Units="cm"))
	float HandLockedSurfaceClearance = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Pose", meta=(ClampMin="0.0", Units="cm"))
	float HandExplorationSurfaceClearance = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Pose", meta=(ClampMin="0.0", Units="cm"))
	float FootLockedSurfaceClearance = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Pose", meta=(ClampMin="0.0", Units="cm"))
	float FootExplorationSurfaceClearance = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Pose", meta=(ClampMin="1.0", Units="cm"))
	float HandLockedMaxReach = 95.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Pose", meta=(ClampMin="1.0", Units="cm"))
	float HandExplorationMaxReach = 82.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Pose", meta=(ClampMin="1.0", Units="cm"))
	float FootLockedMaxReach = 105.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Pose", meta=(ClampMin="1.0", Units="cm"))
	float FootExplorationMaxReach = 88.0f;

private:
	void ResolveClimbingCharacter();
	void SnapshotClimbingData(float DeltaSeconds);
	static FClimbingLimbAnimTarget MakeAnimTarget(const FLimbState& LimbState, const USkeletalMeshComponent* SkeletalMeshComponent);
	FClimbingLimbAnimTarget MakeExplorationTarget(EClimbingLimb Limb, const USkeletalMeshComponent* SkeletalMeshComponent) const;
	static EClimbingLimbPresentationState DeterminePresentationState(
		const FClimbingLimbAnimTarget& LockedTarget,
		const FClimbingLimbAnimTarget& ExplorationTarget,
		bool bIsReleasing);
	FClimbingLimbAnimTarget SmoothLimbTarget(
		const FClimbingLimbAnimTarget& CurrentTarget,
		const FClimbingLimbAnimTarget& DesiredTarget,
		float DeltaSeconds,
		const USkeletalMeshComponent* SkeletalMeshComponent,
		bool& bOutIsReleasing) const;
	FClimbingLimbAnimTarget ApplyPresentationConstraints(
		const FClimbingLimbAnimTarget& Target,
		const USkeletalMeshComponent* SkeletalMeshComponent) const;
	FVector GetLimbReferenceLocation(EClimbingLimb Limb, const USkeletalMeshComponent* SkeletalMeshComponent) const;
	float GetSurfaceClearance(EClimbingLimb Limb, bool bIsLocked) const;
	float GetMaxReach(EClimbingLimb Limb, bool bIsLocked) const;
	void UpdateControlRigTargets();
	void ResetClimbingData();
};
