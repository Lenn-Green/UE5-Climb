#include "Animation/ClimbingAnimInstance.h"

#include "Character/ClimbingCharacter.h"

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
	PelvisOffset = ClimbingCharacter->GetClimbingDebugState().CenterOfMassTargetOffset;

	LeftHandTarget = MakeAnimTarget(ClimbingCharacter->GetLeftHandState());
	RightHandTarget = MakeAnimTarget(ClimbingCharacter->GetRightHandState());
	LeftFootTarget = MakeAnimTarget(ClimbingCharacter->GetLeftFootState());
	RightFootTarget = MakeAnimTarget(ClimbingCharacter->GetRightFootState());
	UpdateControlRigTargets();
}

FClimbingLimbAnimTarget UClimbingAnimInstance::MakeAnimTarget(const FLimbState& LimbState)
{
	FClimbingLimbAnimTarget Target;
	Target.Limb = LimbState.Limb;
	Target.bHasTarget = LimbState.bHasValidContact;
	Target.bIsLocked = LimbState.bIsLocked;
	Target.TargetLocation = LimbState.ContactLocation;
	Target.TargetNormal = LimbState.ContactNormal;
	Target.TargetRotation = LimbState.ContactRotation;
	Target.LoadPercent = LimbState.LoadPercent;
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
