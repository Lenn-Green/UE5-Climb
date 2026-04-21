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
	OutCandidate = FClimbingHoldCandidate();

	UWorld* World = GetWorld();
	if (!World || Direction.IsNearlyZero())
	{
		return false;
	}

	const FVector NormalizedDirection = Direction.GetSafeNormal();
	const FVector End = Start + NormalizedDirection * TraceDistance;

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

		FClimbingHoldCandidate Candidate = MakeCandidate(Hit, Start);
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

FClimbingHoldCandidate UClimbingHoldQueryComponent::MakeCandidate(const FHitResult& Hit, const FVector& Start) const
{
	FClimbingHoldCandidate Candidate;
	Candidate.bIsValid = true;
	Candidate.Location = Hit.ImpactPoint;
	Candidate.Normal = Hit.ImpactNormal.GetSafeNormal();
	Candidate.Actor = Hit.GetActor();
	Candidate.Component = Hit.GetComponent();
	Candidate.Distance = FVector::Distance(Start, Hit.ImpactPoint);
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
