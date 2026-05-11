#include "Animation/ClimbingAnimInstance.h"

#include "Character/ClimbingCharacter.h"
#include "Components/SkeletalMeshComponent.h"

void UClimbingAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	ResolveClimbingCharacter();
	SnapshotClimbingData(0.0f);
}

void UClimbingAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	ResolveClimbingCharacter();
	SnapshotClimbingData(DeltaSeconds);
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

void UClimbingAnimInstance::SnapshotClimbingData(float DeltaSeconds)
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
	const FVector DesiredPelvisOffset = SkeletalMeshComponent
		? SkeletalMeshComponent->GetComponentTransform().InverseTransformVectorNoScale(ClimbingCharacter->GetClimbingDebugState().CenterOfMassTargetOffset)
		: ClimbingCharacter->GetClimbingDebugState().CenterOfMassTargetOffset;
	PelvisOffset = (DeltaSeconds <= 0.0f || PelvisOffsetInterpSpeed <= 0.0f)
		? DesiredPelvisOffset
		: FMath::VInterpTo(PelvisOffset, DesiredPelvisOffset, DeltaSeconds, PelvisOffsetInterpSpeed);

	bool bLeftHandReleasing = false;
	bool bRightHandReleasing = false;
	bool bLeftFootReleasing = false;
	bool bRightFootReleasing = false;
	bool bIgnoredRelease = false;
	LeftHandTarget = SmoothLimbTarget(LeftHandTarget, MakeAnimTarget(ClimbingCharacter->GetLeftHandState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, bLeftHandReleasing);
	RightHandTarget = SmoothLimbTarget(RightHandTarget, MakeAnimTarget(ClimbingCharacter->GetRightHandState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, bRightHandReleasing);
	LeftHandExplorationTarget = SmoothLimbTarget(LeftHandExplorationTarget, MakeExplorationTarget(EClimbingLimb::LeftHand, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, bIgnoredRelease);
	RightHandExplorationTarget = SmoothLimbTarget(RightHandExplorationTarget, MakeExplorationTarget(EClimbingLimb::RightHand, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, bIgnoredRelease);
	LeftFootTarget = SmoothLimbTarget(LeftFootTarget, MakeAnimTarget(ClimbingCharacter->GetLeftFootState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, bLeftFootReleasing);
	RightFootTarget = SmoothLimbTarget(RightFootTarget, MakeAnimTarget(ClimbingCharacter->GetRightFootState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, bRightFootReleasing);
	LeftFootExplorationTarget = SmoothLimbTarget(LeftFootExplorationTarget, MakeExplorationTarget(EClimbingLimb::LeftFoot, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, bIgnoredRelease);
	RightFootExplorationTarget = SmoothLimbTarget(RightFootExplorationTarget, MakeExplorationTarget(EClimbingLimb::RightFoot, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, bIgnoredRelease);
	LeftHandPresentationState = DeterminePresentationState(LeftHandTarget, LeftHandExplorationTarget, bLeftHandReleasing);
	RightHandPresentationState = DeterminePresentationState(RightHandTarget, RightHandExplorationTarget, bRightHandReleasing);
	LeftFootPresentationState = DeterminePresentationState(LeftFootTarget, LeftFootExplorationTarget, bLeftFootReleasing);
	RightFootPresentationState = DeterminePresentationState(RightFootTarget, RightFootExplorationTarget, bRightFootReleasing);

	bLeftHandIsExploration = LeftHandPresentationState == EClimbingLimbPresentationState::Exploration;
	bLeftHandIsLocked = LeftHandPresentationState == EClimbingLimbPresentationState::Locked;
	bLeftHandIsReleasing = LeftHandPresentationState == EClimbingLimbPresentationState::Releasing;

	bRightHandIsExploration = RightHandPresentationState == EClimbingLimbPresentationState::Exploration;
	bRightHandIsLocked = RightHandPresentationState == EClimbingLimbPresentationState::Locked;
	bRightHandIsReleasing = RightHandPresentationState == EClimbingLimbPresentationState::Releasing;

	bLeftFootIsExploration = LeftFootPresentationState == EClimbingLimbPresentationState::Exploration;
	bLeftFootIsLocked = LeftFootPresentationState == EClimbingLimbPresentationState::Locked;
	bLeftFootIsReleasing = LeftFootPresentationState == EClimbingLimbPresentationState::Releasing;

	bRightFootIsExploration = RightFootPresentationState == EClimbingLimbPresentationState::Exploration;
	bRightFootIsLocked = RightFootPresentationState == EClimbingLimbPresentationState::Locked;
	bRightFootIsReleasing = RightFootPresentationState == EClimbingLimbPresentationState::Releasing;
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

FClimbingLimbAnimTarget UClimbingAnimInstance::MakeExplorationTarget(EClimbingLimb Limb, const USkeletalMeshComponent* SkeletalMeshComponent) const
{
	FClimbingLimbAnimTarget Target;
	Target.Limb = Limb;

	if (!ClimbingCharacter)
	{
		return Target;
	}

	const FClimbingDebugState DebugState = ClimbingCharacter->GetClimbingDebugState();
	if (!DebugState.bHasActiveExplorationTarget || ActiveProbeLimb != Limb)
	{
		return Target;
	}

	const FLimbState LimbState =
		(Limb == EClimbingLimb::LeftHand) ? ClimbingCharacter->GetLeftHandState() :
		(Limb == EClimbingLimb::RightHand) ? ClimbingCharacter->GetRightHandState() :
		(Limb == EClimbingLimb::LeftFoot) ? ClimbingCharacter->GetLeftFootState() :
		(Limb == EClimbingLimb::RightFoot) ? ClimbingCharacter->GetRightFootState() :
		FLimbState();
	if (LimbState.bIsLocked)
	{
		return Target;
	}

	Target.bHasTarget = true;
	Target.bIsLocked = false;

	if (!SkeletalMeshComponent)
	{
		Target.TargetLocation = DebugState.ActiveExplorationTargetLocation;
		Target.TargetNormal = DebugState.ActiveExplorationTargetNormal;
		Target.TargetRotation = DebugState.ActiveExplorationTargetNormal.ToOrientationRotator();
		return Target;
	}

	const FTransform ComponentTransform = SkeletalMeshComponent->GetComponentTransform();
	Target.TargetLocation = ComponentTransform.InverseTransformPosition(DebugState.ActiveExplorationTargetLocation);
	Target.TargetNormal = ComponentTransform.InverseTransformVectorNoScale(DebugState.ActiveExplorationTargetNormal).GetSafeNormal();
	Target.TargetRotation = Target.TargetNormal.ToOrientationRotator();
	return Target;
}

EClimbingLimbPresentationState UClimbingAnimInstance::DeterminePresentationState(
	const FClimbingLimbAnimTarget& LockedTarget,
	const FClimbingLimbAnimTarget& ExplorationTarget,
	bool bIsReleasing)
{
	if (bIsReleasing)
	{
		return EClimbingLimbPresentationState::Releasing;
	}

	if (LockedTarget.bIsLocked && LockedTarget.bHasTarget)
	{
		return EClimbingLimbPresentationState::Locked;
	}

	if (ExplorationTarget.bHasTarget)
	{
		return EClimbingLimbPresentationState::Exploration;
	}

	return EClimbingLimbPresentationState::Inactive;
}

FClimbingLimbAnimTarget UClimbingAnimInstance::SmoothLimbTarget(
	const FClimbingLimbAnimTarget& CurrentTarget,
	const FClimbingLimbAnimTarget& DesiredTarget,
	float DeltaSeconds,
	const USkeletalMeshComponent* SkeletalMeshComponent,
	bool& bOutIsReleasing) const
{
	bOutIsReleasing = false;
	FClimbingLimbAnimTarget SmoothedTarget = DesiredTarget;

	if (!DesiredTarget.bHasTarget)
	{
		if (!CurrentTarget.bHasTarget)
		{
			return SmoothedTarget;
		}

		const FVector ReferenceLocation = GetLimbReferenceLocation(CurrentTarget.Limb, SkeletalMeshComponent);
		if (DeltaSeconds <= 0.0f || ReleaseTargetInterpSpeed <= 0.0f)
		{
			return SmoothedTarget;
		}

		// When gameplay clears a limb immediately on release, keep a short presentation-side blend-out
		// so the effector can travel back toward its reference pose instead of snapping off the hold.
		SmoothedTarget = CurrentTarget;
		bOutIsReleasing = true;
		SmoothedTarget.TargetLocation = FMath::VInterpTo(
			CurrentTarget.TargetLocation,
			ReferenceLocation,
			DeltaSeconds,
			ReleaseTargetInterpSpeed);

		if (FVector::DistSquared(SmoothedTarget.TargetLocation, ReferenceLocation) <= FMath::Square(ReleaseTargetCompletionDistance))
		{
			return DesiredTarget;
		}

		return SmoothedTarget;
	}

	const float InterpSpeed = DesiredTarget.bIsLocked ? LockedTargetInterpSpeed : ExplorationTargetInterpSpeed;
	if (DeltaSeconds <= 0.0f || InterpSpeed <= 0.0f)
	{
		return ApplyPresentationConstraints(SmoothedTarget, SkeletalMeshComponent);
	}

	FVector CurrentLocation = CurrentTarget.TargetLocation;
	FVector CurrentNormal = CurrentTarget.TargetNormal;
	FRotator CurrentRotation = CurrentTarget.TargetRotation;

	if (!CurrentTarget.bHasTarget)
	{
		CurrentLocation = GetLimbReferenceLocation(DesiredTarget.Limb, SkeletalMeshComponent);
		CurrentNormal = DesiredTarget.TargetNormal;
		CurrentRotation = DesiredTarget.TargetRotation;
	}

	SmoothedTarget.TargetLocation = FMath::VInterpTo(CurrentLocation, DesiredTarget.TargetLocation, DeltaSeconds, InterpSpeed);
	SmoothedTarget.TargetNormal = FMath::VInterpTo(CurrentNormal, DesiredTarget.TargetNormal, DeltaSeconds, TargetRotationInterpSpeed).GetSafeNormal();
	if (SmoothedTarget.TargetNormal.IsNearlyZero())
	{
		SmoothedTarget.TargetNormal = DesiredTarget.TargetNormal.GetSafeNormal();
	}

	SmoothedTarget.TargetRotation = FMath::RInterpTo(CurrentRotation, DesiredTarget.TargetRotation, DeltaSeconds, TargetRotationInterpSpeed);
	return ApplyPresentationConstraints(SmoothedTarget, SkeletalMeshComponent);
}

FClimbingLimbAnimTarget UClimbingAnimInstance::ApplyPresentationConstraints(
	const FClimbingLimbAnimTarget& Target,
	const USkeletalMeshComponent* SkeletalMeshComponent) const
{
	FClimbingLimbAnimTarget ConstrainedTarget = Target;
	if (!ConstrainedTarget.bHasTarget)
	{
		return ConstrainedTarget;
	}

	const FVector ReferenceLocation = GetLimbReferenceLocation(ConstrainedTarget.Limb, SkeletalMeshComponent);
	const float SurfaceClearance = GetSurfaceClearance(ConstrainedTarget.Limb, ConstrainedTarget.bIsLocked);
	const float MaxReach = GetMaxReach(ConstrainedTarget.Limb, ConstrainedTarget.bIsLocked);

	// Locked contacts need a small presentation offset so the effector reads as resting on the
	// surface instead of trying to bury itself into the literal hit point. Exploration targets,
	// by contrast, should track the plane target directly so the visible debug target and the limb
	// presentation stay aligned while the player steers the limb.
	if (ConstrainedTarget.bIsLocked)
	{
		const FVector TargetNormal = ConstrainedTarget.TargetNormal.GetSafeNormal();
		if (!TargetNormal.IsNearlyZero())
		{
			ConstrainedTarget.TargetLocation += TargetNormal * SurfaceClearance;
			ConstrainedTarget.TargetRotation = TargetNormal.ToOrientationRotator();
		}

		// Clamp locked target distance from the current limb reference so large gameplay jumps do not
		// force the FBIK chain into impossible reaches or severe twist poses in a single state.
		const FVector ReferenceToTarget = ConstrainedTarget.TargetLocation - ReferenceLocation;
		if (!ReferenceToTarget.IsNearlyZero() && ReferenceToTarget.SizeSquared() > FMath::Square(MaxReach))
		{
			ConstrainedTarget.TargetLocation = ReferenceLocation + ReferenceToTarget.GetClampedToMaxSize(MaxReach);
		}
	}

	return ConstrainedTarget;
}

FVector UClimbingAnimInstance::GetLimbReferenceLocation(EClimbingLimb Limb, const USkeletalMeshComponent* SkeletalMeshComponent) const
{
	if (!SkeletalMeshComponent)
	{
		return FVector::ZeroVector;
	}

	const FName ReferenceName =
		(Limb == EClimbingLimb::LeftHand) ? FName(TEXT("hand_l")) :
		(Limb == EClimbingLimb::RightHand) ? FName(TEXT("hand_r")) :
		(Limb == EClimbingLimb::LeftFoot) ? FName(TEXT("foot_l")) :
		(Limb == EClimbingLimb::RightFoot) ? FName(TEXT("foot_r")) :
		NAME_None;

	if (ReferenceName != NAME_None && SkeletalMeshComponent->DoesSocketExist(ReferenceName))
	{
		return SkeletalMeshComponent->GetSocketTransform(ReferenceName, RTS_Component).GetLocation();
	}

	return FVector::ZeroVector;
}

float UClimbingAnimInstance::GetSurfaceClearance(EClimbingLimb Limb, bool bIsLocked) const
{
	switch (Limb)
	{
	case EClimbingLimb::LeftHand:
	case EClimbingLimb::RightHand:
		return bIsLocked ? HandLockedSurfaceClearance : HandExplorationSurfaceClearance;
	case EClimbingLimb::LeftFoot:
	case EClimbingLimb::RightFoot:
	default:
		return bIsLocked ? FootLockedSurfaceClearance : FootExplorationSurfaceClearance;
	}
}

float UClimbingAnimInstance::GetMaxReach(EClimbingLimb Limb, bool bIsLocked) const
{
	switch (Limb)
	{
	case EClimbingLimb::LeftHand:
	case EClimbingLimb::RightHand:
		return bIsLocked ? HandLockedMaxReach : HandExplorationMaxReach;
	case EClimbingLimb::LeftFoot:
	case EClimbingLimb::RightFoot:
	default:
		return bIsLocked ? FootLockedMaxReach : FootExplorationMaxReach;
	}
}

void UClimbingAnimInstance::UpdateControlRigTargets()
{
	ControlRigTargets.bIsClimbing = bIsClimbing;
	ControlRigTargets.ActiveProbeLimb = ActiveProbeLimb;
	ControlRigTargets.PelvisOffset = PelvisOffset;
	ControlRigTargets.LeftHandTarget = LeftHandTarget;
	ControlRigTargets.RightHandTarget = RightHandTarget;
	ControlRigTargets.LeftHandExplorationTarget = LeftHandExplorationTarget;
	ControlRigTargets.RightHandExplorationTarget = RightHandExplorationTarget;
	ControlRigTargets.LeftHandPresentationState = LeftHandPresentationState;
	ControlRigTargets.RightHandPresentationState = RightHandPresentationState;
	ControlRigTargets.bLeftHandIsExploration = bLeftHandIsExploration;
	ControlRigTargets.bLeftHandIsLocked = bLeftHandIsLocked;
	ControlRigTargets.bLeftHandIsReleasing = bLeftHandIsReleasing;
	ControlRigTargets.bRightHandIsExploration = bRightHandIsExploration;
	ControlRigTargets.bRightHandIsLocked = bRightHandIsLocked;
	ControlRigTargets.bRightHandIsReleasing = bRightHandIsReleasing;
	ControlRigTargets.LeftFootTarget = LeftFootTarget;
	ControlRigTargets.RightFootTarget = RightFootTarget;
	ControlRigTargets.LeftFootExplorationTarget = LeftFootExplorationTarget;
	ControlRigTargets.RightFootExplorationTarget = RightFootExplorationTarget;
	ControlRigTargets.LeftFootPresentationState = LeftFootPresentationState;
	ControlRigTargets.RightFootPresentationState = RightFootPresentationState;
	ControlRigTargets.bLeftFootIsExploration = bLeftFootIsExploration;
	ControlRigTargets.bLeftFootIsLocked = bLeftFootIsLocked;
	ControlRigTargets.bLeftFootIsReleasing = bLeftFootIsReleasing;
	ControlRigTargets.bRightFootIsExploration = bRightFootIsExploration;
	ControlRigTargets.bRightFootIsLocked = bRightFootIsLocked;
	ControlRigTargets.bRightFootIsReleasing = bRightFootIsReleasing;
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

	LeftHandExplorationTarget = FClimbingLimbAnimTarget();
	LeftHandExplorationTarget.Limb = EClimbingLimb::LeftHand;

	RightHandExplorationTarget = FClimbingLimbAnimTarget();
	RightHandExplorationTarget.Limb = EClimbingLimb::RightHand;

	LeftHandPresentationState = EClimbingLimbPresentationState::Inactive;
	RightHandPresentationState = EClimbingLimbPresentationState::Inactive;
	bLeftHandIsExploration = false;
	bLeftHandIsLocked = false;
	bLeftHandIsReleasing = false;
	bRightHandIsExploration = false;
	bRightHandIsLocked = false;
	bRightHandIsReleasing = false;

	LeftFootTarget = FClimbingLimbAnimTarget();
	LeftFootTarget.Limb = EClimbingLimb::LeftFoot;

	RightFootTarget = FClimbingLimbAnimTarget();
	RightFootTarget.Limb = EClimbingLimb::RightFoot;

	LeftFootExplorationTarget = FClimbingLimbAnimTarget();
	LeftFootExplorationTarget.Limb = EClimbingLimb::LeftFoot;

	RightFootExplorationTarget = FClimbingLimbAnimTarget();
	RightFootExplorationTarget.Limb = EClimbingLimb::RightFoot;

	LeftFootPresentationState = EClimbingLimbPresentationState::Inactive;
	RightFootPresentationState = EClimbingLimbPresentationState::Inactive;
	bLeftFootIsExploration = false;
	bLeftFootIsLocked = false;
	bLeftFootIsReleasing = false;
	bRightFootIsExploration = false;
	bRightFootIsLocked = false;
	bRightFootIsReleasing = false;

	UpdateControlRigTargets();
}
