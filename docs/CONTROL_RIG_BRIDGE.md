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
