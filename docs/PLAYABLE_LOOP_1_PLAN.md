# Playable Loop 1 Plan

This plan starts after the completed Phase 0-6 base framework. The goal is the first playable climbing loop, not final climbing physics, full Control Rig integration, footwork, Dyno, stamina, or resonance swing.

## Current Status

Stages A-D are passed and documented in `docs/TEST_RECORDS.md`.

Stage E implementation and command verification are complete, but its final manual editor verification is still pending. Until that manual check is recorded as passed, Playable Loop 1 is not formally closed.

Work such as Control Rig FBIK tuning, foot gameplay flow, probe refinement, or exploration IK is outside the original scope of this plan and must be tracked in `docs/POST_PLAYABLE_LOOP_PLAN.md` instead of being treated as additional Playable Loop 1 stages.

## Summary

Playable Loop 1 must let the player grab a tagged hold, stay attached to the wall, move a center-of-mass debug target with the left stick, probe for another hold with the right stick, transfer between left and right hand holds, and inspect Body Tension debug values.

Every stage must update `docs/TEST_RECORDS.md` and map to `docs/GATES.md`.

## Stage A - Wall Attachment Constraint

Goal: A valid hand grip should put the character into a real climbing movement constraint instead of only changing state.

Implementation requirements:

- Add `FClimbingAttachmentFrame` with `ContactLocation`, `WallNormal`, `AnchorLocation`, `WallRight`, `WallUp`, and `TargetWallDistance`.
- Extend `UClimbingMovementComponent` with stored attachment frame data, `AttachInterpSpeed`, and `TargetWallDistance`.
- Keep the existing no-argument `StartClimbingMovement()` for compatibility.
- Add `StartClimbingMovement(const FClimbingAttachmentFrame& AttachmentFrame)` and `UpdateClimbingAttachmentFrame(const FClimbingAttachmentFrame& AttachmentFrame)`.
- In climbing custom movement, zero falling velocity and move the character toward the attachment frame target.
- `StopClimbingMovement()` should return to `MOVE_Falling` and should not clear limb state.

Acceptance:

- Grabbing a valid `ClimbingHold` enters `MOVE_Custom` with the climbing custom mode.
- The character no longer drops while at least one hand remains locked.
- Releasing the final locked hand returns to `MOVE_Falling`.
- Invalid or untagged surfaces do not attach.

Test checklist:

- Build `ClimbEditor Win64 Development`.
- In PIE, grab a tagged hold with L2 and confirm the character does not fall.
- Repeat with R2.
- Try an untagged wall and confirm the character does not enter climbing.
- Release the final locked hand and confirm falling resumes.
- Inspect code to confirm wall constraint logic lives in `UClimbingMovementComponent`.

Mapped gates: `G0`, `G1`, `G3`, `G4`, `G7`.

## Stage B - Center-of-Mass Debug Offset

Goal: The left stick should drive a constrained CoM / pelvis target on the wall plane.

Implementation requirements:

- Add a `FClimbingDebugState` field readable from `AClimbingCharacter`.
- Add `CenterOfMassTargetOffset` and `CenterOfMassTarget` debug values.
- Clamp local wall-plane offset to `60cm` horizontal and `80cm` vertical by default.
- Compute the target from the current attachment frame axes.
- Feed the resulting offset into `UClimbingAnimInstance::PelvisOffset`.
- Draw a debug point or sphere while climbing when debug draw is enabled.

Acceptance:

- In climbing state, left stick moves a debug CoM target left/right/up/down on the wall plane.
- Releasing the stick resets or smoothly recenters the CoM target.
- CoM offset does not grow unbounded and does not use world axes when wall axes are available.

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Grab a valid hold and move the left stick.
- Confirm the debug CoM target moves in wall-local right/up axes.
- Confirm max offset clamps at approximately `60cm` horizontal and `80cm` vertical.
- Confirm `PelvisOffset` updates in `ABP_ClimbingCharacter`.

Mapped gates: `G0`, `G1`, `G2`, `G6`, `G7`.

## Stage C - Right-Stick Limb Probe

Goal: The right stick should aim the active hand probe across nearby wall space.

Implementation requirements:

- Keep Sphere Trace implementation in `UClimbingHoldQueryComponent`.
- Character computes probe origin and direction only.
- Right-stick input offsets the probe direction or wall-plane target from the active hand / camera basis.
- Store the latest candidate hold in a debug-readable field.
- Draw trace, candidate point, and rejected hit feedback when debug draw is enabled.

Acceptance:

- In climbing state, right stick changes which hold candidate is selected.
- Tagged `ClimbingHold` actors/components are valid candidates.
- Untagged wall hits are rejected.
- Character code does not directly call `SweepSingleByChannel` or own trace filtering.

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Place two tagged holds near the character and one untagged wall target.
- Grab one hold, move the right stick toward the second hold, and confirm the candidate changes.
- Move the probe toward the untagged wall and confirm it is rejected.
- Static inspect trace ownership.

Mapped gates: `G0`, `G1`, `G2`, `G3`, `G7`.

## Stage D - Basic Hand Transfer

Goal: The player can move from one hand hold to another without dropping as long as one hand remains locked.

Implementation requirements:

- L2 controls only left hand; R2 controls only right hand.
- Pressing a hand grip attempts to lock that hand to the current valid candidate.
- Releasing a hand clears only that hand state.
- If both hands are locked, load should be approximately `0.5 / 0.5`.
- If one hand remains locked, load should become `1.0` on the remaining hand.
- The active attachment frame should use the single locked hand or the average of both locked hand contacts.

Acceptance:

- Left hand can lock hold A and right hand can lock hold B.
- Releasing left hand while right hand remains locked does not enter falling.
- Releasing the final locked hand enters falling.
- Load percentages and animation targets remain consistent with limb state.

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Grab hold A with L2.
- Probe and grab hold B with R2.
- Confirm both hand targets are locked and load is split.
- Release L2 and confirm right hand remains locked with full load.
- Release R2 and confirm falling resumes.

Mapped gates: `G0`, `G1`, `G2`, `G3`, `G4`, `G6`, `G7`.

## Stage E - Body Tension Debug Integration

Goal: Use the pure solver to expose initial stability and Body Tension debug values without forcing failure behavior yet.

Implementation requirements:

- Use `UClimbingSolver::EstimateTwoPointStability` and `UClimbingSolver::EstimateBodyTension`.
- Add `CurrentBodyTension`, `StabilityPercent`, and `bIsPoseStable` to `FClimbingDebugState`.
- Use the current CoM target and locked hand contacts as solver inputs.
- Handle missing or single-contact cases with stable fallback values and no NaN / Inf.
- Do not force drops from high tension in this stage; show warning/debug color only.

Acceptance:

- With two locked hands, moving CoM away from the support line increases Body Tension.
- Stability percent remains finite and predictable.
- Solver remains pure and does not read input, trace, mutate actors, or call animation code.

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Run `Climb.Solver` automation tests.
- Grab two holds and move CoM with the left stick.
- Confirm Body Tension rises as CoM moves away from the hand support line.
- Confirm debug values remain finite with one hand, two hands, and no hands.
- Static inspect solver boundaries.

Mapped gates: `G0`, `G1`, `G4`, `G5`, `G6`, `G7`.

## Completion Policy

- A stage is incomplete until its mapped gates pass and `docs/TEST_RECORDS.md` is updated.
- Build verification is required for every stage.
- Manual PIE checks are required for every stage because this is a playable loop.
- Advanced mechanics are out of scope for this plan: foot grip, Dyno, resonance swing, stamina, final FBIK, and polished animation.
