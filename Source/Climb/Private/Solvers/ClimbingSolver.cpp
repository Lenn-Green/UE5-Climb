#include "Solvers/ClimbingSolver.h"

namespace ClimbingSolver
{
constexpr float MinVectorLengthSquared = 1.0e-4f;

struct FSupportContact2D
{
	FVector WorldPoint = FVector::ZeroVector;
	FVector2D PlanePoint = FVector2D::ZeroVector;
};

struct FSupportHullResult
{
	bool bHasSupport = false;
	bool bPointInside = false;
	FVector ProjectedCenterOfMass = FVector::ZeroVector;
	FVector NearestSupportPoint = FVector::ZeroVector;
	float DistanceToHull = 0.0f;
	float DistanceToBoundaryInside = 0.0f;
};

static bool BuildWallPlaneBasis(const FVector& WallNormal, FVector& OutAxisX, FVector& OutAxisY)
{
	const FVector SafeWallNormal = WallNormal.GetSafeNormal();
	if (SafeWallNormal.IsNearlyZero())
	{
		return false;
	}

	const FVector SeedAxis = FMath::Abs(SafeWallNormal.Z) < 0.99f ? FVector::UpVector : FVector::RightVector;
	OutAxisX = FVector::CrossProduct(SeedAxis, SafeWallNormal).GetSafeNormal();
	if (OutAxisX.IsNearlyZero())
	{
		return false;
	}

	OutAxisY = FVector::CrossProduct(SafeWallNormal, OutAxisX).GetSafeNormal();
	return !OutAxisY.IsNearlyZero();
}

static FVector2D ToPlanePoint(const FVector& WorldPoint, const FVector& PlaneOrigin, const FVector& AxisX, const FVector& AxisY)
{
	const FVector Offset = WorldPoint - PlaneOrigin;
	return FVector2D(FVector::DotProduct(Offset, AxisX), FVector::DotProduct(Offset, AxisY));
}

static float Cross2D(const FVector2D& A, const FVector2D& B, const FVector2D& C)
{
	const FVector2D AB = B - A;
	const FVector2D AC = C - A;
	return AB.X * AC.Y - AB.Y * AC.X;
}

static FVector2D ClosestPointOnSegment2D(const FVector2D& Point, const FVector2D& SegmentStart, const FVector2D& SegmentEnd)
{
	const FVector2D Segment = SegmentEnd - SegmentStart;
	const float SegmentLengthSquared = Segment.SizeSquared();
	if (SegmentLengthSquared <= MinVectorLengthSquared)
	{
		return SegmentStart;
	}

	const float Alpha = FMath::Clamp(FVector2D::DotProduct(Point - SegmentStart, Segment) / SegmentLengthSquared, 0.0f, 1.0f);
	return SegmentStart + Segment * Alpha;
}

static TArray<FSupportContact2D, TInlineAllocator<8>> BuildSupportHull(const TArray<FVector>& SupportContacts, const FVector& PlaneOrigin, const FVector& AxisX, const FVector& AxisY)
{
	TArray<FSupportContact2D, TInlineAllocator<8>> Points;
	Points.Reserve(SupportContacts.Num());

	for (const FVector& SupportContact : SupportContacts)
	{
		FSupportContact2D Entry;
		Entry.WorldPoint = SupportContact;
		Entry.PlanePoint = ToPlanePoint(SupportContact, PlaneOrigin, AxisX, AxisY);
		Points.Add(Entry);
	}

	Points.Sort([](const FSupportContact2D& A, const FSupportContact2D& B)
	{
		return (A.PlanePoint.X == B.PlanePoint.X) ? (A.PlanePoint.Y < B.PlanePoint.Y) : (A.PlanePoint.X < B.PlanePoint.X);
	});

	TArray<FSupportContact2D, TInlineAllocator<8>> UniquePoints;
	UniquePoints.Reserve(Points.Num());
	for (const FSupportContact2D& Point : Points)
	{
		if (UniquePoints.Num() == 0 || !Point.PlanePoint.Equals(UniquePoints.Last().PlanePoint, KINDA_SMALL_NUMBER))
		{
			UniquePoints.Add(Point);
		}
	}

	if (UniquePoints.Num() <= 2)
	{
		return UniquePoints;
	}

	TArray<FSupportContact2D, TInlineAllocator<8>> LowerHull;
	for (const FSupportContact2D& Point : UniquePoints)
	{
		while (LowerHull.Num() >= 2 &&
			Cross2D(LowerHull[LowerHull.Num() - 2].PlanePoint, LowerHull.Last().PlanePoint, Point.PlanePoint) <= 0.0f)
		{
			LowerHull.Pop();
		}
		LowerHull.Add(Point);
	}

	TArray<FSupportContact2D, TInlineAllocator<8>> UpperHull;
	for (int32 Index = UniquePoints.Num() - 1; Index >= 0; --Index)
	{
		const FSupportContact2D& Point = UniquePoints[Index];
		while (UpperHull.Num() >= 2 &&
			Cross2D(UpperHull[UpperHull.Num() - 2].PlanePoint, UpperHull.Last().PlanePoint, Point.PlanePoint) <= 0.0f)
		{
			UpperHull.Pop();
		}
		UpperHull.Add(Point);
	}

	LowerHull.Pop();
	UpperHull.Pop();
	LowerHull.Append(UpperHull);
	return LowerHull;
}

static FSupportHullResult EvaluateSupportHull(const FVector& CenterOfMass, const TArray<FVector>& SupportContacts, const FVector& WallNormal)
{
	FSupportHullResult Result;
	if (SupportContacts.Num() < 2)
	{
		return Result;
	}

	FVector AxisX = FVector::ZeroVector;
	FVector AxisY = FVector::ZeroVector;
	if (!BuildWallPlaneBasis(WallNormal, AxisX, AxisY))
	{
		return Result;
	}

	FVector PlaneOrigin = FVector::ZeroVector;
	for (const FVector& SupportContact : SupportContacts)
	{
		PlaneOrigin += SupportContact;
	}
	PlaneOrigin /= static_cast<float>(SupportContacts.Num());

	Result.ProjectedCenterOfMass = UClimbingSolver::ProjectCenterOfMassToWallPlane(CenterOfMass, PlaneOrigin, WallNormal);
	const FVector2D ProjectedCenterOfMass2D = ToPlanePoint(Result.ProjectedCenterOfMass, PlaneOrigin, AxisX, AxisY);
	const TArray<FSupportContact2D, TInlineAllocator<8>> Hull = BuildSupportHull(SupportContacts, PlaneOrigin, AxisX, AxisY);
	if (Hull.Num() < 2)
	{
		return Result;
	}

	Result.bHasSupport = true;

	if (Hull.Num() == 2)
	{
		const FVector2D Closest2D = ClosestPointOnSegment2D(ProjectedCenterOfMass2D, Hull[0].PlanePoint, Hull[1].PlanePoint);
		const float SegmentLengthSquared = (Hull[1].PlanePoint - Hull[0].PlanePoint).SizeSquared();
		const float Alpha = SegmentLengthSquared <= MinVectorLengthSquared
			? 0.0f
			: FMath::Clamp(FVector2D::DotProduct(Closest2D - Hull[0].PlanePoint, Hull[1].PlanePoint - Hull[0].PlanePoint) / SegmentLengthSquared, 0.0f, 1.0f);
		Result.NearestSupportPoint = FMath::Lerp(Hull[0].WorldPoint, Hull[1].WorldPoint, Alpha);
		Result.DistanceToHull = FVector2D::Distance(ProjectedCenterOfMass2D, Closest2D);
		Result.DistanceToBoundaryInside = 0.0f;
		Result.bPointInside = Result.DistanceToHull <= KINDA_SMALL_NUMBER;
		return Result;
	}

	float MinDistanceToEdge = TNumericLimits<float>::Max();
	FVector2D NearestPointOnHull2D = FVector2D::ZeroVector;
	FVector NearestWorldPoint = FVector::ZeroVector;
	bool bInsideHull = true;

	for (int32 EdgeIndex = 0; EdgeIndex < Hull.Num(); ++EdgeIndex)
	{
		const FSupportContact2D& EdgeStart = Hull[EdgeIndex];
		const FSupportContact2D& EdgeEnd = Hull[(EdgeIndex + 1) % Hull.Num()];
		if (Cross2D(EdgeStart.PlanePoint, EdgeEnd.PlanePoint, ProjectedCenterOfMass2D) < 0.0f)
		{
			bInsideHull = false;
		}

		const FVector2D Closest2D = ClosestPointOnSegment2D(ProjectedCenterOfMass2D, EdgeStart.PlanePoint, EdgeEnd.PlanePoint);
		const float DistanceToEdge = FVector2D::Distance(ProjectedCenterOfMass2D, Closest2D);
		if (DistanceToEdge < MinDistanceToEdge)
		{
			MinDistanceToEdge = DistanceToEdge;
			NearestPointOnHull2D = Closest2D;
			const FVector2D EdgeDelta = EdgeEnd.PlanePoint - EdgeStart.PlanePoint;
			const float EdgeLengthSquared = EdgeDelta.SizeSquared();
			const float Alpha = EdgeLengthSquared <= MinVectorLengthSquared
				? 0.0f
				: FMath::Clamp(FVector2D::DotProduct(Closest2D - EdgeStart.PlanePoint, EdgeDelta) / EdgeLengthSquared, 0.0f, 1.0f);
			NearestWorldPoint = FMath::Lerp(EdgeStart.WorldPoint, EdgeEnd.WorldPoint, Alpha);
		}
	}

	Result.bPointInside = bInsideHull;
	Result.NearestSupportPoint = NearestWorldPoint;
	Result.DistanceToHull = bInsideHull ? 0.0f : MinDistanceToEdge;
	Result.DistanceToBoundaryInside = bInsideHull ? MinDistanceToEdge : 0.0f;
	return Result;
}
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

FClimbingStabilityResult UClimbingSolver::EstimateMultiContactStability(const FVector& CenterOfMass, const TArray<FVector>& SupportContacts, const FVector& WallNormal, float StableOffsetCm)
{
	FClimbingStabilityResult Result;
	if (SupportContacts.Num() < 2)
	{
		if (SupportContacts.Num() == 1)
		{
			Result.ProjectedCenterOfMass = ProjectCenterOfMassToWallPlane(CenterOfMass, SupportContacts[0], WallNormal);
			Result.NearestSupportPoint = SupportContacts[0];
		}
		return Result;
	}

	const ClimbingSolver::FSupportHullResult Hull = ClimbingSolver::EvaluateSupportHull(CenterOfMass, SupportContacts, WallNormal);
	Result.bHasSupport = Hull.bHasSupport;
	Result.ProjectedCenterOfMass = Hull.ProjectedCenterOfMass;
	Result.NearestSupportPoint = Hull.NearestSupportPoint;
	Result.SignedDistanceToSupportLine = Hull.bPointInside ? Hull.DistanceToBoundaryInside : -Hull.DistanceToHull;

	if (!Hull.bHasSupport)
	{
		return Result;
	}

	const float SafeStableOffset = FMath::Max(StableOffsetCm, KINDA_SMALL_NUMBER);
	if (Hull.bPointInside)
	{
		Result.StabilityPercent = 1.0f;
		Result.bIsStable = true;
	}
	else
	{
		Result.StabilityPercent = 1.0f - FMath::Clamp(Hull.DistanceToHull / SafeStableOffset, 0.0f, 1.0f);
		Result.bIsStable = Hull.DistanceToHull <= SafeStableOffset;
	}

	return Result;
}

float UClimbingSolver::EstimateMultiContactBodyTension(const FVector& CenterOfMass, const TArray<FVector>& SupportContacts, const FVector& WallNormal, float MaxBodyTensionOffsetCm)
{
	if (SupportContacts.Num() < 2)
	{
		return 0.0f;
	}

	if (SupportContacts.Num() == 2)
	{
		return EstimateBodyTension(CenterOfMass, SupportContacts[0], SupportContacts[1], MaxBodyTensionOffsetCm);
	}

	const ClimbingSolver::FSupportHullResult Hull = ClimbingSolver::EvaluateSupportHull(CenterOfMass, SupportContacts, WallNormal);
	if (!Hull.bHasSupport)
	{
		return 0.0f;
	}

	const float SafeMaxOffset = FMath::Max(MaxBodyTensionOffsetCm, KINDA_SMALL_NUMBER);
	// Clearance to the nearest hull edge measures remaining support margin inside the polygon.
	// Less clearance means the pose is leaning on the edge of support, so tension rises toward 1.
	if (Hull.bPointInside)
	{
		return 1.0f - FMath::Clamp(Hull.DistanceToBoundaryInside / SafeMaxOffset, 0.0f, 1.0f);
	}

	return 1.0f;
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
