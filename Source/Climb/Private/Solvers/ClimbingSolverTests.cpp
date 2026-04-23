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
