#include "Animation/ClimbingAnimInstance.h"

#include "Character/ClimbingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UClimbingAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ResolveClimbingCharacter();
	SnapshotClimbingData();
}

void UClimbingAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	ResolveClimbingCharacter();
	SnapshotClimbingData();
}

AClimbingCharacter* UClimbingAnimInstance::GetClimbingCharacter() const
{
	return ClimbingCharacter;
}

FClimbingControlRigTargets UClimbingAnimInstance::GetControlRigTargets() const
{
	return ControlRigTargets;
}

void UClimbingAnimInstance::ResolveClimbingCharacter()
{
	if (ClimbingCharacter)
	{
		return;
	}

	ClimbingCharacter = Cast<AClimbingCharacter>(TryGetPawnOwner());
}

void UClimbingAnimInstance::SnapshotClimbingData()
{
	if (!ClimbingCharacter)
	{
		ResetClimbingData();
		return;
	}

	ClimbingState = ClimbingCharacter->GetClimbingState();
	bIsClimbing = ClimbingCharacter->IsClimbing();
	ActiveProbeLimb = ClimbingCharacter->GetActiveProbeLimb();
	CenterOfMassInput = ClimbingCharacter->GetClimbCenterOfMassInput();
	LimbProbeInput = ClimbingCharacter->GetClimbLimbProbeInput();
	const USkeletalMeshComponent* SkeletalMeshComponent = GetSkelMeshComponent();
	PelvisOffset = SkeletalMeshComponent
		? SkeletalMeshComponent->GetComponentTransform().InverseTransformVectorNoScale(ClimbingCharacter->GetClimbingDebugState().CenterOfMassTargetOffset)
		: ClimbingCharacter->GetClimbingDebugState().CenterOfMassTargetOffset;

	LeftHandTarget = MakeAnimTarget(ClimbingCharacter->GetLeftHandState(), SkeletalMeshComponent);
	RightHandTarget = MakeAnimTarget(ClimbingCharacter->GetRightHandState(), SkeletalMeshComponent);
	LeftFootTarget = MakeAnimTarget(ClimbingCharacter->GetLeftFootState(), SkeletalMeshComponent);
	RightFootTarget = MakeAnimTarget(ClimbingCharacter->GetRightFootState(), SkeletalMeshComponent);
	UpdateControlRigTargets();
}

FClimbingLimbAnimTarget UClimbingAnimInstance::MakeAnimTarget(const FLimbState& LimbState, const USkeletalMeshComponent* SkeletalMeshComponent)
{
	FClimbingLimbAnimTarget Target;
	Target.Limb = LimbState.Limb;
	Target.bHasTarget = LimbState.bHasValidContact;
	Target.bIsLocked = LimbState.bIsLocked;
	Target.LoadPercent = LimbState.LoadPercent;

	if (!SkeletalMeshComponent)
	{
		Target.TargetLocation = LimbState.ContactLocation;
		Target.TargetNormal = LimbState.ContactNormal;
		Target.TargetRotation = LimbState.ContactRotation;
		return Target;
	}

	const FTransform ComponentTransform = SkeletalMeshComponent->GetComponentTransform();
	Target.TargetLocation = ComponentTransform.InverseTransformPosition(LimbState.ContactLocation);
	Target.TargetNormal = ComponentTransform.InverseTransformVectorNoScale(LimbState.ContactNormal).GetSafeNormal();

	const FQuat ComponentRelativeRotation = ComponentTransform.GetRotation().Inverse() * LimbState.ContactRotation.Quaternion();
	Target.TargetRotation = ComponentRelativeRotation.Rotator();
	return Target;
}

void UClimbingAnimInstance::UpdateControlRigTargets()
{
	ControlRigTargets.bIsClimbing = bIsClimbing;
	ControlRigTargets.ActiveProbeLimb = ActiveProbeLimb;
	ControlRigTargets.PelvisOffset = PelvisOffset;
	ControlRigTargets.LeftHandTarget = LeftHandTarget;
	ControlRigTargets.RightHandTarget = RightHandTarget;
	ControlRigTargets.LeftFootTarget = LeftFootTarget;
	ControlRigTargets.RightFootTarget = RightFootTarget;
}

void UClimbingAnimInstance::ResetClimbingData()
{
	ClimbingState = EClimbingState::Falling;
	bIsClimbing = false;
	ActiveProbeLimb = EClimbingLimb::RightHand;
	CenterOfMassInput = FVector2D::ZeroVector;
	LimbProbeInput = FVector2D::ZeroVector;
	PelvisOffset = FVector::ZeroVector;

	LeftHandTarget = FClimbingLimbAnimTarget();
	LeftHandTarget.Limb = EClimbingLimb::LeftHand;

	RightHandTarget = FClimbingLimbAnimTarget();
	RightHandTarget.Limb = EClimbingLimb::RightHand;

	LeftFootTarget = FClimbingLimbAnimTarget();
	LeftFootTarget.Limb = EClimbingLimb::LeftFoot;

	RightFootTarget = FClimbingLimbAnimTarget();
	RightFootTarget.Limb = EClimbingLimb::RightFoot;

	UpdateControlRigTargets();
}
