# Post Playable Loop Plan

This document governs work that starts only after the original `docs/PLAYABLE_LOOP_1_PLAN.md` scope.

## Purpose

Playable Loop 1 proved the first hand-driven climbing loop. The next stage is not "more Playable Loop 1." It is a separate expansion phase focused on Control Rig usability, limb presentation, foot interaction, and future multi-limb support.

Use this plan for any work that exceeds the original Loop 1 scope, including:

- Control Rig FBIK asset tuning
- Foot gameplay flow beyond placeholder data representation
- Limb-specific probe refinement
- Exploration IK for unlocked limbs
- Future foot support or multi-limb support modeling

## Boundary Rules

- Do not move work into this plan just because it feels adjacent to Loop 1.
- Finish the remaining Stage E manual verification before declaring Loop 1 closed.
- Do not add new gameplay mechanics here unless they are explicitly staged below.
- Keep Control Rig as presentation. Gameplay authority remains in C++.

## Stage P1 - Control Rig First-Pass Stabilization

Goal: make the current `CR_ClimbingBody` usable and repeatable without changing gameplay authority.

Scope:

- Stabilize pelvis, hand, and foot FBIK hookup
- Standardize recommended rig parameters
- Keep locked-hand and locked-foot targets readable in animation
- Avoid mesh pull-away, limb pinning, and broken unlock behavior

Out of scope:

- Exploration IK
- Final limb rotation polish
- Support polygon gameplay

Acceptance:

- Existing limb targets remain stable under FBIK
- Unlocking a limb returns it to base pose influence
- Foot FBIK does not break validated hand behavior

Mapped gates: `G0`, `G6`

## Stage P2 - Foot Gameplay Support

Goal: turn the current minimal foot data path into a deliberate gameplay system.

Scope:

- Formalize foot grip inputs and testing assets
- Improve foot probe origin and target area behavior
- Validate foot lock / release flow through PIE
- Keep movement attachment hand-driven unless a later stage changes that rule

Out of scope:

- Foot-only climbing attachment
- Heel hook / toe hook
- Advanced stance solver logic

Acceptance:

- Left and right feet can probe and lock valid tagged holds
- Invalid wall surfaces are rejected for feet
- Foot targets bridge cleanly into animation / Control Rig

Mapped gates: `G0`, `G1`, `G2`, `G3`, `G4`, `G6`, `G7`

## Stage P3 - Exploration IK For Unlocked Limbs

Goal: make the active, unlocked limb visually search the wall instead of only moving an invisible probe.

Scope:

- Add exploration target data from C++ for the active limb
- Bridge exploration targets into AnimInstance / Control Rig
- Make unlocked active hands use exploration IK before lock
- Extend to feet only after hands are stable

Out of scope:

- Final authored climbing animation system
- Full-body anticipation and reachability solver

Acceptance:

- Active unlocked hand follows probe movement visually
- Locking a hold transitions from exploration target to locked contact target
- Exploration behavior does not break current locked-limb stability

Mapped gates: `G0`, `G4`, `G6`, `G7`

## Stage P4 - Multi-Limb Support Model

Goal: begin treating feet as real support contributors instead of animation-only followers.

Scope:

- Revisit load distribution
- Decide when feet participate in attachment or stability models
- Expand solver assumptions beyond hand-only support

Out of scope:

- Final competitive climbing move set
- Dyno / resonance swing / stamina integration

Acceptance:

- Any new support logic is explicitly reflected in solver inputs and debug state
- Attachment authority changes are documented before implementation
- State transitions remain explicit and debuggable

Mapped gates: `G0`, `G4`, `G5`, `G6`, `G7`

## Documentation Rule

Before implementing a new stage from this plan:

1. Name the active stage in the task summary.
2. Update `docs/TEST_RECORDS.md` when the stage changes behavior.
3. Do not treat ad hoc fixes as permission to continue into the next stage automatically.
