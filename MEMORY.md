# Project Memory

This file records durable context for future agents working on the Climb project.

## Stable Project Intent

Climb is an Unreal Engine 5.4 hardcore climbing simulation demo. The goal is not arcade traversal. The goal is a physically readable system where center of mass, limb contacts, body tension, momentum, and wall distance create the climbing experience.

## Current Repo Facts

- Workspace path: `C:\Users\tangyue\Documents\Unreal Projects\Climb`.
- Engine version: UE 5.4 from `Climb.uproject`.
- Current project base: ThirdPerson Blueprint template.
- Existing top-level folders: `Config`, `Content`, `DerivedDataCache`, `Intermediate`, `Saved`.
- No `Source/` C++ module exists yet.
- `Config/DefaultInput.ini` uses Enhanced Input default classes:
  - `/Script/EnhancedInput.EnhancedPlayerInput`
  - `/Script/EnhancedInput.EnhancedInputComponent`
- Existing input assets are still under `Content/ThirdPerson/Input`.
- Existing climber rig folder: `Content/Characters/Climber/Rigs`.

## Target Architecture

- C++ drives gameplay logic.
- Control Rig solves final FBIK presentation.
- `AClimbingCharacter` owns state coordination, not all implementation details.
- `UClimbingMovementComponent` owns climbing movement.
- Trace logic belongs in a component or query library.
- `UClimbingSolver` owns pure math.
- `UAnimInstance` bridges C++ outputs to Control Rig variables.

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

## Initial Implementation Priority

1. Add C++ module and folder skeleton.
2. Add `AClimbingCharacter`.
3. Add `UClimbingMovementComponent`.
4. Define Enhanced Input assets and C++ bindings for CoM movement, limb probe, and grip triggers.
5. Add Sphere Trace hold query outside the character class.
6. Define `FLimbState`.
7. Add minimum `UClimbingSolver` math interface.
8. Add an animation bridge placeholder for future Control Rig variables.

## Gate Policy

Use `docs/GATES.md` as the authoritative acceptance policy. Use `docs/IMPLEMENTATION_PLAN.md` as the staged execution plan. Do not mark a phase complete unless its mapped gate passes or the blocker is explicitly recorded.
