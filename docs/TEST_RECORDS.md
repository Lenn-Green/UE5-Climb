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

## Playable Loop 1 - Stage B - Center-of-Mass Debug Offset

Date: 2026-04-23

Status: passed

Gates checked: `G0`, `G1`, `G2`, `G6`, `G7`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding `FClimbingDebugState` and center-of-mass debug target calculation.
- Static inspection confirmed `UClimbingAnimInstance` only copies the C++ debug offset into `PelvisOffset` and does not perform trace, solver, movement, or state transition logic.
- Static inspection confirmed `AClimbingCharacter` still does not own Sweep / Trace implementation.

Manual checks:

- In PIE, grab a valid hold and move the left stick.
- Confirm the cyan CoM debug sphere moves in the wall-local right/up plane.
- Confirm horizontal offset clamps at about `60cm` and vertical offset clamps at about `80cm`.
- Confirm releasing the stick returns the CoM debug sphere to the attachment center.
- Confirm `ABP_ClimbingCharacter` can read the updated `PelvisOffset`.
- Added temporary keyboard mappings in `IMC_Climbing` to test center-of-mass movement without a gamepad.

Skipped checks:

- No automated gameplay test exists yet for CoM debug target movement.

Known risks:

- `PelvisOffset` is a raw world-space debug offset for now; final Control Rig consumption may need local-space conversion.
- CoM target currently snaps to input values rather than using a smoothing model.

## Playable Loop 1 - Stage C - Right-Stick Limb Probe

Date: 2026-04-23

Status: passed

Gates checked: `G0`, `G1`, `G2`, `G3`, `G7`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding limb probe candidate caching and wall-plane probe direction logic.
- Static inspection confirmed `AClimbingCharacter` computes probe origin and direction only; it still does not own Sweep / Trace implementation.
- Static inspection confirmed `UClimbingAnimInstance` still does not perform trace, movement, solver, or gameplay state logic.

Manual checks:

- In PIE, enter climbing on a tagged hold and move the right-stick substitute input.
- Confirm the orange probe line changes direction while climbing.
- Confirm a valid nearby `ClimbingHold` becomes the current candidate and shows a yellow marker.
- Confirm probe movement toward an untagged wall does not produce a valid candidate.
- Confirm pressing the opposite hand grip while climbing uses the current candidate instead of the fallback viewpoint query.
- Added temporary keyboard mappings in `IMC_Climbing` to test limb probe movement without a gamepad.

Skipped checks:

- No automated gameplay test exists yet for probe candidate selection.

Known risks:

- Probe targeting currently uses wall-plane offsets from the current attachment frame and active hand contact; later tuning may need camera influence for better feel.
- Candidate selection still uses simple distance score from `UClimbingHoldQueryComponent` and does not yet consider reachability or body tension.

## Playable Loop 1 - Stage D - Basic Hand Transfer

Date: 2026-04-23

Status: passed

Gates checked: `G0`, `G1`, `G2`, `G3`, `G4`, `G6`, `G7`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding attachment refresh for dual-hand transfer and single-hand release.
- Static inspection confirmed `AClimbingCharacter` still does not own Sweep / Trace implementation.
- Static inspection confirmed `UClimbingAnimInstance` still does not perform gameplay state, movement, or hold query logic.

Manual checks:

- In PIE, lock hold A with the left hand.
- Use limb probe input to select hold B and lock it with the right hand.
- Confirm both hands stay locked and load is approximately `0.5 / 0.5`.
- Release the left hand and confirm the right hand remains locked, load becomes `1.0`, and the character stays in `Climbing`.
- Release the final locked hand and confirm the character returns to `Falling`.
- Verified `LoadPercent` through temporary Blueprint debug inspection in `BP_ClimbingCharacter`.

Skipped checks:

- No automated gameplay test exists yet for hand transfer and attachment refresh.

Known risks:

- Dual-hand attachment currently uses the average of locked hand contacts; later tuning may need a solver-informed support frame.
- Transfer flow currently assumes hand grips only and does not yet integrate feet or reachability limits.

## Playable Loop 1 - Stage E - Body Tension Debug Integration

Date: 2026-04-23

Status: passed

Gates checked: `G0`, `G1`, `G4`, `G5`, `G6`, `G7`

Command verification:

- `ClimbEditor Win64 Development` build passed after wiring `UClimbingSolver` outputs into `FClimbingDebugState`.
- `UnrealEditor-Cmd.exe ... -ExecCmds="Automation RunTests Climb.Solver; Quit"` completed with exit code `0`.
- Automation log reported `Success` for all 4 `Climb.Solver` tests:
  - `Climb.Solver.ProjectCenterOfMassToWallPlane`
  - `Climb.Solver.EstimateTwoPointStability`
  - `Climb.Solver.EstimateBodyTension`
  - `Climb.Solver.CalculateDynoLaunchVelocity`
- Static inspection confirmed `AClimbingCharacter` still does not own Sweep / Trace implementation.
- Static inspection confirmed `UClimbingAnimInstance` still does not perform trace, solver, movement, or gameplay state logic.

Manual checks:

- In PIE, lock two tagged holds with left and right hands.
- Move the center-of-mass target with the left-stick substitute input.
- Confirm `CurrentBodyTension` rises as the CoM target moves farther from the support line between the two hands.
- Confirm `StabilityPercent` changes and remains finite.
- Confirm single-hand and no-hand cases do not produce NaN / Inf and do not trigger forced falling from the debug values alone.

Skipped checks:

- No automated gameplay test exists yet for runtime debug-value visualization.

Known risks:

- Single-hand stability currently falls back to a simplified debug state rather than a dedicated one-point solver.
- Body Tension and stability are still debug-only outputs and do not yet drive gameplay consequences.

## Control Rig Bridge Scaffold

Date: 2026-04-23

Status: command-verified

Gates checked: `G0`, `G1`, `G6`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding `ControlRigTargets` to `UClimbingAnimInstance`.
- Static inspection confirmed the new bridge package only groups existing animation-facing values and does not introduce gameplay authority into animation.
- Added `docs/CONTROL_RIG_BRIDGE.md` to document the future variable contract for `ABP_ClimbingCharacter` and `CR_ClimbingBody`.

Manual checks skipped:

- No editor asset wiring was performed yet for an actual climbing Control Rig asset.

Known risks:

- The bridge contract is now explicit, but the actual Control Rig asset and FBIK graph are still unimplemented.
- `ControlRigTargets` currently mirrors animation-facing values; if future rig needs local-space targets, that conversion still needs to be designed.

## Control Rig Target Space Fix

Date: 2026-04-27

Status: passed

Gates checked: `G0`, `G1`, `G6`

Command verification:

- `ClimbEditor Win64 Development` build passed after converting `PelvisOffset` and limb targets in `UClimbingAnimInstance` from world space to skeletal mesh component space.
- Static inspection confirmed the space conversion remains inside the animation bridge and does not move gameplay authority into Control Rig.

Manual checks:

- `CR_ClimbingBody` asset was created.
- `ABP_ClimbingCharacter` was wired to the Control Rig node.
- Pelvis offset and hand target values were confirmed to be present before the space fix.
- After the component-space conversion, the character no longer gets pulled far away when FBIK hand targets are active.

Known risks:

- FBIK behavior still depends on the in-editor rig graph setup and has not yet been tuned beyond the initial hand/pelvis verification.

## Control Rig First-Pass Hand FBIK

Date: 2026-04-28

Status: passed

Gates checked: `G0`, `G6`, `G7`

Command verification:

- No C++ code changes were required in this pass; the existing component-space bridge remained the runtime source for hand and pelvis targets.
- Static inspection confirmed the rig work stayed inside `CR_ClimbingBody` and did not move gameplay authority into Control Rig.

Manual checks:

- In `CR_ClimbingBody`, a first-pass FBIK graph was configured with pelvis offset followed by hand effectors.
- Left-hand-only manual validation passed: locking the left hand pulled the left arm toward the target without pulling the whole mesh away from the capsule.
- Right-hand-only manual validation passed with the same stability expectations.
- Dual-hand manual validation passed: both hands could stay active together without the character being pulled far away.
- Unlocking a hand returned that limb to the base pose influence rather than pinning it in space.

Skipped checks:

- Feet effectors were intentionally left out of this hand-focused pass.
- Hand rotation solving remains disabled in this pass.
- No automated editor or animation test exists yet for FBIK behavior.

Known risks:

- FBIK quality still depends on manual rig tuning inside `CR_ClimbingBody`; torso compression, elbow shape, and clavicle behavior may still need refinement.
- The current pass proves stable hand targeting, but not final climbing pose quality.

## Control Rig First-Pass Foot FBIK

Date: 2026-04-28

Status: passed

Gates checked: `G0`, `G6`, `G7`

Command verification:

- No C++ code changes were required in this pass; foot validation remained inside the Control Rig asset layer.
- Static inspection confirmed foot FBIK work stayed in `CR_ClimbingBody` and did not add gameplay state decisions into the rig.

Manual checks:

- Left and right foot effectors were connected in `CR_ClimbingBody`.
- Manual in-editor validation confirmed the foot FBIK setup is triggerable at the rig level.
- Foot effector hookup did not break the previously validated dual-hand FBIK stability.

Skipped checks:

- Foot gameplay lock/release flow is not implemented yet, so this pass did not verify runtime player-driven foot contact.
- Foot rotation solving remains disabled in this pass.
- No automated editor or animation test exists yet for foot FBIK behavior.

Known risks:

- Current validation proves rig-layer foot effector connectivity only, not final climbing-feel behavior.
- Because gameplay foot targeting does not exist yet, runtime foot targets still need a future C++ input, probe, and limb-state pass.

## Foot Gameplay Data Chain

Date: 2026-04-28

Status: passed

Gates checked: `G0`, `G1`, `G2`, `G3`, `G4`, `G6`, `G7`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding minimal foot gameplay bindings and limb-state flow.
- Static inspection confirmed trace ownership remains in `UClimbingHoldQueryComponent`.
- Static inspection confirmed Control Rig remains a presentation consumer; foot lock state still originates in `AClimbingCharacter`.

Manual checks:

- Assign `ClimbLeftFootGripAction` and `ClimbRightFootGripAction` on `BP_ClimbingCharacter`.
- Add temporary Enhanced Input mappings for left and right foot grip actions.
- In PIE while climbing, switch the active probe to a foot and confirm the probe origin falls back to the corresponding foot socket instead of the pelvis/anchor center.
- Confirm the foot probe target area is centered around the active foot rather than the hand-built body anchor.
- Trigger left and right foot grip separately and confirm `LeftFootState` / `RightFootState` become locked on valid tagged holds.
- Confirm foot targets propagate into the existing AnimInstance / Control Rig bridge without breaking hand FBIK.
- Confirm exiting climbing clears stale hand and foot locks instead of leaving FBIK pinned while falling.
- Confirm the first foot grip attempt after switching away from a hand uses the foot's freshly updated probe candidate rather than the previous hand candidate.

Skipped checks:

- No automated gameplay test exists yet for player-driven foot lock and release flow.
- Foot-only attachment is intentionally unsupported in this pass; movement attachment still depends on locked hands.

Known risks:

- Load distribution is now shared across locked hands and feet, but the movement attachment frame is still built from hands only.
- Foot probe now has a limb-centered target area, but it still uses the current wall-plane candidate model and may need later tuning for reachability and realistic stance.

## P1 - Hand Exploration Target Bridge

Date: 2026-04-28

Status: pending manual editor verification

Gates checked so far: `G0`, `G1`, `G4`, `G6`, `G7`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding hand exploration target data and animation bridge fields.
- Static inspection confirmed gameplay probe and candidate logic remain in `AClimbingCharacter`.
- Static inspection confirmed the new exploration target data is still transferred through `UClimbingAnimInstance` rather than computed inside Control Rig.

Manual checks pending:

- Confirm `ABP_ClimbingCharacter` can read `LeftHandExplorationTarget` and `RightHandExplorationTarget`.
- In `CR_ClimbingBody`, wire the active unlocked hand to the matching exploration target with lower weight than a locked hand target.
- In PIE, activate the left hand without locking a hold and move probe input.
- Confirm the unlocked active hand visibly follows the probe search area instead of staying fixed in the base pose.
- Lock a valid hold and confirm the hand transitions from exploration target to locked contact target without a large snap or mesh instability.
- Repeat for the right hand.

Skipped checks:

- Control Rig asset wiring for exploration targets is not automated and still requires editor work.
- Hand rotation solving remains out of scope for this pass.

Known risks:

- Exploration targets currently use candidate location when a valid hold exists and probe-forward fallback otherwise; that may still need feel tuning.
- Until the rig asset is updated, the new bridge data exists but will not change visible arm behavior on its own.
