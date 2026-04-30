#include "Solvers/ClimbingSolver.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClimbingSolverProjectionTest, "Climb.Solver.ProjectCenterOfMassToWallPlane", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClimbingSolverProjectionTest::RunTest(const FString& Parameters)
{
	const FVector Projected = UClimbingSolver::ProjectCenterOfMassToWallPlane(
		FVector(30.0f, 20.0f, 10.0f),
		FVector::ZeroVector,
		FVector::ForwardVector);

	TestEqual(TEXT("Projection removes wall-normal offset"), Projected, FVector(0.0f, 20.0f, 10.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClimbingSolverStabilityTest, "Climb.Solver.EstimateTwoPointStability", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClimbingSolverStabilityTest::RunTest(const FString& Parameters)
{
	const FVector FirstContact(0.0f, -50.0f, 0.0f);
	const FVector SecondContact(0.0f, 50.0f, 0.0f);

	const FClimbingStabilityResult Stable = UClimbingSolver::EstimateTwoPointStability(
		FVector(0.0f, 0.0f, 10.0f),
		FirstContact,
		SecondContact,
		FVector::ForwardVector,
		25.0f);

	TestTrue(TEXT("CoM near support line is stable"), Stable.bIsStable);
	TestTrue(TEXT("Two distinct contacts create support"), Stable.bHasSupport);

	const FClimbingStabilityResult Unstable = UClimbingSolver::EstimateTwoPointStability(
		FVector(0.0f, 0.0f, 100.0f),
		FirstContact,
		SecondContact,
		FVector::ForwardVector,
		25.0f);

	TestFalse(TEXT("CoM far from support line is unstable"), Unstable.bIsStable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClimbingSolverBodyTensionTest, "Climb.Solver.EstimateBodyTension", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClimbingSolverBodyTensionTest::RunTest(const FString& Parameters)
{
	const float Tension = UClimbingSolver::EstimateBodyTension(
		FVector(0.0f, 0.0f, 60.0f),
		FVector(0.0f, -50.0f, 0.0f),
		FVector(0.0f, 50.0f, 0.0f),
		120.0f);

	TestEqual(TEXT("Lever arm halfway to max tension returns 0.5"), Tension, 0.5f);

	const float MissingContactTension = UClimbingSolver::EstimateBodyTension(
		FVector::ZeroVector,
		FVector::ZeroVector,
		FVector::ZeroVector,
		120.0f);

	TestEqual(TEXT("Missing contact span has no usable tension estimate"), MissingContactTension, 0.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClimbingSolverMultiContactStabilityTest, "Climb.Solver.EstimateMultiContactStability", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClimbingSolverMultiContactStabilityTest::RunTest(const FString& Parameters)
{
	const TArray<FVector> SupportContacts{
		FVector(0.0f, -40.0f, 0.0f),
		FVector(0.0f, 40.0f, 0.0f),
		FVector(0.0f, 0.0f, 80.0f)
	};

	const FClimbingStabilityResult Stable = UClimbingSolver::EstimateMultiContactStability(
		FVector(0.0f, 0.0f, 20.0f),
		SupportContacts,
		FVector::ForwardVector,
		20.0f);

	TestTrue(TEXT("Projected CoM inside support triangle is stable"), Stable.bIsStable);
	TestEqual(TEXT("Inside polygon produces full stability percent"), Stable.StabilityPercent, 1.0f);

	const FClimbingStabilityResult Unstable = UClimbingSolver::EstimateMultiContactStability(
		FVector(0.0f, 0.0f, 140.0f),
		SupportContacts,
		FVector::ForwardVector,
		20.0f);

	TestFalse(TEXT("Projected CoM outside support triangle becomes unstable"), Unstable.bIsStable);
	TestTrue(TEXT("Outside polygon lowers stability percent"), Unstable.StabilityPercent < 1.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClimbingSolverMultiContactBodyTensionTest, "Climb.Solver.EstimateMultiContactBodyTension", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClimbingSolverMultiContactBodyTensionTest::RunTest(const FString& Parameters)
{
	// A three-point support triangle should still produce a readable tension gradient as the CoM approaches an edge.
	const TArray<FVector> SupportContacts{
		FVector(0.0f, -50.0f, 0.0f),
		FVector(0.0f, 50.0f, 0.0f),
		FVector(0.0f, 0.0f, 100.0f)
	};

	const float LowerTension = UClimbingSolver::EstimateMultiContactBodyTension(
		FVector(0.0f, 0.0f, 30.0f),
		SupportContacts,
		FVector::ForwardVector,
		40.0f);

	const float HigherTension = UClimbingSolver::EstimateMultiContactBodyTension(
		FVector(0.0f, 0.0f, 85.0f),
		SupportContacts,
		FVector::ForwardVector,
		40.0f);

	TestTrue(TEXT("Moving closer to the support boundary increases multi-contact body tension"), HigherTension > LowerTension);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FClimbingSolverDynoVelocityTest, "Climb.Solver.CalculateDynoLaunchVelocity", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FClimbingSolverDynoVelocityTest::RunTest(const FString& Parameters)
{
	const FVector Velocity = UClimbingSolver::CalculateDynoLaunchVelocity(
		FVector::ZeroVector,
		FVector(0.0f, 0.0f, 100.0f),
		1.0f,
		-980.0f,
		1800.0f);

	TestEqual(TEXT("One second vertical dyno compensates gravity"), Velocity, FVector(0.0f, 0.0f, 590.0f));
	return true;
}

#endif
