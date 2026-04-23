#include "Solvers/ClimbingSolver.h"

namespace ClimbingSolver
{
constexpr float MinVectorLengthSquared = 1.0e-4f;
}

FVector UClimbingSolver::ProjectCenterOfMassToWallPlane(const FVector& CenterOfMass, const FVector& PlaneOrigin, const FVector& PlaneNormal)
{
	const FVector SafeNormal = PlaneNormal.GetSafeNormal();
	if (SafeNormal.IsNearlyZero())
	{
		return CenterOfMass;
	}

	// Orthogonal projection removes the component of CoM offset that points through the wall normal.
	const FVector OffsetFromPlane = CenterOfMass - PlaneOrigin;
	return CenterOfMass - FVector::DotProduct(OffsetFromPlane, SafeNormal) * SafeNormal;
}

FClimbingStabilityResult UClimbingSolver::EstimateTwoPointStability(const FVector& CenterOfMass, const FVector& FirstContact, const FVector& SecondContact, const FVector& WallNormal, float StableOffsetCm)
{
	FClimbingStabilityResult Result;
	Result.ProjectedCenterOfMass = CenterOfMass;
	Result.NearestSupportPoint = FirstContact;

	const FVector ContactSpan = SecondContact - FirstContact;
	const float ContactSpanLengthSquared = ContactSpan.SizeSquared();
	const float SafeStableOffset = FMath::Max(StableOffsetCm, KINDA_SMALL_NUMBER);
	if (ContactSpanLengthSquared <= ClimbingSolver::MinVectorLengthSquared)
	{
		return Result;
	}

	Result.bHasSupport = true;
	Result.ProjectedCenterOfMass = ProjectCenterOfMassToWallPlane(CenterOfMass, FirstContact, WallNormal);

	// The hand/foot pair forms a support segment; the closest point on it is the balance reference.
	const float SupportAlpha = FMath::Clamp(FVector::DotProduct(Result.ProjectedCenterOfMass - FirstContact, ContactSpan) / ContactSpanLengthSquared, 0.0f, 1.0f);
	Result.NearestSupportPoint = FirstContact + ContactSpan * SupportAlpha;

	const FVector OffsetFromSupport = Result.ProjectedCenterOfMass - Result.NearestSupportPoint;
	const float DistanceToSupport = OffsetFromSupport.Size();

	const FVector SafeWallNormal = WallNormal.GetSafeNormal();
	const FVector SideAxis = SafeWallNormal.IsNearlyZero()
		? FVector::ZeroVector
		: FVector::CrossProduct(ContactSpan, SafeWallNormal).GetSafeNormal();
	const float DirectionSign = SideAxis.IsNearlyZero() ? 1.0f : FMath::Sign(FVector::DotProduct(OffsetFromSupport, SideAxis));

	Result.SignedDistanceToSupportLine = DistanceToSupport * DirectionSign;
	Result.StabilityPercent = 1.0f - FMath::Clamp(DistanceToSupport / SafeStableOffset, 0.0f, 1.0f);
	Result.bIsStable = DistanceToSupport <= SafeStableOffset;
	return Result;
}

float UClimbingSolver::EstimateBodyTension(const FVector& CenterOfMass, const FVector& FirstContact, const FVector& SecondContact, float MaxBodyTensionOffsetCm)
{
	const FVector ContactSpan = SecondContact - FirstContact;
	const float ContactSpanLengthSquared = ContactSpan.SizeSquared();
	const float SafeMaxOffset = FMath::Max(MaxBodyTensionOffsetCm, KINDA_SMALL_NUMBER);
	if (ContactSpanLengthSquared <= ClimbingSolver::MinVectorLengthSquared)
	{
		return 0.0f;
	}

	// Tension rises as CoM moves away from the line between contact points, like a lever arm under load.
	const float SupportAlpha = FMath::Clamp(FVector::DotProduct(CenterOfMass - FirstContact, ContactSpan) / ContactSpanLengthSquared, 0.0f, 1.0f);
	const FVector NearestSupportPoint = FirstContact + ContactSpan * SupportAlpha;
	const float LeverArmCm = FVector::Distance(CenterOfMass, NearestSupportPoint);
	return FMath::Clamp(LeverArmCm / SafeMaxOffset, 0.0f, 1.0f);
}

FVector UClimbingSolver::CalculateDynoLaunchVelocity(const FVector& StartLocation, const FVector& TargetLocation, float TimeToTargetSeconds, float GravityZ, float MaxLaunchSpeed)
{
	const float SafeTime = FMath::Max(TimeToTargetSeconds, KINDA_SMALL_NUMBER);
	const float SafeMaxLaunchSpeed = FMath::Max(MaxLaunchSpeed, 0.0f);
	const FVector Gravity(0.0f, 0.0f, GravityZ);

	// Ballistic rearrangement: target = start + velocity * t + 0.5 * gravity * t^2.
	const FVector RequiredVelocity = (TargetLocation - StartLocation - 0.5f * Gravity * FMath::Square(SafeTime)) / SafeTime;
	return SafeMaxLaunchSpeed > 0.0f ? RequiredVelocity.GetClampedToMaxSize(SafeMaxLaunchSpeed) : FVector::ZeroVector;
}
