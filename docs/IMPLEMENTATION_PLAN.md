# Implementation Plan

This plan covers the base framework only. It intentionally does not implement the full climbing game. Each phase maps to one or more gates in `docs/GATES.md`.

## Phase 0 - C++ Module and Project Skeleton

Goal: Convert the current Blueprint-template project into a C++-ready UE project without changing gameplay behavior.

Deliverables:

- `Source/Climb` module with standard `Public` and `Private` folders.
- Initial module rules with explicit dependencies for Engine, Core, CoreUObject, Enhanced Input, Animation, and Control Rig integration when needed.
- Folder skeleton for `Character`, `Components`, `Data`, `Interfaces`, `Animation`, and `Solvers`.

Acceptance:

- Project files generate successfully.
- Editor target builds.
- No unrelated asset or config churn.

Mapped gates: `G0`, `G1`.

## Phase 1 - Climbing Character and Movement Component

Goal: Establish the gameplay authority objects without implementing deep climbing physics yet.

Deliverables:

- `AClimbingCharacter` base class.
- `UClimbingMovementComponent` custom movement component.
- Minimal climbing state enum or state container.
- Narrow APIs for entering climbing, leaving climbing, and querying current climbing state.

Acceptance:

- Character compiles and can be used as a base for a Blueprint.
- Movement component can represent a climbing movement mode or equivalent framework state.
- State transitions are explicit and observable.

Mapped gates: `G0`, `G1`, `G4`.

## Phase 2 - Enhanced Input Framework

Goal: Add climbing-specific input actions and bindings.

Deliverables:

- Input Actions:
  - `IA_ClimbCoMMove`
  - `IA_ClimbLimbProbe`
  - `IA_ClimbLeftGrip`
  - `IA_ClimbRightGrip`
- Input Mapping Context:
  - `IMC_Climbing`
- C++ binding through `UEnhancedInputComponent`.
- Mapping context setup through `UEnhancedInputLocalPlayerSubsystem`.
- Stored input fields for CoM vector, limb probe vector, left grip value, and right grip value.

Acceptance:

- Left stick drives CoM input data.
- Right stick drives limb probe input data.
- L2 and R2 remain side-specific.
- No legacy input is used for core climbing controls.

Mapped gates: `G0`, `G1`, `G2`.

## Phase 3 - Hold Query and Sphere Trace

Goal: Implement reusable hold detection without coupling it to the character.

Deliverables:

- Hold query component or static query library.
- Sphere Trace parameters for radius, distance, collision channel, and debug draw.
- Structured hold candidate data with hit location, normal, actor, component, and score or distance.
- Owner ignore filtering.

Acceptance:

- Character can request hold candidates without owning trace implementation.
- Valid and invalid wall surfaces can be distinguished by an explicit rule.
- Debug trace visualization is available during development.

Mapped gates: `G0`, `G1`, `G3`.

## Phase 4 - Limb State and Basic Grip Flow

Goal: Store per-limb contact and grip information in a clear data model.

Deliverables:

- `FLimbState` with contact position, contact rotation or normal, lock state, validity, and load percentage.
- Four limb state fields for left hand, right hand, left foot, and right foot.
- Basic grip press and release flow for hands.
- Initial active limb selection rules for right-stick probing.

Acceptance:

- Left and right hand grip inputs update separate limb states.
- Releasing a hand clears or transitions that hand through an explicit release path.
- Invalid hold candidates do not lock a limb.

Mapped gates: `G0`, `G2`, `G3`, `G4`.

## Phase 5 - Minimal Climbing Solver

Goal: Add deterministic math interfaces before tuning gameplay feel.

Deliverables:

- `UClimbingSolver` or equivalent static solver class.
- Minimum functions for center-of-mass projection, stability estimate, body tension estimate, and dyno launch velocity estimate.
- Named tuning values for thresholds and scale factors.
- Short geometric comments for non-trivial vector operations.

Acceptance:

- Solver functions do not perform traces, read input, mutate actors, or call animation systems.
- Edge cases for missing contacts and zero-length vectors are handled.
- Fixed inputs produce repeatable outputs.

Mapped gates: `G0`, `G1`, `G5`.

## Phase 6 - Animation and Control Rig Bridge Placeholder

Goal: Create the C++ to animation data boundary without moving gameplay logic into animation.

Deliverables:

- AnimInstance subclass or bridge placeholder for climbing data.
- Data fields for limb effector targets, pelvis offset, and optional debug values.
- Clear handoff path from character or movement component to animation.
- Control Rig variable naming expectations documented in code or comments.

Acceptance:

- Gameplay state remains authoritative in C++.
- Animation receives final targets only.
- Missing Control Rig assets do not block base C++ framework testing.

Mapped gates: `G0`, `G1`, `G6`.

## Phase Completion Policy

- A phase can be considered complete only when every mapped gate passes.
- If a mapped gate cannot be fully verified, the implementation note must record why and what unlocks verification.
- Do not begin tuning advanced mechanics such as resonance swing until Phases 0-6 are stable.
- Do not add visual polish before input, trace, state, and solver boundaries are in place.
