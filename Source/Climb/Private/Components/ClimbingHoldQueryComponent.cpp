#include "Components/ClimbingHoldQueryComponent.h"

#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UClimbingHoldQueryComponent::UClimbingHoldQueryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UClimbingHoldQueryComponent::QueryBestHoldFromViewpoint(FClimbingHoldCandidate& OutCandidate) const
{
	const AActor* Owner = GetOwner();
	if (!Owner)
	{
		OutCandidate = FClimbingHoldCandidate();
		return false;
	}

	const FVector Start = Owner->GetActorLocation();
	const FVector Direction = Owner->GetActorForwardVector();
	return QueryBestHold(Start, Direction, OutCandidate);
}

bool UClimbingHoldQueryComponent::QueryBestHold(const FVector& Start, const FVector& Direction, FClimbingHoldCandidate& OutCandidate) const
{
	const FVector NormalizedDirection = Direction.GetSafeNormal();
	const FVector End = Start + NormalizedDirection * TraceDistance;
	return QueryBestHoldSweep(Start, End, Start, TraceDistance, OutCandidate);
}

bool UClimbingHoldQueryComponent::QueryBestHoldNearPoint(const FVector& Center, const FVector& SearchDirection, float SearchDistance, FClimbingHoldCandidate& OutCandidate) const
{
	OutCandidate = FClimbingHoldCandidate();

	if (SearchDirection.IsNearlyZero() || SearchDistance <= UE_KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const FVector NormalizedSearchDirection = SearchDirection.GetSafeNormal();
	const float HalfDistance = SearchDistance * 0.5f;
	const FVector Start = Center - NormalizedSearchDirection * HalfDistance;
	const FVector End = Center + NormalizedSearchDirection * HalfDistance;
	return QueryBestHoldSweep(Start, End, Center, SearchDistance, OutCandidate);
}

bool UClimbingHoldQueryComponent::QueryBestHoldSweep(const FVector& Start, const FVector& End, const FVector& ScoreOrigin, float ScoreReferenceDistance, FClimbingHoldCandidate& OutCandidate) const
{
	OutCandidate = FClimbingHoldCandidate();

	UWorld* World = GetWorld();
	if (!World || Start.Equals(End))
	{
		return false;
	}

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ClimbingHoldQuery), bTraceComplex);
	QueryParams.AddIgnoredActor(GetOwner());

	TArray<FHitResult> Hits;
	const bool bHit = World->SweepMultiByChannel(
		Hits,
		Start,
		End,
		FQuat::Identity,
		TraceChannel,
		FCollisionShape::MakeSphere(TraceRadius),
		QueryParams);

	if (bDebugDraw)
	{
		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;
		DrawDebugLine(World, Start, End, TraceColor, false, DebugDrawTime, 0, 1.0f);
		DrawDebugSphere(World, End, TraceRadius, 16, TraceColor, false, DebugDrawTime);
	}

	float BestScore = TNumericLimits<float>::Lowest();
	for (const FHitResult& Hit : Hits)
	{
		if (!Hit.bBlockingHit || !IsExplicitHold(Hit))
		{
			continue;
		}

		// Near-point limb queries should rank holds around the intended reach-plane target rather than
		// around the sweep segment start. This keeps the selected hold aligned with the nearest
		// reachable point for the active hand or foot.
		FClimbingHoldCandidate Candidate = MakeCandidate(Hit, ScoreOrigin);
		Candidate.Score = ScoreReferenceDistance - Candidate.Distance;
		if (Candidate.Score > BestScore)
		{
			BestScore = Candidate.Score;
			OutCandidate = Candidate;
		}
	}

	if (bDebugDraw && OutCandidate.bIsValid)
	{
		DrawDebugPoint(World, OutCandidate.Location, 12.0f, FColor::Yellow, false, DebugDrawTime);
		DrawDebugDirectionalArrow(World, OutCandidate.Location, OutCandidate.Location + OutCandidate.Normal * 35.0f, 12.0f, FColor::Cyan, false, DebugDrawTime);
	}

	return OutCandidate.bIsValid;
}

FClimbingHoldCandidate UClimbingHoldQueryComponent::MakeCandidate(const FHitResult& Hit, const FVector& ScoreOrigin) const
{
	FClimbingHoldCandidate Candidate;
	Candidate.bIsValid = true;
	Candidate.Location = Hit.ImpactPoint;
	Candidate.Normal = Hit.ImpactNormal.GetSafeNormal();
	Candidate.Actor = Hit.GetActor();
	Candidate.Component = Hit.GetComponent();
	Candidate.Distance = FVector::Distance(ScoreOrigin, Hit.ImpactPoint);
	Candidate.Score = TraceDistance - Candidate.Distance;
	return Candidate;
}

bool UClimbingHoldQueryComponent::IsExplicitHold(const FHitResult& Hit) const
{
	const AActor* HitActor = Hit.GetActor();
	const UPrimitiveComponent* HitComponent = Hit.GetComponent();
	return (HitActor && HitActor->ActorHasTag(TEXT("ClimbingHold"))) ||
		(HitComponent && HitComponent->ComponentHasTag(TEXT("ClimbingHold")));
}
