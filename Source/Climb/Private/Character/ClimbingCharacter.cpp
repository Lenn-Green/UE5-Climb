#include "Character/ClimbingCharacter.h"

#include "Components/ClimbingHoldQueryComponent.h"
#include "Components/ClimbingMovementComponent.h"
#include "Solvers/ClimbingSolver.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "DrawDebugHelpers.h"

AClimbingCharacter::AClimbingCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UClimbingMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;
	HoldQueryComponent = CreateDefaultSubobject<UClimbingHoldQueryComponent>(TEXT("HoldQueryComponent"));
	LeftHandState.Limb = EClimbingLimb::LeftHand;
	RightHandState.Limb = EClimbingLimb::RightHand;
	LeftFootState.Limb = EClimbingLimb::LeftFoot;
	RightFootState.Limb = EClimbingLimb::RightFoot;
}

void AClimbingCharacter::BeginPlay()
{
	Super::BeginPlay();

	const UCharacterMovementComponent* Movement = GetCharacterMovement();
	SetClimbingState(Movement && Movement->IsMovingOnGround() ? EClimbingState::Grounded : EClimbingState::Falling);
	AddClimbingInputMappingContext();
}

void AClimbingCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateClimbingDebugState(DeltaSeconds);
}

void AClimbingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	AddClimbingInputMappingContext();
	BindClimbingInputActions(PlayerInputComponent);
}

void AClimbingCharacter::EnterClimbing()
{
	if (UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent())
	{
		ClimbingMovement->StartClimbingMovement();
	}

	SetClimbingState(EClimbingState::Climbing);
}

void AClimbingCharacter::EnterClimbingWithAttachment(const FClimbingAttachmentFrame& AttachmentFrame)
{
	if (UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent())
	{
		ClimbingMovement->StartClimbingMovement(AttachmentFrame);
	}

	SetClimbingState(EClimbingState::Climbing);
}

void AClimbingCharacter::ExitClimbing()
{
	if (UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent())
	{
		ClimbingMovement->StopClimbingMovement();
	}

	// Falling should not keep any stale limb locks alive once climbing authority is gone.
	ClearLimb(EClimbingLimb::LeftHand);
	ClearLimb(EClimbingLimb::RightHand);
	ClearLimb(EClimbingLimb::LeftFoot);
	ClearLimb(EClimbingLimb::RightFoot);
	UpdateHandLoadPercentages();

	SetClimbingState(EClimbingState::Falling);
}

void AClimbingCharacter::PrepareClimbingJump()
{
	if (ClimbingState == EClimbingState::Climbing)
	{
		SetClimbingState(EClimbingState::PreparingJump);
	}
}

EClimbingState AClimbingCharacter::GetClimbingState() const
{
	return ClimbingState;
}

bool AClimbingCharacter::IsClimbing() const
{
	return ClimbingState == EClimbingState::Climbing || ClimbingState == EClimbingState::PreparingJump;
}

UClimbingMovementComponent* AClimbingCharacter::GetClimbingMovementComponent() const
{
	return Cast<UClimbingMovementComponent>(GetCharacterMovement());
}

UClimbingHoldQueryComponent* AClimbingCharacter::GetHoldQueryComponent() const
{
	return HoldQueryComponent;
}

FVector2D AClimbingCharacter::GetClimbCenterOfMassInput() const
{
	return ClimbCenterOfMassInput;
}

FVector2D AClimbingCharacter::GetClimbLimbProbeInput() const
{
	return ClimbLimbProbeInput;
}

float AClimbingCharacter::GetLeftGripInput() const
{
	return LeftGripInput;
}

float AClimbingCharacter::GetRightGripInput() const
{
	return RightGripInput;
}

float AClimbingCharacter::GetLeftFootGripInput() const
{
	return LeftFootGripInput;
}

float AClimbingCharacter::GetRightFootGripInput() const
{
	return RightFootGripInput;
}

FLimbState AClimbingCharacter::GetLeftHandState() const
{
	return LeftHandState;
}

FLimbState AClimbingCharacter::GetRightHandState() const
{
	return RightHandState;
}

FLimbState AClimbingCharacter::GetLeftFootState() const
{
	return LeftFootState;
}

FLimbState AClimbingCharacter::GetRightFootState() const
{
	return RightFootState;
}

EClimbingLimb AClimbingCharacter::GetActiveProbeLimb() const
{
	return ActiveProbeLimb;
}

FClimbingDebugState AClimbingCharacter::GetClimbingDebugState() const
{
	return ClimbingDebugState;
}

void AClimbingCharacter::TryLockFoot(EClimbingLimb FootLimb)
{
	if (FootLimb != EClimbingLimb::LeftFoot && FootLimb != EClimbingLimb::RightFoot)
	{
		return;
	}

	TryLockLimb(FootLimb);
}

void AClimbingCharacter::ReleaseFoot(EClimbingLimb FootLimb)
{
	if (FootLimb != EClimbingLimb::LeftFoot && FootLimb != EClimbingLimb::RightFoot)
	{
		return;
	}

	ReleaseLimb(FootLimb);
}

void AClimbingCharacter::AddClimbingInputMappingContext() const
{
	if (!ClimbingInputMappingContext)
	{
		UE_LOG(LogTemp, Warning, TEXT("Climbing input mapping context is not assigned on %s."), *GetName());
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		InputSubsystem->AddMappingContext(ClimbingInputMappingContext, ClimbingInputMappingPriority);
	}
}

void AClimbingCharacter::BindClimbingInputActions(UInputComponent* PlayerInputComponent)
{
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("AClimbingCharacter requires UEnhancedInputComponent for climbing controls."));
		return;
	}

	if (ClimbCenterOfMassMoveAction)
	{
		EnhancedInputComponent->BindAction(ClimbCenterOfMassMoveAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbCenterOfMassMove);
		EnhancedInputComponent->BindAction(ClimbCenterOfMassMoveAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbCenterOfMassMoveCompleted);
	}

	if (ClimbLimbProbeAction)
	{
		EnhancedInputComponent->BindAction(ClimbLimbProbeAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbLimbProbe);
		EnhancedInputComponent->BindAction(ClimbLimbProbeAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbLimbProbeCompleted);
	}

	if (ClimbLeftGripAction)
	{
		EnhancedInputComponent->BindAction(ClimbLeftGripAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbLeftGrip);
		EnhancedInputComponent->BindAction(ClimbLeftGripAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbLeftGripCompleted);
	}

	if (ClimbRightGripAction)
	{
		EnhancedInputComponent->BindAction(ClimbRightGripAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbRightGrip);
		EnhancedInputComponent->BindAction(ClimbRightGripAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbRightGripCompleted);
	}

	if (ClimbLeftFootGripAction)
	{
		EnhancedInputComponent->BindAction(ClimbLeftFootGripAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbLeftFootGrip);
		EnhancedInputComponent->BindAction(ClimbLeftFootGripAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbLeftFootGripCompleted);
	}

	if (ClimbRightFootGripAction)
	{
		EnhancedInputComponent->BindAction(ClimbRightFootGripAction, ETriggerEvent::Triggered, this, &AClimbingCharacter::HandleClimbRightFootGrip);
		EnhancedInputComponent->BindAction(ClimbRightFootGripAction, ETriggerEvent::Completed, this, &AClimbingCharacter::HandleClimbRightFootGripCompleted);
	}
}

void AClimbingCharacter::HandleClimbCenterOfMassMove(const FInputActionValue& Value)
{
	ClimbCenterOfMassInput = Value.Get<FVector2D>();
}

void AClimbingCharacter::HandleClimbCenterOfMassMoveCompleted(const FInputActionValue& Value)
{
	ClimbCenterOfMassInput = FVector2D::ZeroVector;
}

void AClimbingCharacter::HandleClimbLimbProbe(const FInputActionValue& Value)
{
	ClimbLimbProbeInput = Value.Get<FVector2D>();
}

void AClimbingCharacter::HandleClimbLimbProbeCompleted(const FInputActionValue& Value)
{
	ClimbLimbProbeInput = FVector2D::ZeroVector;
}

void AClimbingCharacter::HandleClimbLeftGrip(const FInputActionValue& Value)
{
	const bool bWasPressed = LeftGripInput >= GripPressedThreshold;
	LeftGripInput = Value.Get<float>();
	const bool bIsPressed = LeftGripInput >= GripPressedThreshold;
	if (!bWasPressed && bIsPressed)
	{
		TryLockLimb(EClimbingLimb::LeftHand);
	}
}

void AClimbingCharacter::HandleClimbLeftGripCompleted(const FInputActionValue& Value)
{
	LeftGripInput = 0.0f;
	ReleaseLimb(EClimbingLimb::LeftHand);
}

void AClimbingCharacter::HandleClimbRightGrip(const FInputActionValue& Value)
{
	const bool bWasPressed = RightGripInput >= GripPressedThreshold;
	RightGripInput = Value.Get<float>();
	const bool bIsPressed = RightGripInput >= GripPressedThreshold;
	if (!bWasPressed && bIsPressed)
	{
		TryLockLimb(EClimbingLimb::RightHand);
	}
}

void AClimbingCharacter::HandleClimbRightGripCompleted(const FInputActionValue& Value)
{
	RightGripInput = 0.0f;
	ReleaseLimb(EClimbingLimb::RightHand);
}

void AClimbingCharacter::HandleClimbLeftFootGrip(const FInputActionValue& Value)
{
	const bool bWasPressed = LeftFootGripInput >= GripPressedThreshold;
	LeftFootGripInput = Value.Get<float>();
	const bool bIsPressed = LeftFootGripInput >= GripPressedThreshold;
	if (!bWasPressed && bIsPressed)
	{
		TryLockLimb(EClimbingLimb::LeftFoot);
	}
}

void AClimbingCharacter::HandleClimbLeftFootGripCompleted(const FInputActionValue& Value)
{
	LeftFootGripInput = 0.0f;
	ReleaseLimb(EClimbingLimb::LeftFoot);
}

void AClimbingCharacter::HandleClimbRightFootGrip(const FInputActionValue& Value)
{
	const bool bWasPressed = RightFootGripInput >= GripPressedThreshold;
	RightFootGripInput = Value.Get<float>();
	const bool bIsPressed = RightFootGripInput >= GripPressedThreshold;
	if (!bWasPressed && bIsPressed)
	{
		TryLockLimb(EClimbingLimb::RightFoot);
	}
}

void AClimbingCharacter::HandleClimbRightFootGripCompleted(const FInputActionValue& Value)
{
	RightFootGripInput = 0.0f;
	ReleaseLimb(EClimbingLimb::RightFoot);
}

void AClimbingCharacter::TryLockLimb(EClimbingLimb Limb)
{
	ActiveProbeLimb = Limb;

	FClimbingHoldCandidate Candidate;
	if (IsClimbing())
	{
		RefreshProbeCandidateForActiveLimb();
		if (ClimbingDebugState.CurrentHoldCandidate.bIsValid)
		{
			Candidate = ClimbingDebugState.CurrentHoldCandidate;
		}
	}

	if (!Candidate.bIsValid)
	{
		if (!HoldQueryComponent || !HoldQueryComponent->QueryBestHoldFromViewpoint(Candidate))
		{
			UE_LOG(LogTemp, Verbose, TEXT("No valid climbing hold found for %s."), *UEnum::GetValueAsString(Limb));
			return;
		}
	}

	if (!Candidate.bIsValid)
	{
		UE_LOG(LogTemp, Verbose, TEXT("No valid climbing hold found for %s."), *UEnum::GetValueAsString(Limb));
		return;
	}

	ApplyHoldCandidateToLimb(Limb, Candidate);
	UpdateHandLoadPercentages();
	RefreshClimbingAttachment();
	RefreshAutomaticActiveHand();
}

void AClimbingCharacter::ReleaseLimb(EClimbingLimb Limb)
{
	const bool bWasLocked = GetLimbState(Limb).bIsLocked;
	ClearLimb(Limb);
	ActiveProbeLimb = Limb;
	UpdateHandLoadPercentages();
	RefreshAutomaticActiveHand();

	if (bWasLocked && !HasLockedHand() && IsClimbing())
	{
		ExitClimbing();
		return;
	}

	if (bWasLocked && HasLockedHand() && IsClimbing())
	{
		RefreshClimbingAttachment();
	}
}

bool AClimbingCharacter::HasLockedHand() const
{
	return LeftHandState.bIsLocked || RightHandState.bIsLocked;
}

bool AClimbingCharacter::HasLockedFoot() const
{
	return LeftFootState.bIsLocked || RightFootState.bIsLocked;
}

void AClimbingCharacter::RefreshClimbingAttachment()
{
	const FClimbingAttachmentFrame AttachmentFrame = BuildAttachmentFrameFromLockedHands();
	if (!AttachmentFrame.bIsValid)
	{
		if (IsClimbing())
		{
			ExitClimbing();
		}
		return;
	}

	if (UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent())
	{
		if (ClimbingMovement->IsInClimbingMovementMode())
		{
			ClimbingMovement->UpdateClimbingAttachmentFrame(AttachmentFrame);
		}
		else
		{
			ClimbingMovement->StartClimbingMovement(AttachmentFrame);
		}
	}

	SetClimbingState(EClimbingState::Climbing);
}

void AClimbingCharacter::UpdateHandLoadPercentages()
{
	const int32 LockedLimbCount =
		(LeftHandState.bIsLocked ? 1 : 0) +
		(RightHandState.bIsLocked ? 1 : 0) +
		(LeftFootState.bIsLocked ? 1 : 0) +
		(RightFootState.bIsLocked ? 1 : 0);
	const float SharedLoad = LockedLimbCount > 0 ? 1.0f / LockedLimbCount : 0.0f;
	LeftHandState.LoadPercent = LeftHandState.bIsLocked ? SharedLoad : 0.0f;
	RightHandState.LoadPercent = RightHandState.bIsLocked ? SharedLoad : 0.0f;
	LeftFootState.LoadPercent = LeftFootState.bIsLocked ? SharedLoad : 0.0f;
	RightFootState.LoadPercent = RightFootState.bIsLocked ? SharedLoad : 0.0f;
}

FLimbState& AClimbingCharacter::GetMutableLimbState(EClimbingLimb Limb)
{
	switch (Limb)
	{
	case EClimbingLimb::LeftHand:
		return LeftHandState;
	case EClimbingLimb::RightHand:
		return RightHandState;
	case EClimbingLimb::LeftFoot:
		return LeftFootState;
	case EClimbingLimb::RightFoot:
	default:
		return RightFootState;
	}
}

const FLimbState& AClimbingCharacter::GetLimbState(EClimbingLimb Limb) const
{
	switch (Limb)
	{
	case EClimbingLimb::LeftHand:
		return LeftHandState;
	case EClimbingLimb::RightHand:
		return RightHandState;
	case EClimbingLimb::LeftFoot:
		return LeftFootState;
	case EClimbingLimb::RightFoot:
	default:
		return RightFootState;
	}
}

void AClimbingCharacter::ApplyHoldCandidateToLimb(EClimbingLimb Limb, const FClimbingHoldCandidate& Candidate)
{
	FLimbState& LimbState = GetMutableLimbState(Limb);
	LimbState.bHasValidContact = Candidate.bIsValid;
	LimbState.bIsLocked = Candidate.bIsValid;
	LimbState.ContactLocation = Candidate.Location;
	LimbState.ContactNormal = Candidate.Normal;
	LimbState.ContactRotation = Candidate.Normal.ToOrientationRotator();
	LimbState.ContactActor = Candidate.Actor;
	LimbState.ContactComponent = Candidate.Component;
}

void AClimbingCharacter::ClearLimb(EClimbingLimb Limb)
{
	FLimbState& LimbState = GetMutableLimbState(Limb);
	const EClimbingLimb LimbId = LimbState.Limb;
	LimbState = FLimbState();
	LimbState.Limb = LimbId;
}

void AClimbingCharacter::RefreshAutomaticActiveHand()
{
	if (LeftHandState.bIsLocked == RightHandState.bIsLocked)
	{
		return;
	}

	// With exactly one supporting hand locked, the free hand becomes the active probe limb.
	// This keeps probe input and exploration IK attached to the hand that is actually searching.
	ActiveProbeLimb = LeftHandState.bIsLocked ? EClimbingLimb::RightHand : EClimbingLimb::LeftHand;
}

FClimbingAttachmentFrame AClimbingCharacter::BuildAttachmentFrameFromLockedHands() const
{
	FClimbingAttachmentFrame AttachmentFrame;

	int32 LockedHandCount = 0;
	if (LeftHandState.bIsLocked)
	{
		AttachmentFrame.AnchorLocation += LeftHandState.ContactLocation;
		AttachmentFrame.WallNormal += LeftHandState.ContactNormal.GetSafeNormal();
		++LockedHandCount;
	}

	if (RightHandState.bIsLocked)
	{
		AttachmentFrame.AnchorLocation += RightHandState.ContactLocation;
		AttachmentFrame.WallNormal += RightHandState.ContactNormal.GetSafeNormal();
		++LockedHandCount;
	}

	if (LockedHandCount == 0)
	{
		return AttachmentFrame;
	}

	AttachmentFrame.bIsValid = true;
	AttachmentFrame.AnchorLocation /= LockedHandCount;
	AttachmentFrame.ContactLocation = AttachmentFrame.AnchorLocation;
	AttachmentFrame.WallNormal = AttachmentFrame.WallNormal.GetSafeNormal();
	if (AttachmentFrame.WallNormal.IsNearlyZero())
	{
		AttachmentFrame.WallNormal = -GetActorForwardVector();
	}

	FillWallAxes(AttachmentFrame);
	return AttachmentFrame;
}

void AClimbingCharacter::FillWallAxes(FClimbingAttachmentFrame& AttachmentFrame)
{
	const FVector SafeWallNormal = AttachmentFrame.WallNormal.GetSafeNormal();
	AttachmentFrame.WallNormal = SafeWallNormal.IsNearlyZero() ? FVector::ForwardVector : SafeWallNormal;

	// The wall plane axes are built by crossing world up with the wall normal, then deriving wall up from that tangent.
	AttachmentFrame.WallRight = FVector::CrossProduct(FVector::UpVector, AttachmentFrame.WallNormal).GetSafeNormal();
	if (AttachmentFrame.WallRight.IsNearlyZero())
	{
		AttachmentFrame.WallRight = FVector::RightVector;
	}

	AttachmentFrame.WallUp = FVector::CrossProduct(AttachmentFrame.WallNormal, AttachmentFrame.WallRight).GetSafeNormal();
	if (AttachmentFrame.WallUp.IsNearlyZero())
	{
		AttachmentFrame.WallUp = FVector::UpVector;
	}
}

void AClimbingCharacter::UpdateClimbingDebugState(float DeltaSeconds)
{
	const UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent();
	const FClimbingAttachmentFrame AttachmentFrame = ClimbingMovement ? ClimbingMovement->GetClimbingAttachmentFrame() : FClimbingAttachmentFrame();
	if (!IsClimbing() || !AttachmentFrame.bIsValid)
	{
		ClimbingDebugState = FClimbingDebugState();
		return;
	}

	const FVector WallRight = AttachmentFrame.WallRight.GetSafeNormal();
	const FVector WallUp = AttachmentFrame.WallUp.GetSafeNormal();
	const FVector2D ClampedInput(
		FMath::Clamp(ClimbCenterOfMassInput.X, -1.0f, 1.0f),
		FMath::Clamp(ClimbCenterOfMassInput.Y, -1.0f, 1.0f));

	// CoM debug motion is constrained to the wall tangent plane using the attachment frame right/up axes.
	ClimbingDebugState.CenterOfMassTargetOffset =
		WallRight * (ClampedInput.X * MaxCenterOfMassHorizontalOffset) +
		WallUp * (ClampedInput.Y * MaxCenterOfMassVerticalOffset);
	ClimbingDebugState.CenterOfMassTarget =
		AttachmentFrame.AnchorLocation +
		AttachmentFrame.WallNormal.GetSafeNormal() * AttachmentFrame.TargetWallDistance +
		ClimbingDebugState.CenterOfMassTargetOffset;

	const bool bHasLeftHand = LeftHandState.bIsLocked;
	const bool bHasRightHand = RightHandState.bIsLocked;
	if (bHasLeftHand && bHasRightHand)
	{
		const FClimbingStabilityResult Stability = UClimbingSolver::EstimateTwoPointStability(
			ClimbingDebugState.CenterOfMassTarget,
			LeftHandState.ContactLocation,
			RightHandState.ContactLocation,
			AttachmentFrame.WallNormal,
			StableOffsetThreshold);

		ClimbingDebugState.CurrentBodyTension = UClimbingSolver::EstimateBodyTension(
			ClimbingDebugState.CenterOfMassTarget,
			LeftHandState.ContactLocation,
			RightHandState.ContactLocation,
			MaxBodyTensionOffset);
		ClimbingDebugState.StabilityPercent = Stability.StabilityPercent;
		ClimbingDebugState.bIsPoseStable = Stability.bIsStable;
	}
	else
	{
		ClimbingDebugState.CurrentBodyTension = 0.0f;
		ClimbingDebugState.StabilityPercent = HasLockedHand() ? 1.0f : 0.0f;
		ClimbingDebugState.bIsPoseStable = HasLockedHand();
	}

	UpdateLimbProbeCandidate(AttachmentFrame);

	if ((ActiveProbeLimb == EClimbingLimb::LeftHand || ActiveProbeLimb == EClimbingLimb::RightHand) && !GetLimbState(ActiveProbeLimb).bIsLocked)
	{
		ClimbingDebugState.bHasActiveExplorationTarget = true;
		ClimbingDebugState.ActiveExplorationTargetLocation = ClimbingDebugState.CurrentHoldCandidate.bIsValid
			? ClimbingDebugState.CurrentHoldCandidate.Location
			: ClimbingDebugState.ProbeOrigin + ClimbingDebugState.ProbeDirection * HandExplorationReach;
		ClimbingDebugState.ActiveExplorationTargetNormal = ClimbingDebugState.CurrentHoldCandidate.bIsValid
			? ClimbingDebugState.CurrentHoldCandidate.Normal
			: (-AttachmentFrame.WallNormal).GetSafeNormal();
	}

	DrawClimbingDebugState();
}

void AClimbingCharacter::UpdateLimbProbeCandidate(const FClimbingAttachmentFrame& AttachmentFrame)
{
	ClimbingDebugState.ProbeOrigin = GetActiveLimbProbeOrigin(AttachmentFrame);
	const FVector ProbeTarget = GetActiveLimbProbeTarget(AttachmentFrame, ClimbingDebugState.ProbeOrigin);
	ClimbingDebugState.ProbeDirection = (ProbeTarget - ClimbingDebugState.ProbeOrigin).GetSafeNormal();

	FClimbingHoldCandidate Candidate;
	if (HoldQueryComponent && HoldQueryComponent->QueryBestHold(ClimbingDebugState.ProbeOrigin, ClimbingDebugState.ProbeDirection, Candidate))
	{
		ClimbingDebugState.CurrentHoldCandidate = Candidate;
		return;
	}

	ClimbingDebugState.CurrentHoldCandidate = FClimbingHoldCandidate();
}

bool AClimbingCharacter::RefreshProbeCandidateForActiveLimb()
{
	const UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent();
	const FClimbingAttachmentFrame AttachmentFrame = ClimbingMovement ? ClimbingMovement->GetClimbingAttachmentFrame() : FClimbingAttachmentFrame();
	if (!IsClimbing() || !AttachmentFrame.bIsValid)
	{
		return false;
	}

	UpdateLimbProbeCandidate(AttachmentFrame);
	return ClimbingDebugState.CurrentHoldCandidate.bIsValid;
}

FVector AClimbingCharacter::GetActiveLimbProbeOrigin(const FClimbingAttachmentFrame& AttachmentFrame) const
{
	const FLimbState& ActiveLimbState = GetLimbState(ActiveProbeLimb);
	if (ActiveLimbState.bIsLocked || ActiveLimbState.bHasValidContact)
	{
		return ActiveLimbState.ContactLocation + AttachmentFrame.WallNormal.GetSafeNormal() * AttachmentFrame.TargetWallDistance;
	}

	if (const USkeletalMeshComponent* MeshComponent = GetMesh())
	{
		const TArray<FName> ProbeSocketNames =
			(ActiveProbeLimb == EClimbingLimb::LeftHand) ? TArray<FName>{TEXT("hand_probe_l"), TEXT("hand_l")} :
			(ActiveProbeLimb == EClimbingLimb::RightHand) ? TArray<FName>{TEXT("hand_probe_r"), TEXT("hand_r")} :
			(ActiveProbeLimb == EClimbingLimb::LeftFoot) ? TArray<FName>{TEXT("foot_probe_l"), TEXT("foot_l")} :
			(ActiveProbeLimb == EClimbingLimb::RightFoot) ? TArray<FName>{TEXT("foot_probe_r"), TEXT("foot_r")} :
			TArray<FName>{};

		for (const FName ProbeSocketName : ProbeSocketNames)
		{
			if (ProbeSocketName != NAME_None && MeshComponent->DoesSocketExist(ProbeSocketName))
			{
				return MeshComponent->GetSocketLocation(ProbeSocketName);
			}
		}
	}

	return AttachmentFrame.AnchorLocation + AttachmentFrame.WallNormal.GetSafeNormal() * AttachmentFrame.TargetWallDistance;
}

FVector AClimbingCharacter::GetActiveLimbProbeTarget(const FClimbingAttachmentFrame& AttachmentFrame, const FVector& ProbeOrigin) const
{
	const FClimbingAttachmentFrame ProbeFrame = BuildProbeFrame(AttachmentFrame, ProbeOrigin);
	const FVector ProbeOffset =
		ProbeFrame.WallRight.GetSafeNormal() * (ClimbLimbProbeInput.X * LimbProbeHorizontalRange) +
		ProbeFrame.WallUp.GetSafeNormal() * (ClimbLimbProbeInput.Y * LimbProbeVerticalRange);
	FVector ProbeCenter = ProbeFrame.AnchorLocation;

	if (ActiveProbeLimb == EClimbingLimb::LeftFoot || ActiveProbeLimb == EClimbingLimb::RightFoot)
	{
		// Foot probing should search from the foot's local stance area rather than the hand-built body anchor.
		ProbeCenter += GetActorForwardVector().GetSafeNormal() * FootProbeForwardOffset;
		ProbeCenter += ProbeFrame.WallUp.GetSafeNormal() * FootProbeVerticalBias;
	}

	return ProbeCenter + ProbeOffset - ProbeFrame.WallNormal.GetSafeNormal() * LimbProbeWallDepth;
}

FClimbingAttachmentFrame AClimbingCharacter::BuildProbeFrame(const FClimbingAttachmentFrame& AttachmentFrame, const FVector& ProbeOrigin) const
{
	if (AttachmentFrame.bIsValid)
	{
		FClimbingAttachmentFrame ProbeFrame = AttachmentFrame;
		if (ActiveProbeLimb == EClimbingLimb::LeftFoot || ActiveProbeLimb == EClimbingLimb::RightFoot)
		{
			ProbeFrame.AnchorLocation = ProbeOrigin;
		}

		return ProbeFrame;
	}

	FClimbingAttachmentFrame ProbeFrame;
	ProbeFrame.bIsValid = true;
	ProbeFrame.AnchorLocation = ProbeOrigin;
	ProbeFrame.ContactLocation = ProbeOrigin;
	ProbeFrame.WallNormal = -GetActorForwardVector().GetSafeNormal();
	if (ProbeFrame.WallNormal.IsNearlyZero())
	{
		ProbeFrame.WallNormal = FVector::ForwardVector;
	}

	FillWallAxes(ProbeFrame);
	return ProbeFrame;
}

void AClimbingCharacter::DrawClimbingDebugState() const
{
	if (!bDrawClimbingDebug)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	DrawDebugSphere(World, ClimbingDebugState.CenterOfMassTarget, CenterOfMassDebugSphereRadius, 12, FColor::Cyan, false, 0.0f);
	DrawDebugLine(World, GetActorLocation(), ClimbingDebugState.CenterOfMassTarget, FColor::Cyan, false, 0.0f, 0, 1.5f);
	const FColor StabilityColor = ClimbingDebugState.bIsPoseStable ? FColor::Green : FColor::Red;
	DrawDebugDirectionalArrow(
		World,
		ClimbingDebugState.CenterOfMassTarget,
		ClimbingDebugState.CenterOfMassTarget + FVector::UpVector * 25.0f,
		10.0f,
		StabilityColor,
		false,
		0.0f,
		0,
		1.5f);

	if (!ClimbingDebugState.ProbeDirection.IsNearlyZero())
	{
		const FVector ProbeEnd = ClimbingDebugState.ProbeOrigin + ClimbingDebugState.ProbeDirection * 120.0f;
		DrawDebugLine(World, ClimbingDebugState.ProbeOrigin, ProbeEnd, FColor::Orange, false, 0.0f, 0, 1.5f);
	}

	if (ClimbingDebugState.CurrentHoldCandidate.bIsValid)
	{
		DrawDebugSphere(World, ClimbingDebugState.CurrentHoldCandidate.Location, 10.0f, 12, FColor::Yellow, false, 0.0f);
	}
}

void AClimbingCharacter::SetClimbingState(EClimbingState NewState)
{
	if (ClimbingState == NewState)
	{
		return;
	}

	const EClimbingState PreviousState = ClimbingState;
	ClimbingState = NewState;

	UE_LOG(LogTemp, Log, TEXT("Climbing state changed: %s -> %s"), *UEnum::GetValueAsString(PreviousState), *UEnum::GetValueAsString(ClimbingState));
}
