#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Data/ClimbingHoldTypes.h"
#include "Data/ClimbingLimbTypes.h"
#include "Data/ClimbingMovementTypes.h"
#include "Data/ClimbingTypes.h"
#include "ClimbingCharacter.generated.h"

struct FInputActionValue;
class UClimbingMovementComponent;
class UClimbingHoldQueryComponent;
class UInputAction;
class UInputMappingContext;

UCLASS()
class CLIMB_API AClimbingCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AClimbingCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category="Climbing|State")
	void EnterClimbing();

	UFUNCTION(BlueprintCallable, Category="Climbing|State")
	void EnterClimbingWithAttachment(const FClimbingAttachmentFrame& AttachmentFrame);

	UFUNCTION(BlueprintCallable, Category="Climbing|State")
	void ExitClimbing();

	UFUNCTION(BlueprintCallable, Category="Climbing|State")
	void PrepareClimbingJump();

	UFUNCTION(BlueprintPure, Category="Climbing|State")
	EClimbingState GetClimbingState() const;

	UFUNCTION(BlueprintPure, Category="Climbing|State")
	bool IsClimbing() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Movement")
	UClimbingMovementComponent* GetClimbingMovementComponent() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Hold Query")
	UClimbingHoldQueryComponent* GetHoldQueryComponent() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	FVector2D GetClimbCenterOfMassInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	FVector2D GetClimbLimbProbeInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	float GetLeftGripInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	float GetRightGripInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	float GetLeftFootGripInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Input")
	float GetRightFootGripInput() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Limb")
	FLimbState GetLeftHandState() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Limb")
	FLimbState GetRightHandState() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Limb")
	FLimbState GetLeftFootState() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Limb")
	FLimbState GetRightFootState() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Limb")
	EClimbingLimb GetActiveProbeLimb() const;

	UFUNCTION(BlueprintPure, Category="Climbing|Debug")
	FClimbingDebugState GetClimbingDebugState() const;

	UFUNCTION(BlueprintCallable, Category="Climbing|Limb")
	void TryLockFoot(EClimbingLimb FootLimb);

	UFUNCTION(BlueprintCallable, Category="Climbing|Limb")
	void ReleaseFoot(EClimbingLimb FootLimb);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|State")
	EClimbingState ClimbingState = EClimbingState::Falling;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputMappingContext> ClimbingInputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	int32 ClimbingInputMappingPriority = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbCenterOfMassMoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbLimbProbeAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbLeftGripAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbRightGripAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbLeftFootGripAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Input")
	TObjectPtr<UInputAction> ClimbRightFootGripAction;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	FVector2D ClimbCenterOfMassInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	FVector2D ClimbLimbProbeInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	float LeftGripInput = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	float RightGripInput = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	float LeftFootGripInput = 0.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Input")
	float RightFootGripInput = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Grip", meta=(ClampMin="0.0", ClampMax="1.0"))
	float GripPressedThreshold = 0.35f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Limb")
	FLimbState LeftHandState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Limb")
	FLimbState RightHandState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Limb")
	FLimbState LeftFootState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Limb")
	FLimbState RightFootState;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Limb")
	EClimbingLimb ActiveProbeLimb = EClimbingLimb::RightHand;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="0.0", Units="cm"))
	float MaxCenterOfMassHorizontalOffset = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="0.0", Units="cm"))
	float MaxCenterOfMassVerticalOffset = 80.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug")
	bool bDrawClimbingDebug = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="1.0", Units="cm"))
	float CenterOfMassDebugSphereRadius = 8.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="0.0", Units="cm"))
	float LimbProbeHorizontalRange = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="0.0", Units="cm"))
	float LimbProbeVerticalRange = 120.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="0.0", Units="cm"))
	float LimbProbeWallDepth = 35.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="0.0", Units="cm"))
	float FootProbeForwardOffset = 20.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="0.0", Units="cm"))
	float FootProbeVerticalBias = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="0.0", Units="cm"))
	float HandProbeLateralOffset = 28.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="1.0", Units="cm"))
	float HandExplorationReach = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="1.0", Units="cm"))
	float StableOffsetThreshold = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Climbing|Debug", meta=(ClampMin="1.0", Units="cm"))
	float MaxBodyTensionOffset = 120.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Climbing|Debug")
	FClimbingDebugState ClimbingDebugState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Climbing|Components")
	TObjectPtr<UClimbingHoldQueryComponent> HoldQueryComponent;

private:
	void AddClimbingInputMappingContext() const;
	void BindClimbingInputActions(UInputComponent* PlayerInputComponent);
	void HandleClimbCenterOfMassMove(const FInputActionValue& Value);
	void HandleClimbCenterOfMassMoveCompleted(const FInputActionValue& Value);
	void HandleClimbLimbProbe(const FInputActionValue& Value);
	void HandleClimbLimbProbeCompleted(const FInputActionValue& Value);
	void HandleClimbLeftGrip(const FInputActionValue& Value);
	void HandleClimbLeftGripCompleted(const FInputActionValue& Value);
	void HandleClimbRightGrip(const FInputActionValue& Value);
	void HandleClimbRightGripCompleted(const FInputActionValue& Value);
	void HandleClimbLeftFootGrip(const FInputActionValue& Value);
	void HandleClimbLeftFootGripCompleted(const FInputActionValue& Value);
	void HandleClimbRightFootGrip(const FInputActionValue& Value);
	void HandleClimbRightFootGripCompleted(const FInputActionValue& Value);
	void TryLockLimb(EClimbingLimb Limb);
	void ReleaseLimb(EClimbingLimb Limb);
	bool HasLockedHand() const;
	bool HasLockedFoot() const;
	void RefreshClimbingAttachment();
	void UpdateHandLoadPercentages();
	FLimbState& GetMutableLimbState(EClimbingLimb Limb);
	const FLimbState& GetLimbState(EClimbingLimb Limb) const;
	void ApplyHoldCandidateToLimb(EClimbingLimb Limb, const FClimbingHoldCandidate& Candidate);
	void ClearLimb(EClimbingLimb Limb);
	void RefreshAutomaticActiveHand();
	FClimbingAttachmentFrame BuildAttachmentFrameFromLockedHands() const;
	static void FillWallAxes(FClimbingAttachmentFrame& AttachmentFrame);
	void UpdateClimbingDebugState(float DeltaSeconds);
	void UpdateLimbProbeCandidate(const FClimbingAttachmentFrame& AttachmentFrame);
	bool RefreshProbeCandidateForActiveLimb();
	FVector GetActiveLimbProbeOrigin(const FClimbingAttachmentFrame& AttachmentFrame) const;
	FVector GetActiveLimbProbeTarget(const FClimbingAttachmentFrame& AttachmentFrame, const FVector& ProbeOrigin) const;
	FClimbingAttachmentFrame BuildProbeFrame(const FClimbingAttachmentFrame& AttachmentFrame, const FVector& ProbeOrigin) const;
	void DrawClimbingDebugState() const;
	void SetClimbingState(EClimbingState NewState);
};
