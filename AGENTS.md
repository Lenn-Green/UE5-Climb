# AI Engineering Contract

This document is the primary contract for Codex and other AI agents working in this Unreal Engine 5.4 project.

## Project Direction

Build a hardcore climbing simulation demo driven by C++ gameplay logic and expressed through Control Rig. The design prioritizes physically readable climbing behavior: center of mass, body tension, momentum conversion, dyno movement, heel hooks, and run-and-jump transitions.

## Required Architecture

- Use Unreal Engine 5.4 standard C++ patterns.
- Use Enhanced Input for all gameplay input.
- Keep gameplay logic in C++ and final body solving in Control Rig.
- Treat C++ as the brain and Control Rig as the execution layer.
- Do not place core gameplay decisions in Control Rig graphs.
- Do not build a monolithic `AClimbingCharacter`.

## C++ Responsibility Boundaries

- `AClimbingCharacter`
  - Own high-level climbing state orchestration.
  - Bind Enhanced Input actions.
  - Coordinate components and animation-facing data.
  - Do not directly contain trace-heavy, solver-heavy, or animation-bridge-heavy logic.

- `UClimbingMovementComponent`
  - Own custom climbing movement behavior.
  - Implement climbing tick behavior and custom movement mode handling.
  - Convert solver output into movement updates.

- `FLimbState`
  - Store per-limb contact state.
  - Include position, rotation, lock state, contact validity, and load percentage.
  - Keep this struct serializable and Blueprint-visible where useful.

- `UClimbingSolver`
  - Provide pure math and deterministic calculations.
  - Calculate center-of-mass relationships, stability, body tension, and dyno launch velocity.
  - Avoid direct world queries, actor mutation, animation calls, or input reads.

- Trace / hold query logic
  - Place Sphere Trace and hold-detection code in a component or static query library.
  - Return structured hit results or hold candidates.
  - Do not bury trace code directly inside the character class.

- Animation bridge
  - Use `UAnimInstance` or a dedicated bridge object to push C++ outputs to Control Rig variables.
  - Control Rig receives effector targets and body offsets; it does not decide gameplay state.

## Coding Standards

- Use UE naming conventions: PascalCase types and functions, `b` prefix for booleans, clear `UCLASS`, `USTRUCT`, `UENUM`, `UPROPERTY`, and `UFUNCTION` usage.
- Prefer `TObjectPtr` for UObject references where appropriate for UE 5.
- Expose tuning values with `UPROPERTY(EditDefaultsOnly)` or `UPROPERTY(EditAnywhere)` only when designers should tune them.
- Keep private implementation in `Source/Climb/Private` and public interfaces in `Source/Climb/Public`.
- Use folders such as `Character`, `Components`, `Data`, `Interfaces`, `Animation`, and `Solvers`.
- Complex vector, projection, or force calculations must include a short comment explaining the geometric principle.

## Asset and Naming Conventions

- C++ source target path: `Source/Climb/Public|Private/[Character|Components|Data|Interfaces|Animation|Solvers]`.
- Core assets: `Content/01_Core/`.
- Climber character assets: `Content/02_Characters/Climber/`.
- Control Rig assets: `Content/02_Characters/Climber/Rigs/`.
- Environment Blueprints: `Content/03_Environment/Blueprints/`.
- Prefix Blueprints with `BP_`.
- Prefix Control Rigs with `CR_`.
- Prefix Input Actions with `IA_`.
- Prefix Input Mapping Contexts with `IMC_`.
- Prefix Data Assets with `DA_`.

## Physics and Gameplay Rules

- Body Tension is derived from contact relationships, center-of-mass offset, and load distribution.
- Momentum conversion must preserve readable directionality. Lateral run-wall movement can feed vertical dyno launch velocity only through explicit solver logic.
- Resonance Swing requires input timing to align with the simulated swing frequency before it can amplify dyno power.
- Hand movement pulls the pelvis closer to the wall.
- Foot movement pushes the pelvis farther from the wall to create clearance.

## Gates

Before marking work complete, check the relevant gate in `docs/GATES.md`. If a task touches multiple systems, all affected gates must pass. If a gate cannot be run yet, document the reason and the next concrete unblocker.

## Current Phase

The project is in the base framework phase. Prioritize:

1. C++ module and directory skeleton.
2. `AClimbingCharacter`.
3. `UClimbingMovementComponent`.
4. Enhanced Input actions and mapping.
5. Independent Sphere Trace hold detection.
6. Minimal `FLimbState` and `UClimbingSolver` interfaces.
