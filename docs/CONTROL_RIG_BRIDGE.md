# Control Rig Bridge Contract

This document defines the intended boundary between gameplay C++ and future Control Rig assets.

## Purpose

`AClimbingCharacter`, `UClimbingMovementComponent`, `UClimbingHoldQueryComponent`, and `UClimbingSolver` remain the gameplay authority. `UClimbingAnimInstance` packages finalized animation targets so `ABP_ClimbingCharacter` and a future climbing Control Rig can consume them without re-implementing gameplay logic.

## Current Source of Truth

- Gameplay state: `AClimbingCharacter`
- Movement attachment: `UClimbingMovementComponent`
- Hold queries: `UClimbingHoldQueryComponent`
- Pure math: `UClimbingSolver`
- Animation bridge: `UClimbingAnimInstance`

## Current Rig Target Contract

`UClimbingAnimInstance` now exposes `ControlRigTargets` as a single Blueprint-readable package containing:

- `bIsClimbing`
- `ActiveProbeLimb`
- `PelvisOffset`
- `LeftHandTarget`
- `RightHandTarget`
- `LeftFootTarget`
- `RightFootTarget`

Each limb target contains:

- `bHasTarget`
- `bIsLocked`
- `TargetLocation`
- `TargetNormal`
- `TargetRotation`
- `LoadPercent`

## Recommended Control Rig Variable Names

When creating `CR_ClimbingBody`, keep variable names aligned with the bridge:

- `PelvisOffset`
- `LeftHandTarget`
- `RightHandTarget`
- `LeftFootTarget`
- `RightFootTarget`
- `ActiveProbeLimb`

`ABP_ClimbingCharacter` should read from `ControlRigTargets` and push those values into the Control Rig node or rig variables. Do not rebuild gameplay state inside the Anim Blueprint or the rig.

## Forbidden Responsibilities In Control Rig

Do not put the following in Control Rig:

- Hold validity checks
- Sphere Trace or any world query
- Climbing state transitions
- Body Tension / stability solver logic
- Grip press / release authority

Control Rig is presentation only: FBIK solving, secondary offsets, and visual polish.

## Next Integration Step

When the project begins the Control Rig asset phase:

1. Create `CR_ClimbingBody`.
2. Add variables matching the contract above.
3. In `ABP_ClimbingCharacter`, feed `ControlRigTargets` into the Control Rig node.
4. Use FBIK for hands, feet, and pelvis presentation only.
5. Keep failure cases graceful when the Control Rig asset is missing or partially wired.

## Current Implementation Note

`UClimbingAnimInstance` now converts pelvis and limb targets into skeletal mesh component space before exposing them to `ABP_ClimbingCharacter` and `CR_ClimbingBody`. Do not re-interpret those values as world-space transforms inside the rig.

## First-Pass FBIK Setup

The first Control Rig pass is meant to prove that the bridge, pelvis offset, and hand effectors work together without destabilizing the mesh. It is not the final climbing pose solve.

### ABP Wiring

In `ABP_ClimbingCharacter`:

1. Feed a stable base pose into a `Control Rig` node.
2. Assign `CR_ClimbingBody` as the rig class.
3. Expose and connect these inputs on the node:
   - `PelvisOffset`
   - `ActiveProbeLimb`
   - `LeftHandTarget`
   - `RightHandTarget`
   - `LeftFootTarget`
   - `RightFootTarget`
4. Route the Control Rig output into `Output Pose`.

Do not duplicate the inherited `UClimbingAnimInstance` variables inside the Anim Blueprint. Read the inherited values directly.

### Rig Variables

Inside `CR_ClimbingBody`, create variables with exactly the same names and types as the bridge contract:

- `PelvisOffset` as `Vector`
- `ActiveProbeLimb` as `EClimbingLimb`
- `LeftHandTarget` as `FClimbingLimbAnimTarget`
- `RightHandTarget` as `FClimbingLimbAnimTarget`
- `LeftFootTarget` as `FClimbingLimbAnimTarget`
- `RightFootTarget` as `FClimbingLimbAnimTarget`

### Pelvis Pass

Before FBIK, apply `PelvisOffset` to the pelvis bone.

- Node: `Set Translation - Bone`
- Bone: `pelvis`
- Space: `Local Space`
- Weight: `1.0`

The geometric intent is simple: `PelvisOffset` is already a local offset vector, so it should shift the pelvis relative to its current rig frame instead of forcing an absolute global location.

### FBIK Pass

Add a `Full Body IK` node after the pelvis translation step.

Recommended initial configuration:

- Root: `pelvis`
- Left hand effector bone: `hand_l`
- Right hand effector bone: `hand_r`
- Left and right `Chain Depth`: `4`
- `Pull Chain Alpha`: `1.0`
- `Pin Rotation`: `0.0`
- `Rotation Alpha`: `0.0`

For the first pass, connect only hand translation targets. Leave hand rotation solving disabled until the body is stable.

### Effector Input Mapping

For each hand effector:

- `Translation` <- `TargetLocation`
- `Position Alpha` <- `IsLocked`
- `Strength Alpha` <- `IsLocked`
- `Rotation Alpha` <- `0.0`

This keeps unlocked hands on the base pose and fully enables the effector only when gameplay has actually locked the hand to a hold.

### What Not To Do

Do not do any of the following in the first pass:

- Feed world-space target positions directly into FBIK
- Force hand rotation from `TargetRotation`
- Add feet effectors before the hand pass is stable
- Re-run hold selection, trace logic, or solver logic in the rig
- Use the rig to decide whether the character should keep climbing

## Validation Order

Use this order when debugging the rig:

1. Confirm `PelvisOffset` changes the pelvis bone by itself.
2. Confirm `LeftHandTarget` and `RightHandTarget` values change in the rig debugger.
3. Disconnect hand translations and verify the mesh no longer explodes or gets pulled away.
4. Reconnect one hand effector at a time.
5. Reconnect both hands only after single-hand behavior is stable.

This order isolates space mismatches and chain configuration problems before they get mixed with two-handed solving.

## Initial Acceptance Criteria

The first FBIK pass is acceptable when all of the following are true:

- Pelvis offset visibly shifts the torso while climbing.
- Locking the left hand moves the left arm toward the left target.
- Locking the right hand moves the right arm toward the right target.
- The mesh is not pulled far away from the capsule when hand targets activate.
- Unlocking a hand returns that arm to the base pose influence instead of pinning it in space.

Only after those criteria pass should the project move on to feet effectors, target rotation, and more advanced climbing pose polish.
