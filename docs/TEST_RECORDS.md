# Test Records

This file records phase gate checks that were completed during project setup and feature work.

## Phase 3 - Hold Query and Sphere Trace

Date: 2026-04-23

Gates checked: `G0`, `G1`, `G3`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding `UClimbingHoldQueryComponent`.
- Static inspection confirmed `AClimbingCharacter` does not contain Sweep/Trace calls.
- Static inspection confirmed Sphere Trace / Sweep logic lives in `UClimbingHoldQueryComponent`.

Manual checks:

- Created `BP_ClimbingHold` test Blueprint.
- Added `ClimbingHold` tag to a test hold Actor.
- Placed tagged hold and untagged test geometry in the ThirdPerson test map.
- Enabled `HoldQueryComponent` debug draw on `BP_ClimbingCharacter`.
- PIE test confirmed the tagged hold is detected.
- PIE test confirmed untagged wall/geometry is rejected.
- Debug draw showed trace path, hit point, and normal direction.

Skipped checks:

- No automated gameplay test exists yet for hold query filtering.

Known risks:

- Hold validity currently depends on the literal `ClimbingHold` Actor or Component tag.
- Collision channel setup is manual and must block the configured query channel.

## Phase 4 - Limb State and Basic Grip Flow

Date: 2026-04-23

Gates checked: `G0`, `G2`, `G3`, `G4`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding `FLimbState` and basic hand grip flow.
- Static inspection confirmed left and right hand states are stored separately.
- Static inspection confirmed hold trace logic remains in `UClimbingHoldQueryComponent`, not in `AClimbingCharacter`.

Manual checks:

- PIE test confirmed L2 can lock the left hand to a valid `ClimbingHold` candidate.
- PIE test confirmed R2 can lock the right hand to a valid `ClimbingHold` candidate.
- PIE test confirmed invalid or untagged wall surfaces do not lock either hand.
- PIE test confirmed releasing one hand clears only that hand state.
- PIE test confirmed releasing the last locked hand exits climbing and enters falling.
- PIE test confirmed two locked hands split load percentage, and the remaining locked hand receives full load after the other hand releases.

Skipped checks:

- No automated gameplay test exists yet for grip state transitions.
- Foot locking is represented in data but not implemented in input flow yet.

Known risks:

- Load distribution is intentionally simple and only supports hand grips at this phase.
- State verification currently relies on Blueprint/debug inspection during PIE.

## Phase 5 - Minimal Climbing Solver

Date: 2026-04-23

Gates checked: `G0`, `G1`, `G5`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding `UClimbingSolver`.
- `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests Climb.Solver; Quit"` completed with exit code `0`.
- Automation log found 4 tests under `Climb.Solver`.
- Automation log reported `Success` for:
  - `Climb.Solver.ProjectCenterOfMassToWallPlane`
  - `Climb.Solver.EstimateTwoPointStability`
  - `Climb.Solver.EstimateBodyTension`
  - `Climb.Solver.CalculateDynoLaunchVelocity`

Manual checks:

- Inspected solver code to confirm it performs no traces, input reads, actor mutation, or animation calls.
- Inspected non-trivial vector calculations for short geometry comments.
- Confirmed edge cases are handled for zero-length contact spans, invalid wall normals, zero/negative time, and launch speed clamping.

Skipped checks:

- No gameplay tuning pass was performed.
- No Control Rig or animation integration was attempted in this phase.

Known risks:

- Solver formulas are deterministic framework estimates, not final climbing-feel tuning.
- Two-point stability currently models the support region as a segment, not a full multi-limb support polygon.

## Phase 6 - Animation and Control Rig Bridge Placeholder

Date: 2026-04-23

Status: passed

Gates checked: `G0`, `G1`, `G6`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding `UClimbingAnimInstance`.
- Static inspection confirmed animation bridge code does not call trace, solver, Control Rig, or climbing state transition APIs.

Manual checks:

- Created `ABP_ClimbingCharacter` derived from `UClimbingAnimInstance`.
- Assigned the climbing Anim Blueprint to `BP_ClimbingCharacter`.
- Confirmed the Anim Blueprint can access exposed bridge fields such as `ClimbingState`, `bIsClimbing`, `LeftHandTarget`, and `RightHandTarget`.
- PIE test confirmed exposed climbing animation fields update when hand grip state changes.
- PIE test confirmed missing Control Rig assets do not crash the character or animation instance.
- Moved climbing character assets under `Content/Climb/Characters/` during editor setup.

Skipped checks:

- No Control Rig variable binding was implemented in this phase.
- No automated animation graph test exists yet.

Known risks:

- `PelvisOffset` is currently a zero placeholder until the procedural Z-axis policy is implemented.
- The bridge exposes target data only; actual FBIK / Control Rig consumption still needs a future asset pass.

## Playable Loop 1 - Stage A - Wall Attachment Constraint

Date: 2026-04-23

Status: passed

Gates checked: `G0`, `G1`, `G3`, `G4`, `G7`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding `FClimbingAttachmentFrame` and movement attachment constraints.
- Static inspection confirmed movement attachment and target correction live in `UClimbingMovementComponent`.
- Static inspection confirmed `AClimbingCharacter` still delegates hold query to `UClimbingHoldQueryComponent` and does not own Sweep / Trace implementation.

Manual checks:

- In PIE, grab a tagged `ClimbingHold` with L2 and confirm the character does not fall while the left hand remains locked.
- Repeat with R2 and confirm the right hand can attach independently.
- Try an untagged wall and confirm the character does not enter climbing.
- Release the final locked hand and confirm the character enters Falling and drops.
- Adjusted and verified the third-person camera setup on `BP_ClimbingCharacter` during manual testing.

Skipped checks:

- No automated gameplay movement test exists yet for climbing attachment.

Known risks:

- Attachment target currently uses locked hand contact points and wall normals directly; final body spacing will need tuning once CoM and animation offsets are active.
- The Blueprint-exposed attachment entry point is named `StartClimbingMovementWithAttachment` because Unreal Header Tool does not allow two reflected `StartClimbingMovement` overloads.
