# P3 Foot Gameplay Checklist

This checklist is for `P3 - Foot Gameplay Support` only.

Use it after `P1` and `P2` are already closed.

## Goal

Turn the current foot data path into a deliberate gameplay system without changing the rule that movement attachment remains hand-driven.

## Required Gates

- `G0`
- `G1`
- `G2`
- `G3`
- `G4`
- `G6`
- `G7`

## Scope Boundary

This phase includes:

- left and right foot grip input validation
- foot probe origin and target area validation
- runtime foot lock / release validation
- bridge validation into AnimInstance / Control Rig

This phase does not include:

- foot-only climbing attachment
- heel hook / toe hook
- support polygon or stance solver logic
- changing attachment authority away from hands

## Baseline Expectations

Before starting `P3`, these assumptions must remain true:

- Hand-side exploration and hand-side rig stabilization are already passing.
- Foot effectors already exist in `CR_ClimbingBody`.
- Foot gameplay input still originates in `AClimbingCharacter`.
- Probe query ownership remains in `UClimbingHoldQueryComponent`.

## Runtime Input Setup

Confirm these assets are assigned in `BP_ClimbingCharacter`:

- `ClimbLeftFootGripAction`
- `ClimbRightFootGripAction`

Recommended temporary keyboard mapping:

- left foot grip -> `Z`
- right foot grip -> `C`

## Manual Validation Sequence

### Check 1 - Input Fires

Expected:

- `Z` changes left foot grip input
- `C` changes right foot grip input

Fail if:

- either input never reaches the character

### Check 2 - Probe Origin Uses Foot

Expected:

- while climbing, activating left foot makes probe origin appear near the left foot region
- activating right foot makes probe origin appear near the right foot region
- probe origin must not fall back to pelvis / anchor center in the normal foot case

Fail if:

- left and right foot use the same visible origin
- foot probe still appears to originate from the body center

### Check 3 - Probe Target Area Uses Foot-Centered Space

Expected:

- candidate search area is centered around the active foot rather than the hand-built body anchor
- foot probe movement feels local to the stance area

Fail if:

- foot probe behaves like a hand-centered search
- target area ignores active foot context

### Check 4 - Valid Hold Lock

Expected:

- left foot can lock a valid tagged hold
- right foot can lock a valid tagged hold
- locked foot state becomes visible through `LeftFootState` / `RightFootState`

Fail if:

- valid tagged holds do not lock
- lock works on one foot only

### Check 5 - Invalid Surface Rejection

Expected:

- untagged wall geometry is rejected
- foot input should not produce a false lock on invalid surfaces

Fail if:

- ordinary wall surfaces can be locked as feet

### Check 6 - Bridge To Animation

Expected:

- `LeftFootTarget` / `RightFootTarget` update in `ABP_ClimbingCharacter`
- foot targets reach `CR_ClimbingBody` without destabilizing the already-passed hand-side FBIK

Fail if:

- foot states change in gameplay but no animation targets update
- foot targets break hand-side stability

### Check 7 - Release / Exit Safety

Expected:

- releasing a foot clears only that foot state
- exiting climbing clears stale hand and foot locks
- falling state does not keep feet pinned through stale gameplay data

Fail if:

- foot states remain locked after release
- falling still behaves like feet are locked

## Exit Criteria

`P3` is ready to pass when:

- both feet can probe and lock valid tagged holds
- invalid wall surfaces are rejected
- foot targets bridge cleanly into animation / Control Rig
- no hand-driven attachment rules are accidentally changed
