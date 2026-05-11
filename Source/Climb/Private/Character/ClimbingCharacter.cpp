#include "Character/ClimbingCharacter.h"

#include "Components/ClimbingHoldQueryComponent.h"
#include "Components/ClimbingMovementComponent.h"
#include "Solvers/ClimbingSolver.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/CapsuleComponent.h"
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
	LeftHandProbeSettings.HorizontalRange = 55.0f;
	LeftHandProbeSettings.VerticalRange = 70.0f;
	LeftHandProbeSettings.ForwardOffset = 18.0f;
	LeftHandProbeSettings.VerticalOffset = 8.0f;
	LeftHandProbeSettings.SearchDepth = 36.0f;
	RightHandProbeSettings = LeftHandProbeSettings;
	LeftFootProbeSettings.HorizontalRange = 35.0f;
	LeftFootProbeSettings.VerticalRange = 45.0f;
	LeftFootProbeSettings.ForwardOffset = 20.0f;
	LeftFootProbeSettings.SearchDepth = 35.0f;
	RightFootProbeSettings = LeftFootProbeSettings;
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
	UpdateLimbGripState(EClimbingLimb::LeftHand, Value.Get<float>(), LeftGripInput, LeftGripPressStartTime, bLeftGripLockTriggered);
}

void AClimbingCharacter::HandleClimbLeftGripCompleted(const FInputActionValue& Value)
{
	CompleteLimbGripState(EClimbingLimb::LeftHand, LeftGripInput, LeftGripPressStartTime, bLeftGripLockTriggered);
}

void AClimbingCharacter::HandleClimbRightGrip(const FInputActionValue& Value)
{
	UpdateLimbGripState(EClimbingLimb::RightHand, Value.Get<float>(), RightGripInput, RightGripPressStartTime, bRightGripLockTriggered);
}

void AClimbingCharacter::HandleClimbRightGripCompleted(const FInputActionValue& Value)
{
	CompleteLimbGripState(EClimbingLimb::RightHand, RightGripInput, RightGripPressStartTime, bRightGripLockTriggered);
}

void AClimbingCharacter::HandleClimbLeftFootGrip(const FInputActionValue& Value)
{
	UpdateLimbGripState(EClimbingLimb::LeftFoot, Value.Get<float>(), LeftFootGripInput, LeftFootGripPressStartTime, bLeftFootGripLockTriggered);
}

void AClimbingCharacter::HandleClimbLeftFootGripCompleted(const FInputActionValue& Value)
{
	CompleteLimbGripState(EClimbingLimb::LeftFoot, LeftFootGripInput, LeftFootGripPressStartTime, bLeftFootGripLockTriggered);
}

void AClimbingCharacter::HandleClimbRightFootGrip(const FInputActionValue& Value)
{
	UpdateLimbGripState(EClimbingLimb::RightFoot, Value.Get<float>(), RightFootGripInput, RightFootGripPressStartTime, bRightFootGripLockTriggered);
}

void AClimbingCharacter::HandleClimbRightFootGripCompleted(const FInputActionValue& Value)
{
	CompleteLimbGripState(EClimbingLimb::RightFoot, RightFootGripInput, RightFootGripPressStartTime, bRightFootGripLockTriggered);
}

void AClimbingCharacter::UpdateLimbGripState(EClimbingLimb Limb, float NewInputValue, float& StoredInputValue, float& PressStartTime, bool& bLockTriggered)
{
	const bool bWasPressed = StoredInputValue >= GripPressedThreshold;
	StoredInputValue = NewInputValue;
	const bool bIsPressed = StoredInputValue >= GripPressedThreshold;
	if (!bIsPressed)
	{
		return;
	}

	if (!bWasPressed)
	{
		ActiveProbeLimb = Limb;
		PressStartTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
		bLockTriggered = false;
		return;
	}

	if (bLockTriggered)
	{
		return;
	}

	const float CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	if (CurrentTimeSeconds - PressStartTime < GripHoldLockDelaySeconds)
	{
		return;
	}

	TryLockLimb(Limb);
	bLockTriggered = GetLimbState(Limb).bIsLocked;
}

void AClimbingCharacter::CompleteLimbGripState(EClimbingLimb Limb, float& StoredInputValue, float& PressStartTime, bool& bLockTriggered)
{
	StoredInputValue = 0.0f;
	PressStartTime = -1.0f;
	const bool bWasLocked = GetLimbState(Limb).bIsLocked;
	bLockTriggered = false;

	if (bWasLocked)
	{
		ReleaseLimb(Limb);
	}
}

void AClimbingCharacter::TryLockLimb(EClimbingLimb Limb)
{
	ActiveProbeLimb = Limb;

	FClimbingHoldCandidate Candidate;
	if (RefreshProbeCandidateForActiveLimb())
	{
		Candidate = ClimbingDebugState.CurrentHoldCandidate;
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

	if (bWasLocked && IsClimbing())
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

int32 AClimbingCharacter::GetLockedSupportCount() const
{
	return
		(LeftHandState.bIsLocked ? 1 : 0) +
		(RightHandState.bIsLocked ? 1 : 0) +
		(LeftFootState.bIsLocked ? 1 : 0) +
		(RightFootState.bIsLocked ? 1 : 0);
}

FClimbingAttachmentFrame AClimbingCharacter::BuildAttachmentAuthorityFrame() const
{
	const FClimbingAttachmentFrame HandAttachmentFrame = BuildAttachmentFrameFromLockedHands();
	const FClimbingAttachmentFrame SupportFrame = BuildSupportFrameFromLockedLimbs(HandAttachmentFrame);

	if (!HasLockedHand())
	{
		return FClimbingAttachmentFrame();
	}

	return HasLockedFoot() ? SupportFrame : HandAttachmentFrame;
}

void AClimbingCharacter::RefreshClimbingAttachment()
{
	const FClimbingAttachmentFrame AttachmentFrame = BuildAttachmentAuthorityFrame();
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

FClimbingAttachmentFrame AClimbingCharacter::BuildSupportFrameFromLockedLimbs(const FClimbingAttachmentFrame& FallbackAttachmentFrame) const
{
	FClimbingAttachmentFrame SupportFrame = FallbackAttachmentFrame;
	FVector AccumulatedLocation = FVector::ZeroVector;
	FVector AccumulatedNormal = FVector::ZeroVector;
	int32 LockedSupportCount = 0;

	const auto AccumulateLockedLimb = [&](const FLimbState& LimbState)
	{
		if (!LimbState.bIsLocked)
		{
			return;
		}

		AccumulatedLocation += LimbState.ContactLocation;
		AccumulatedNormal += LimbState.ContactNormal.GetSafeNormal();
		++LockedSupportCount;
	};

	AccumulateLockedLimb(LeftHandState);
	AccumulateLockedLimb(RightHandState);
	AccumulateLockedLimb(LeftFootState);
	AccumulateLockedLimb(RightFootState);

	if (LockedSupportCount == 0)
	{
		return FallbackAttachmentFrame;
	}

	SupportFrame.bIsValid = true;
	SupportFrame.AnchorLocation = AccumulatedLocation / LockedSupportCount;
	SupportFrame.ContactLocation = SupportFrame.AnchorLocation;
	SupportFrame.WallNormal = AccumulatedNormal.GetSafeNormal();
	if (SupportFrame.WallNormal.IsNearlyZero())
	{
		SupportFrame.WallNormal = FallbackAttachmentFrame.bIsValid
			? FallbackAttachmentFrame.WallNormal.GetSafeNormal()
			: (-GetActorForwardVector()).GetSafeNormal();
	}

	FillWallAxes(SupportFrame);
	return SupportFrame;
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
	const FClimbingAttachmentFrame MovementAttachmentFrame = ClimbingMovement ? ClimbingMovement->GetClimbingAttachmentFrame() : FClimbingAttachmentFrame();
	if (!MovementAttachmentFrame.bIsValid)
	{
		ClimbingDebugState = FClimbingDebugState();
	}

	const bool bHasClimbingAttachment = IsClimbing() && MovementAttachmentFrame.bIsValid;
	const FClimbingAttachmentFrame SupportFrame = bHasClimbingAttachment
		? BuildSupportFrameFromLockedLimbs(MovementAttachmentFrame)
		: FClimbingAttachmentFrame();
	const FClimbingAttachmentFrame ProbeAttachmentFrame = MovementAttachmentFrame.bIsValid
		? MovementAttachmentFrame
		: FClimbingAttachmentFrame();

	if (bHasClimbingAttachment)
	{
		const FVector WallRight = SupportFrame.WallRight.GetSafeNormal();
		const FVector WallUp = SupportFrame.WallUp.GetSafeNormal();
		const FVector2D ClampedInput(
			FMath::Clamp(ClimbCenterOfMassInput.X, -1.0f, 1.0f),
			FMath::Clamp(ClimbCenterOfMassInput.Y, -1.0f, 1.0f));

		// CoM debug motion is constrained to the wall tangent plane using the attachment frame right/up axes.
		ClimbingDebugState.CenterOfMassTargetOffset =
			WallRight * (ClampedInput.X * MaxCenterOfMassHorizontalOffset) +
			WallUp * (ClampedInput.Y * MaxCenterOfMassVerticalOffset);
		ClimbingDebugState.CenterOfMassTarget =
			SupportFrame.AnchorLocation +
			SupportFrame.WallNormal.GetSafeNormal() * SupportFrame.TargetWallDistance +
			ClimbingDebugState.CenterOfMassTargetOffset;

		TArray<FVector> LockedSupportContacts;
		LockedSupportContacts.Reserve(4);
		const auto AddLockedSupportContact = [&](const FLimbState& LimbState)
		{
			if (LimbState.bIsLocked)
			{
				LockedSupportContacts.Add(LimbState.ContactLocation);
			}
		};

		AddLockedSupportContact(LeftHandState);
		AddLockedSupportContact(RightHandState);
		AddLockedSupportContact(LeftFootState);
		AddLockedSupportContact(RightFootState);

		if (LockedSupportContacts.Num() >= 2)
		{
			const FClimbingStabilityResult Stability = UClimbingSolver::EstimateMultiContactStability(
				ClimbingDebugState.CenterOfMassTarget,
				LockedSupportContacts,
				SupportFrame.WallNormal,
				StableOffsetThreshold);

			ClimbingDebugState.CurrentBodyTension = UClimbingSolver::EstimateMultiContactBodyTension(
				ClimbingDebugState.CenterOfMassTarget,
				LockedSupportContacts,
				SupportFrame.WallNormal,
				MaxBodyTensionOffset);
			ClimbingDebugState.StabilityPercent = Stability.StabilityPercent;
			ClimbingDebugState.bIsPoseStable = Stability.bIsStable;
		}
		else
		{
			ClimbingDebugState.CurrentBodyTension = 0.0f;
			ClimbingDebugState.StabilityPercent = LockedSupportContacts.Num() == 1 ? 1.0f : 0.0f;
			ClimbingDebugState.bIsPoseStable = LockedSupportContacts.Num() == 1;
		}
	}
	else
	{
		ClimbingDebugState.CenterOfMassTargetOffset = FVector::ZeroVector;
		ClimbingDebugState.CenterOfMassTarget = FVector::ZeroVector;
		ClimbingDebugState.CurrentBodyTension = 0.0f;
		ClimbingDebugState.StabilityPercent = 0.0f;
		ClimbingDebugState.bIsPoseStable = false;
	}

	UpdateLimbProbeCandidate(ProbeAttachmentFrame);
	const FVector ProbeTarget = GetActiveLimbProbeTarget(ProbeAttachmentFrame);

	const bool bHasProbeInput = !ClimbLimbProbeInput.IsNearlyZero(0.05f);
	if (!GetLimbState(ActiveProbeLimb).bIsLocked && bHasProbeInput)
	{
		ClimbingDebugState.bHasActiveExplorationTarget = true;
		// Exploration always follows the reach-plane target. A yellow hold candidate only gates
		// whether a long-press can commit a lock; it does not override the free steering pose.
		ClimbingDebugState.ActiveExplorationTargetLocation = ProbeTarget;
		ClimbingDebugState.ActiveExplorationTargetNormal = (-ProbeAttachmentFrame.WallNormal).GetSafeNormal();
	}
	else
	{
		ClimbingDebugState.bHasActiveExplorationTarget = false;
		ClimbingDebugState.ActiveExplorationTargetLocation = FVector::ZeroVector;
		ClimbingDebugState.ActiveExplorationTargetNormal = FVector::ZeroVector;
	}

	DrawClimbingDebugState();
}

void AClimbingCharacter::UpdateLimbProbeCandidate(const FClimbingAttachmentFrame& AttachmentFrame)
{
	ClimbingDebugState.ProbeOrigin = GetActiveLimbProbeOrigin(AttachmentFrame);
	const FVector ProbeTarget = GetActiveLimbProbeTarget(AttachmentFrame);
	ClimbingDebugState.ProbeDirection = (ProbeTarget - ClimbingDebugState.ProbeOrigin).GetSafeNormal();

	FClimbingHoldCandidate Candidate;
	if (!HoldQueryComponent)
	{
		ClimbingDebugState.CurrentHoldCandidate = FClimbingHoldCandidate();
		return;
	}

	if (!GetLimbState(ActiveProbeLimb).bIsLocked)
	{
		const FClimbingAttachmentFrame ProbeFrame = BuildProbeFrame(AttachmentFrame, ClimbingDebugState.ProbeOrigin);
		// Unlocked limb queries search around the wall-local reach-plane target instead of using a long aim-like ray.
		const FVector SearchDirection = -ProbeFrame.WallNormal.GetSafeNormal();
		if (HoldQueryComponent->QueryBestHoldNearPoint(ProbeTarget, SearchDirection, GetProbePlaneSettings(ActiveProbeLimb).SearchDepth, Candidate))
		{
			ClimbingDebugState.CurrentHoldCandidate = Candidate;
			return;
		}
	}
	else if (HoldQueryComponent->QueryBestHold(ClimbingDebugState.ProbeOrigin, ClimbingDebugState.ProbeDirection, Candidate))
	{
		ClimbingDebugState.CurrentHoldCandidate = Candidate;
		return;
	}

	ClimbingDebugState.CurrentHoldCandidate = FClimbingHoldCandidate();
}

bool AClimbingCharacter::RefreshProbeCandidateForActiveLimb()
{
	const UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent();
	const FClimbingAttachmentFrame MovementAttachmentFrame = ClimbingMovement ? ClimbingMovement->GetClimbingAttachmentFrame() : FClimbingAttachmentFrame();

	UpdateLimbProbeCandidate(MovementAttachmentFrame);
	return ClimbingDebugState.CurrentHoldCandidate.bIsValid;
}

const FClimbingProbePlaneSettings& AClimbingCharacter::GetProbePlaneSettings(EClimbingLimb Limb) const
{
	switch (Limb)
	{
	case EClimbingLimb::LeftHand:
		return LeftHandProbeSettings;
	case EClimbingLimb::RightHand:
		return RightHandProbeSettings;
	case EClimbingLimb::LeftFoot:
		return LeftFootProbeSettings;
	case EClimbingLimb::RightFoot:
	default:
		return RightFootProbeSettings;
	}
}

FVector AClimbingCharacter::GetStableProbeBodyAnchor(EClimbingLimb Limb) const
{
	const float CapsuleHalfHeight = GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 88.0f;
	const FVector ActorOrigin = GetActorLocation();
	const FVector ActorUp = GetActorUpVector().GetSafeNormal();

	if (Limb == EClimbingLimb::LeftHand || Limb == EClimbingLimb::RightHand)
	{
		// Hand work areas should stay anchored around the upper torso/chest instead of drifting with
		// current support contacts. This keeps activation and locked-support states using the same
		// stable body-facing plane.
		return ActorOrigin + ActorUp * (CapsuleHalfHeight * 0.35f);
	}

	// Feet explore from a stable lower-body anchor so support contacts do not drag the stance plane.
	return ActorOrigin - ActorUp * (CapsuleHalfHeight * 0.35f);
}

FVector AClimbingCharacter::GetActiveLimbProbeCenter(const FClimbingAttachmentFrame& AttachmentFrame) const
{
	const FVector BodyAnchor = GetStableProbeBodyAnchor(ActiveProbeLimb);
	const FClimbingAttachmentFrame ProbeFrame = BuildProbeFrame(AttachmentFrame, BodyAnchor);
	const FClimbingProbePlaneSettings& Settings = GetProbePlaneSettings(ActiveProbeLimb);
	FVector ProbeCenter = BodyAnchor;

	if (ActiveProbeLimb == EClimbingLimb::LeftHand || ActiveProbeLimb == EClimbingLimb::RightHand)
	{
		// Hand reach planes stay anchored in front of the upper body so the work area is stable while the limb explores inside it.
		ProbeCenter += ProbeFrame.WallRight.GetSafeNormal() * Settings.LateralOffset;
		ProbeCenter += ProbeFrame.WallUp.GetSafeNormal() * Settings.VerticalOffset;
		ProbeCenter += -ProbeFrame.WallNormal.GetSafeNormal() * Settings.ForwardOffset;
		return ProbeCenter;
	}

	if (ActiveProbeLimb == EClimbingLimb::LeftFoot || ActiveProbeLimb == EClimbingLimb::RightFoot)
	{
		// Foot reach planes stay anchored below the body so left/right feet explore a stable stance area instead of dragging the plane with the foot.
		ProbeCenter += ProbeFrame.WallRight.GetSafeNormal() * Settings.LateralOffset;
		ProbeCenter += ProbeFrame.WallUp.GetSafeNormal() * Settings.VerticalOffset;
		ProbeCenter += -ProbeFrame.WallNormal.GetSafeNormal() * Settings.ForwardOffset;
	}

	return ProbeCenter;
}

FVector AClimbingCharacter::GetActiveLimbProbeOrigin(const FClimbingAttachmentFrame& AttachmentFrame) const
{
	const FLimbState& ActiveLimbState = GetLimbState(ActiveProbeLimb);
	if (ActiveLimbState.bIsLocked || ActiveLimbState.bHasValidContact)
	{
		return ActiveLimbState.ContactLocation + AttachmentFrame.WallNormal.GetSafeNormal() * AttachmentFrame.TargetWallDistance;
	}

	const FVector ProbeCenter = GetActiveLimbProbeCenter(AttachmentFrame);
	const FClimbingAttachmentFrame ProbeFrame = BuildProbeFrame(AttachmentFrame, ProbeCenter);
	return ProbeCenter + ProbeFrame.WallNormal.GetSafeNormal() * ProbeFrame.TargetWallDistance;
}

FVector AClimbingCharacter::GetActiveLimbProbeTarget(const FClimbingAttachmentFrame& AttachmentFrame) const
{
	const FVector ProbeCenter = GetActiveLimbProbeCenter(AttachmentFrame);
	const FClimbingAttachmentFrame ProbeFrame = BuildProbeFrame(AttachmentFrame, ProbeCenter);
	const FClimbingProbePlaneSettings& Settings = GetProbePlaneSettings(ActiveProbeLimb);
	const FVector ProbeOffset =
		ProbeFrame.WallRight.GetSafeNormal() * (ClimbLimbProbeInput.X * Settings.HorizontalRange) +
		ProbeFrame.WallUp.GetSafeNormal() * (ClimbLimbProbeInput.Y * Settings.VerticalRange);

	// The plane target lives on the reach plane. Near-point hold queries then search inward
	// from this point along the wall normal instead of baking search depth into the target itself.
	return ProbeCenter + ProbeOffset;
}

FClimbingAttachmentFrame AClimbingCharacter::BuildProbeFrame(const FClimbingAttachmentFrame& AttachmentFrame, const FVector& ProbeOrigin) const
{
	FClimbingAttachmentFrame ProbeFrame;
	ProbeFrame.bIsValid = true;
	ProbeFrame.AnchorLocation = ProbeOrigin;
	ProbeFrame.ContactLocation = ProbeFrame.AnchorLocation;
	ProbeFrame.WallNormal = -GetActorForwardVector().GetSafeNormal();
	if (ProbeFrame.WallNormal.IsNearlyZero())
	{
		ProbeFrame.WallNormal = AttachmentFrame.bIsValid
			? AttachmentFrame.WallNormal.GetSafeNormal()
			: FVector::ForwardVector;
	}

	// Probe planes should remain body-facing and stable regardless of which limbs are currently
	// locked. Use actor orientation for the basis, and only keep movement distance from the current
	// attachment data.
	const FVector FacingRightOnWall = FVector::VectorPlaneProject(GetActorRightVector(), ProbeFrame.WallNormal).GetSafeNormal();
	if (!FacingRightOnWall.IsNearlyZero())
	{
		ProbeFrame.WallRight = FacingRightOnWall;
		ProbeFrame.WallUp = FVector::CrossProduct(ProbeFrame.WallNormal, ProbeFrame.WallRight).GetSafeNormal();
		if (ProbeFrame.WallUp.IsNearlyZero())
		{
			ProbeFrame.WallUp = FVector::UpVector;
		}
	}
	else
	{
		FillWallAxes(ProbeFrame);
	}

	ProbeFrame.TargetWallDistance = AttachmentFrame.bIsValid && AttachmentFrame.TargetWallDistance > 0.0f
		? AttachmentFrame.TargetWallDistance
		: 45.0f;
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

	const UClimbingMovementComponent* ClimbingMovement = GetClimbingMovementComponent();
	const FClimbingAttachmentFrame MovementAttachmentFrame = ClimbingMovement ? ClimbingMovement->GetClimbingAttachmentFrame() : FClimbingAttachmentFrame();
	if (MovementAttachmentFrame.bIsValid)
	{
		const FVector ProbeCenter = GetActiveLimbProbeCenter(MovementAttachmentFrame);
		const FClimbingAttachmentFrame ProbeFrame = BuildProbeFrame(MovementAttachmentFrame, ProbeCenter);
		const FVector ProbeTarget = GetActiveLimbProbeTarget(MovementAttachmentFrame);
		const FVector Right = ProbeFrame.WallRight.GetSafeNormal();
		const FVector Up = ProbeFrame.WallUp.GetSafeNormal();
		const FClimbingProbePlaneSettings& Settings = GetProbePlaneSettings(ActiveProbeLimb);
		const FVector HalfRight = Right * Settings.HorizontalRange;
		const FVector HalfUp = Up * Settings.VerticalRange;
		const int32 SegmentCount = 24;
		FVector PreviousPoint = ProbeCenter + Right * HalfRight.Size();
		for (int32 SegmentIndex = 1; SegmentIndex <= SegmentCount; ++SegmentIndex)
		{
			const float AngleRadians = (2.0f * PI * SegmentIndex) / SegmentCount;
			const FVector CurrentPoint =
				ProbeCenter +
				Right * (FMath::Cos(AngleRadians) * HalfRight.Size()) +
				Up * (FMath::Sin(AngleRadians) * HalfUp.Size());
			DrawDebugLine(World, PreviousPoint, CurrentPoint, FColor::Purple, false, 0.0f, 0, 1.0f);
			PreviousPoint = CurrentPoint;
		}
		DrawDebugLine(World, ProbeCenter - HalfRight, ProbeCenter + HalfRight, FColor::Purple, false, 0.0f, 0, 0.75f);
		DrawDebugLine(World, ProbeCenter - HalfUp, ProbeCenter + HalfUp, FColor::Purple, false, 0.0f, 0, 0.75f);
		DrawDebugSphere(World, ProbeCenter, 6.0f, 10, FColor::Purple, false, 0.0f);
		DrawDebugSphere(World, ProbeTarget, 7.0f, 10, FColor::Magenta, false, 0.0f);
		DrawDebugLine(World, ProbeCenter, ProbeTarget, FColor::Magenta, false, 0.0f, 0, 1.0f);
	}
	else
	{
		const FClimbingAttachmentFrame SupportFrame;
		const FVector ProbeCenter = GetActiveLimbProbeCenter(SupportFrame);
		const FClimbingAttachmentFrame ProbeFrame = BuildProbeFrame(SupportFrame, ProbeCenter);
		const FVector ProbeTarget = GetActiveLimbProbeTarget(SupportFrame);
		const FVector Right = ProbeFrame.WallRight.GetSafeNormal();
		const FVector Up = ProbeFrame.WallUp.GetSafeNormal();
		const FClimbingProbePlaneSettings& Settings = GetProbePlaneSettings(ActiveProbeLimb);
		const FVector HalfRight = Right * Settings.HorizontalRange;
		const FVector HalfUp = Up * Settings.VerticalRange;
		const int32 SegmentCount = 24;
		FVector PreviousPoint = ProbeCenter + Right * HalfRight.Size();
		for (int32 SegmentIndex = 1; SegmentIndex <= SegmentCount; ++SegmentIndex)
		{
			const float AngleRadians = (2.0f * PI * SegmentIndex) / SegmentCount;
			const FVector CurrentPoint =
				ProbeCenter +
				Right * (FMath::Cos(AngleRadians) * HalfRight.Size()) +
				Up * (FMath::Sin(AngleRadians) * HalfUp.Size());
			DrawDebugLine(World, PreviousPoint, CurrentPoint, FColor::Purple, false, 0.0f, 0, 1.0f);
			PreviousPoint = CurrentPoint;
		}
		DrawDebugLine(World, ProbeCenter - HalfRight, ProbeCenter + HalfRight, FColor::Purple, false, 0.0f, 0, 0.75f);
		DrawDebugLine(World, ProbeCenter - HalfUp, ProbeCenter + HalfUp, FColor::Purple, false, 0.0f, 0, 0.75f);
		DrawDebugSphere(World, ProbeCenter, 6.0f, 10, FColor::Purple, false, 0.0f);
		DrawDebugSphere(World, ProbeTarget, 7.0f, 10, FColor::Magenta, false, 0.0f);
		DrawDebugLine(World, ProbeCenter, ProbeTarget, FColor::Magenta, false, 0.0f, 0, 1.0f);
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
