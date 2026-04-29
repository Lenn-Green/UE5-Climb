# Project Memory

This file records durable context for future agents working on the Climb project.

## Stable Project Intent

Climb is an Unreal Engine 5.4 hardcore climbing simulation demo. The goal is not arcade traversal. The goal is a physically readable system where center of mass, limb contacts, body tension, momentum, and wall distance create the climbing experience.

## Current Repo Facts

- Workspace path: `C:\Users\tangyue\Documents\Unreal Projects\Climb`.
- Engine version: UE 5.4 from `Climb.uproject`.
- Project base: ThirdPerson template converted to a C++ project.
- Runtime module exists at `Source/Climb`.
- Current C++ framework includes:
  - `AClimbingCharacter`
  - `UClimbingMovementComponent`
  - `UClimbingHoldQueryComponent`
  - `UClimbingSolver`
  - `UClimbingAnimInstance`
- Enhanced Input is the required input system.
- Climbing input assets exist for center-of-mass movement, limb probe movement, left grip, and right grip.
- Climbing assets are organized under `Content/Climb`, including character Blueprints, input assets, environment hold Blueprint, and climbing Anim Blueprint.
- Base framework Phase 0-6 is complete and documented in `docs/TEST_RECORDS.md`.

## Target Architecture

- C++ drives gameplay logic.
- Control Rig solves final FBIK presentation.
- `AClimbingCharacter` owns state coordination, not all implementation details.
- `UClimbingMovementComponent` owns climbing movement.
- Trace logic belongs in `UClimbingHoldQueryComponent` or another query-only module.
- `UClimbingSolver` owns pure math.
- `UClimbingAnimInstance` bridges finalized C++ outputs to animation / Control Rig variables.

## Core Mechanics To Preserve

- Left stick moves center of mass / pelvis on the wall plane.
- Right stick moves the active limb probe on the wall.
- L2 and R2 map to left and right hand grip control.
- Releasing a grip transitions that hand into physical release behavior.
- Body Tension evaluates stability from contacts, center-of-mass offset, and load distribution.
- Momentum System converts wall-run or swing momentum into controlled jump velocity.
- Resonance Swing rewards input rhythm aligned with the simulated swing frequency.
- Z-axis pelvis offset is automated:
  - Hand movement decreases pelvis wall distance.
  - Foot movement increases pelvis wall distance.

## Current Implementation Priority

Playable Loop 1 is complete and documented. Use `docs/POST_PLAYABLE_LOOP_PLAN.md` for all subsequent work on:

1. Foot support and future multi-limb support modeling

`P1 - Hand Exploration And Hand Presentation` is complete and recorded in `docs/TEST_RECORDS.md`.
`P2 - Control Rig Hand Stabilization` is complete and recorded in `docs/TEST_RECORDS.md`.
`P3 - Foot Gameplay Support` is complete and recorded in `docs/TEST_RECORDS.md`.

## Gate Policy

Use `docs/GATES.md` as the authoritative acceptance policy. Use `docs/IMPLEMENTATION_PLAN.md` for the completed base framework plan, `docs/PLAYABLE_LOOP_1_PLAN.md` as the closed record of the first climbing loop, and `docs/POST_PLAYABLE_LOOP_PLAN.md` for all later Rig / limb expansion work. Do not mark a phase complete unless its mapped gate passes or the blocker is explicitly recorded.
