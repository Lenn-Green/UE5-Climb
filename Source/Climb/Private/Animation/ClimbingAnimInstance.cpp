#include "Animation/ClimbingAnimInstance.h"

#include "Character/ClimbingCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
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
	CacheNeutralPoseTargets(SkeletalMeshComponent);
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
	float DummyReleaseBlendElapsed = 0.0f;
	FClimbingLimbAnimTarget DummyReleaseStartTarget;
	LeftHandTarget = SmoothLimbTarget(LeftHandTarget, MakeAnimTarget(ClimbingCharacter->GetLeftHandState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, true, bLeftHandReleasing, LeftHandReleaseStartTarget, LeftHandReleaseBlendElapsed, bLeftHandReleaseBlendActive);
	RightHandTarget = SmoothLimbTarget(RightHandTarget, MakeAnimTarget(ClimbingCharacter->GetRightHandState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, true, bRightHandReleasing, RightHandReleaseStartTarget, RightHandReleaseBlendElapsed, bRightHandReleaseBlendActive);
	LeftHandExplorationTarget = SmoothLimbTarget(LeftHandExplorationTarget, MakeExplorationTarget(EClimbingLimb::LeftHand, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, false, bIgnoredRelease, DummyReleaseStartTarget, DummyReleaseBlendElapsed, bDummyReleaseBlendActive);
	RightHandExplorationTarget = SmoothLimbTarget(RightHandExplorationTarget, MakeExplorationTarget(EClimbingLimb::RightHand, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, false, bIgnoredRelease, DummyReleaseStartTarget, DummyReleaseBlendElapsed, bDummyReleaseBlendActive);
	LeftFootTarget = SmoothLimbTarget(LeftFootTarget, MakeAnimTarget(ClimbingCharacter->GetLeftFootState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, true, bLeftFootReleasing, LeftFootReleaseStartTarget, LeftFootReleaseBlendElapsed, bLeftFootReleaseBlendActive);
	RightFootTarget = SmoothLimbTarget(RightFootTarget, MakeAnimTarget(ClimbingCharacter->GetRightFootState(), SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, true, bRightFootReleasing, RightFootReleaseStartTarget, RightFootReleaseBlendElapsed, bRightFootReleaseBlendActive);
	LeftFootExplorationTarget = SmoothLimbTarget(LeftFootExplorationTarget, MakeExplorationTarget(EClimbingLimb::LeftFoot, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, false, bIgnoredRelease, DummyReleaseStartTarget, DummyReleaseBlendElapsed, bDummyReleaseBlendActive);
	RightFootExplorationTarget = SmoothLimbTarget(RightFootExplorationTarget, MakeExplorationTarget(EClimbingLimb::RightFoot, SkeletalMeshComponent), DeltaSeconds, SkeletalMeshComponent, false, bIgnoredRelease, DummyReleaseStartTarget, DummyReleaseBlendElapsed, bDummyReleaseBlendActive);
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

FClimbingLimbAnimTarget UClimbingAnimInstance::MakeAnimTarget(const FLimbState& LimbState, const USkeletalMeshComponent* SkeletalMeshComponent) const
{
	FClimbingLimbAnimTarget Target;
	Target.Limb = LimbState.Limb;
	Target.bHasTarget = LimbState.bHasValidContact;
	Target.bIsLocked = LimbState.bIsLocked;
	Target.LoadPercent = LimbState.LoadPercent;

	if (!LimbState.bHasValidContact)
	{
		Target = GetNeutralLimbPoseTarget(LimbState.Limb);
		Target.Limb = LimbState.Limb;
		Target.bHasTarget = false;
		Target.bIsLocked = false;
		Target.LoadPercent = 0.0f;
		return Target;
	}

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
	float& ReleaseBlendElapsed,
	bool& bReleaseBlendActive) const
{
	bOutIsReleasing = false;
	FClimbingLimbAnimTarget SmoothedTarget = DesiredTarget;

	if (!DesiredTarget.bHasTarget)
	{
		if (!bAllowReleaseBlend)
		{
			ReleaseBlendElapsed = 0.0f;
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
		if (DeltaSeconds <= 0.0f || ReleaseTargetBlendDuration <= 0.0f)
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
			ReleaseBlendElapsed = 0.0f;
			bReleaseBlendActive = true;

			// Hold the exact bridge target for one frame. If we start interpolating immediately,
			// the first release frame can still show a visible pop before the blend-out begins.
			bOutIsReleasing = true;
			return ReleaseStartTarget;
		}

		ReleaseBlendElapsed = FMath::Min(ReleaseBlendElapsed + DeltaSeconds, ReleaseTargetBlendDuration);
		const float LinearAlpha = FMath::Clamp(ReleaseBlendElapsed / ReleaseTargetBlendDuration, 0.0f, 1.0f);
		const float BlendAlpha = LinearAlpha * LinearAlpha * (3.0f - 2.0f * LinearAlpha);

		SmoothedTarget = ReleaseStartTarget;
		SmoothedTarget.bHasTarget = true;
		SmoothedTarget.bIsLocked = false;
		bOutIsReleasing = true;
		SmoothedTarget.TargetLocation = FMath::Lerp(ReleaseStartTarget.TargetLocation, ReferenceTarget.TargetLocation, BlendAlpha);
		SmoothedTarget.TargetNormal = FMath::Lerp(ReleaseStartTarget.TargetNormal, ReferenceTarget.TargetNormal, BlendAlpha).GetSafeNormal();
		if (SmoothedTarget.TargetNormal.IsNearlyZero())
		{
			SmoothedTarget.TargetNormal = ReferenceTarget.TargetNormal.GetSafeNormal();
		}
		SmoothedTarget.TargetRotation = FQuat::Slerp(
			ReleaseStartTarget.TargetRotation.Quaternion(),
			ReferenceTarget.TargetRotation.Quaternion(),
			BlendAlpha).Rotator();

		if (LinearAlpha >= 1.0f)
		{
			ReleaseStartTarget = FClimbingLimbAnimTarget();
			ReleaseStartTarget.Limb = ReleaseLimb;
			ReleaseBlendElapsed = 0.0f;
			bReleaseBlendActive = false;
			FClimbingLimbAnimTarget CompletedTarget = ReferenceTarget;
			CompletedTarget.bHasTarget = false;
			CompletedTarget.bIsLocked = false;
			return CompletedTarget;
		}

		return SmoothedTarget;
	}

	ReleaseBlendElapsed = 0.0f;
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
	const FClimbingLimbAnimTarget NeutralTarget = GetNeutralLimbPoseTarget(Limb);
	if (NeutralTarget.bHasTarget)
	{
		return NeutralTarget.TargetLocation;
	}

	return FVector::ZeroVector;
}

FClimbingLimbAnimTarget UClimbingAnimInstance::BuildReferencePoseTarget(EClimbingLimb Limb, const USkeletalMesh* SkeletalMesh) const
{
	FClimbingLimbAnimTarget Target;
	Target.Limb = Limb;
	Target.bIsLocked = false;

	if (!SkeletalMesh)
	{
		return Target;
	}

	const FName ReferenceName =
		(Limb == EClimbingLimb::LeftHand) ? FName(TEXT("hand_l")) :
		(Limb == EClimbingLimb::RightHand) ? FName(TEXT("hand_r")) :
		(Limb == EClimbingLimb::LeftFoot) ? FName(TEXT("foot_l")) :
		(Limb == EClimbingLimb::RightFoot) ? FName(TEXT("foot_r")) :
		NAME_None;

	const FReferenceSkeleton& ReferenceSkeleton = SkeletalMesh->GetRefSkeleton();
	const int32 BoneIndex = ReferenceName != NAME_None ? ReferenceSkeleton.FindBoneIndex(ReferenceName) : INDEX_NONE;
	if (BoneIndex == INDEX_NONE)
	{
		return Target;
	}

	FTransform ComponentSpaceTransform = ReferenceSkeleton.GetRefBonePose()[BoneIndex];
	for (int32 ParentIndex = ReferenceSkeleton.GetParentIndex(BoneIndex); ParentIndex != INDEX_NONE; ParentIndex = ReferenceSkeleton.GetParentIndex(ParentIndex))
	{
		ComponentSpaceTransform = ComponentSpaceTransform * ReferenceSkeleton.GetRefBonePose()[ParentIndex];
	}

	Target.bHasTarget = true;
	Target.TargetLocation = ComponentSpaceTransform.GetLocation();
	Target.TargetRotation = ComponentSpaceTransform.Rotator();
	Target.TargetNormal = ComponentSpaceTransform.GetRotation().GetUpVector();
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

void UClimbingAnimInstance::CacheNeutralPoseTargets(const USkeletalMeshComponent* SkeletalMeshComponent)
{
	if (!SkeletalMeshComponent)
	{
		return;
	}

	USkeletalMesh* SkeletalMesh = SkeletalMeshComponent->GetSkeletalMeshAsset();
	if (!SkeletalMesh || CachedNeutralPoseMesh.Get() == SkeletalMesh)
	{
		return;
	}

	CachedNeutralPoseMesh = SkeletalMesh;
	LeftHandNeutralTarget = BuildReferencePoseTarget(EClimbingLimb::LeftHand, SkeletalMesh);
	RightHandNeutralTarget = BuildReferencePoseTarget(EClimbingLimb::RightHand, SkeletalMesh);
	LeftFootNeutralTarget = BuildReferencePoseTarget(EClimbingLimb::LeftFoot, SkeletalMesh);
	RightFootNeutralTarget = BuildReferencePoseTarget(EClimbingLimb::RightFoot, SkeletalMesh);
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

	CachedNeutralPoseMesh.Reset();

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
	LeftHandReleaseBlendElapsed = 0.0f;
	RightHandReleaseBlendElapsed = 0.0f;
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
	LeftFootReleaseBlendElapsed = 0.0f;
	RightFootReleaseBlendElapsed = 0.0f;
	bLeftFootReleaseBlendActive = false;
	bRightFootReleaseBlendActive = false;

	UpdateControlRigTargets();
}
