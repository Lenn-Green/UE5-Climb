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
	const bool bHasActivePresentationTarget =
		LeftHandTarget.bHasTarget ||
		RightHandTarget.bHasTarget ||
		LeftFootTarget.bHasTarget ||
		RightFootTarget.bHasTarget ||
		LeftHandExplorationTarget.bHasTarget ||
		RightHandExplorationTarget.bHasTarget ||
		LeftFootExplorationTarget.bHasTarget ||
		RightFootExplorationTarget.bHasTarget ||
		bLeftHandReleaseBlendActive ||
		bRightHandReleaseBlendActive ||
		bLeftFootReleaseBlendActive ||
		bRightFootReleaseBlendActive;
	const bool bRefreshNeutralPose = !bIsClimbing && !bHasActivePresentationTarget;
	CacheNeutralPoseTargets(SkeletalMeshComponent, bRefreshNeutralPose);
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
	bool bDummyReleaseBlendActive = false;
	FClimbingLimbAnimTarget DummyReleaseStartTarget;
	LeftHandTarget = SmoothLimbTarget(LeftHandTarget, MakeAnimTarget(ClimbingCharacter->GetLeftHandState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, true, bLeftHandReleasing, LeftHandReleaseStartTarget, bLeftHandReleaseBlendActive);
	RightHandTarget = SmoothLimbTarget(RightHandTarget, MakeAnimTarget(ClimbingCharacter->GetRightHandState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, true, bRightHandReleasing, RightHandReleaseStartTarget, bRightHandReleaseBlendActive);
	LeftHandExplorationTarget = SmoothLimbTarget(LeftHandExplorationTarget, MakeExplorationTarget(EClimbingLimb::LeftHand, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, false, bIgnoredRelease, DummyReleaseStartTarget, bDummyReleaseBlendActive);
	RightHandExplorationTarget = SmoothLimbTarget(RightHandExplorationTarget, MakeExplorationTarget(EClimbingLimb::RightHand, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, false, bIgnoredRelease, DummyReleaseStartTarget, bDummyReleaseBlendActive);
	LeftFootTarget = SmoothLimbTarget(LeftFootTarget, MakeAnimTarget(ClimbingCharacter->GetLeftFootState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, true, bLeftFootReleasing, LeftFootReleaseStartTarget, bLeftFootReleaseBlendActive);
	RightFootTarget = SmoothLimbTarget(RightFootTarget, MakeAnimTarget(ClimbingCharacter->GetRightFootState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, true, bRightFootReleasing, RightFootReleaseStartTarget, bRightFootReleaseBlendActive);
	LeftFootExplorationTarget = SmoothLimbTarget(LeftFootExplorationTarget, MakeExplorationTarget(EClimbingLimb::LeftFoot, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, false, bIgnoredRelease, DummyReleaseStartTarget, bDummyReleaseBlendActive);
	RightFootExplorationTarget = SmoothLimbTarget(RightFootExplorationTarget, MakeExplorationTarget(EClimbingLimb::RightFoot, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, false, bIgnoredRelease, DummyReleaseStartTarget, bDummyReleaseBlendActive);
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
	bool bAllowReleaseBlend,
	bool& bOutIsReleasing,
	FClimbingLimbAnimTarget& ReleaseStartTarget,
	bool& bReleaseBlendActive) const
{
	bOutIsReleasing = false;
	FClimbingLimbAnimTarget SmoothedTarget = DesiredTarget;

	if (!DesiredTarget.bHasTarget)
	{
		if (!bAllowReleaseBlend)
		{
			bReleaseBlendActive = false;
			return DesiredTarget;
		}

		if (!ReleaseStartTarget.bHasTarget && !CurrentTarget.bHasTarget && !bReleaseBlendActive)
		{
			return SmoothedTarget;
		}

		const EClimbingLimb ReleaseLimb = ReleaseStartTarget.bHasTarget ? ReleaseStartTarget.Limb : CurrentTarget.Limb;
		const FClimbingLimbAnimTarget ReferenceTarget = GetNeutralLimbPoseTarget(ReleaseLimb);
		if (!ReferenceTarget.bHasTarget)
		{
			return SmoothedTarget;
		}
		const FVector ReferenceLocation = ReferenceTarget.TargetLocation;
		if (DeltaSeconds <= 0.0f || ReleaseTargetInterpSpeed <= 0.0f)
		{
			return SmoothedTarget;
		}

		// Release starts from the previous animation-facing target, not from the current socket.
		// Reading the socket after Control Rig has solved can feed the rigged pose back into the
		// bridge and create a one-frame jump when the effector weight changes.
		if (!bReleaseBlendActive)
		{
			if (!ReleaseStartTarget.bHasTarget)
			{
				ReleaseStartTarget = CurrentTarget;
			}
			if (!ReleaseStartTarget.bHasTarget)
			{
				return DesiredTarget;
			}
			ReleaseStartTarget.bIsLocked = false;
			bReleaseBlendActive = true;

			// Hold the exact bridge target for one frame. If we start interpolating immediately,
			// the first release frame can still show a visible pop before the blend-out begins.
			bOutIsReleasing = true;
			return ReleaseStartTarget;
		}

		const FClimbingLimbAnimTarget ReleaseSource = ReleaseStartTarget.bHasTarget ? ReleaseStartTarget : CurrentTarget;
		SmoothedTarget = ReleaseSource;
		SmoothedTarget.bHasTarget = true;
		SmoothedTarget.bIsLocked = false;
		bOutIsReleasing = true;
		SmoothedTarget.TargetLocation = FMath::VInterpTo(
			ReleaseSource.TargetLocation,
			ReferenceLocation,
			DeltaSeconds,
			ReleaseTargetInterpSpeed);
		SmoothedTarget.TargetRotation = FMath::RInterpTo(
			ReleaseSource.TargetRotation,
			ReferenceTarget.TargetRotation,
			DeltaSeconds,
			ReleaseTargetInterpSpeed);

		if (FVector::DistSquared(SmoothedTarget.TargetLocation, ReferenceLocation) <= FMath::Square(ReleaseTargetCompletionDistance))
		{
			ReleaseStartTarget = FClimbingLimbAnimTarget();
			ReleaseStartTarget.Limb = ReleaseLimb;
			bReleaseBlendActive = false;
			return DesiredTarget;
		}

		ReleaseStartTarget = SmoothedTarget;
		return SmoothedTarget;
	}

	bReleaseBlendActive = false;

	const float InterpSpeed = DesiredTarget.bIsLocked ? LockedTargetInterpSpeed : ExplorationTargetInterpSpeed;
	if (DeltaSeconds <= 0.0f || InterpSpeed <= 0.0f)
	{
		SmoothedTarget = ApplyPresentationConstraints(SmoothedTarget, SkeletalMeshComponent);
		if (DesiredTarget.bIsLocked && SmoothedTarget.bHasTarget)
		{
			ReleaseStartTarget = SmoothedTarget;
		}
		return SmoothedTarget;
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
	SmoothedTarget = ApplyPresentationConstraints(SmoothedTarget, SkeletalMeshComponent);
	if (DesiredTarget.bIsLocked && SmoothedTarget.bHasTarget)
	{
		ReleaseStartTarget = SmoothedTarget;
	}
	return SmoothedTarget;
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

FClimbingLimbAnimTarget UClimbingAnimInstance::GetCurrentLimbPoseTarget(EClimbingLimb Limb, const USkeletalMeshComponent* SkeletalMeshComponent) const
{
	FClimbingLimbAnimTarget Target;
	Target.Limb = Limb;
	Target.bHasTarget = SkeletalMeshComponent != nullptr;
	Target.bIsLocked = false;

	if (!SkeletalMeshComponent)
	{
		return Target;
	}

	const FName ReferenceName =
		(Limb == EClimbingLimb::LeftHand) ? FName(TEXT("hand_l")) :
		(Limb == EClimbingLimb::RightHand) ? FName(TEXT("hand_r")) :
		(Limb == EClimbingLimb::LeftFoot) ? FName(TEXT("foot_l")) :
		(Limb == EClimbingLimb::RightFoot) ? FName(TEXT("foot_r")) :
		NAME_None;

	if (ReferenceName == NAME_None || !SkeletalMeshComponent->DoesSocketExist(ReferenceName))
	{
		Target.bHasTarget = false;
		return Target;
	}

	const FTransform SocketTransform = SkeletalMeshComponent->GetSocketTransform(ReferenceName, RTS_Component);
	Target.TargetLocation = SocketTransform.GetLocation();
	Target.TargetRotation = SocketTransform.Rotator();
	Target.TargetNormal = SocketTransform.GetRotation().GetUpVector();
	return Target;
}

FClimbingLimbAnimTarget UClimbingAnimInstance::GetNeutralLimbPoseTarget(EClimbingLimb Limb) const
{
	switch (Limb)
	{
	case EClimbingLimb::LeftHand:
		return LeftHandNeutralTarget;
	case EClimbingLimb::RightHand:
		return RightHandNeutralTarget;
	case EClimbingLimb::LeftFoot:
		return LeftFootNeutralTarget;
	case EClimbingLimb::RightFoot:
	default:
		return RightFootNeutralTarget;
	}
}

void UClimbingAnimInstance::CacheNeutralPoseTargets(const USkeletalMeshComponent* SkeletalMeshComponent, bool bForceRefresh)
{
	if (!SkeletalMeshComponent)
	{
		return;
	}

	if (bForceRefresh || !LeftHandNeutralTarget.bHasTarget)
	{
		LeftHandNeutralTarget = GetCurrentLimbPoseTarget(EClimbingLimb::LeftHand, SkeletalMeshComponent);
	}

	if (bForceRefresh || !RightHandNeutralTarget.bHasTarget)
	{
		RightHandNeutralTarget = GetCurrentLimbPoseTarget(EClimbingLimb::RightHand, SkeletalMeshComponent);
	}

	if (bForceRefresh || !LeftFootNeutralTarget.bHasTarget)
	{
		LeftFootNeutralTarget = GetCurrentLimbPoseTarget(EClimbingLimb::LeftFoot, SkeletalMeshComponent);
	}

	if (bForceRefresh || !RightFootNeutralTarget.bHasTarget)
	{
		RightFootNeutralTarget = GetCurrentLimbPoseTarget(EClimbingLimb::RightFoot, SkeletalMeshComponent);
	}
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
	LeftHandNeutralTarget = FClimbingLimbAnimTarget();
	LeftHandNeutralTarget.Limb = EClimbingLimb::LeftHand;

	RightHandTarget = FClimbingLimbAnimTarget();
	RightHandTarget.Limb = EClimbingLimb::RightHand;
	RightHandNeutralTarget = FClimbingLimbAnimTarget();
	RightHandNeutralTarget.Limb = EClimbingLimb::RightHand;

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
	LeftHandReleaseStartTarget = FClimbingLimbAnimTarget();
	RightHandReleaseStartTarget = FClimbingLimbAnimTarget();
	bLeftHandReleaseBlendActive = false;
	bRightHandReleaseBlendActive = false;

	LeftFootTarget = FClimbingLimbAnimTarget();
	LeftFootTarget.Limb = EClimbingLimb::LeftFoot;
	LeftFootNeutralTarget = FClimbingLimbAnimTarget();
	LeftFootNeutralTarget.Limb = EClimbingLimb::LeftFoot;

	RightFootTarget = FClimbingLimbAnimTarget();
	RightFootTarget.Limb = EClimbingLimb::RightFoot;
	RightFootNeutralTarget = FClimbingLimbAnimTarget();
	RightFootNeutralTarget.Limb = EClimbingLimb::RightFoot;

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
	LeftFootReleaseStartTarget = FClimbingLimbAnimTarget();
	RightFootReleaseStartTarget = FClimbingLimbAnimTarget();
	bLeftFootReleaseBlendActive = false;
	bRightFootReleaseBlendActive = false;

	UpdateControlRigTargets();
}
