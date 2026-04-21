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

Test checklist:

- Run Unreal project file generation for `Climb.uproject`.
- Build `ClimbEditor Win64 Development`.
- Manually inspect `Source/Climb` folder layout and module dependency list.
- Confirm `git status --short` contains no generated `Binaries`, `Intermediate`, `Saved`, or IDE files.

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

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Open the editor and create a Blueprint derived from `AClimbingCharacter`.
- Place the Blueprint in a test map and start PIE.
- Manually call or trigger `EnterClimbing`, `ExitClimbing`, and jump-prep state APIs through temporary Blueprint/debug hooks.
- Confirm logs or debug display show one active climbing state at a time.

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

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Open the editor and inspect `IA_ClimbCoMMove`, `IA_ClimbLimbProbe`, `IA_ClimbLeftGrip`, `IA_ClimbRightGrip`, and `IMC_Climbing`.
- Run PIE with a gamepad and verify left stick, right stick, L2, and R2 each update the expected debug/log value.
- Confirm left and right grip values are stored separately.
- Inspect code to confirm `UEnhancedInputComponent` and `UEnhancedInputLocalPlayerSubsystem` are used, not legacy input.

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

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Run a repeatable debug map or PIE setup with at least one valid hold and one invalid wall surface.
- Enable trace debug draw and verify sphere paths, hit location, and hit normal are visible.
- Confirm the owning character is ignored by the query.
- Inspect code to confirm trace implementation is not inside `AClimbingCharacter`.

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

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Run PIE and press/release L2 and R2 independently.
- Verify debug/log output for left hand and right hand limb state changes.
- Try locking to a valid hold and attempting to lock an invalid surface.
- Confirm release transitions do not leave contradictory limb or climbing states.

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

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Run available unit-style solver tests or a deterministic debug command.
- Verify fixed contact/CoM inputs produce expected stability, tension, and dyno outputs.
- Test missing contacts, coincident contacts, zero-length vectors, and extreme offsets.
- Inspect solver code for short geometry comments on non-trivial vector calculations.

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

Test checklist:

- Build `ClimbEditor Win64 Development`.
- Open the editor and verify the AnimInstance or bridge exposes climbing target data fields.
- In PIE, confirm debug target values change when C++ state or limb targets change.
- Confirm missing or unassigned Control Rig assets produce a graceful fallback or warning, not a crash.
- Inspect code to confirm animation code does not perform traces, solver math, or gameplay state decisions.

Mapped gates: `G0`, `G1`, `G6`.

## Phase Completion Policy

- A phase can be considered complete only when every mapped gate passes.
- Every phase completion must include a test record naming commands run, manual checks performed, skipped checks, and known risks.
- If a mapped gate cannot be fully verified, the implementation note must record why and what unlocks verification.
- Do not begin tuning advanced mechanics such as resonance swing until Phases 0-6 are stable.
- Do not add visual polish before input, trace, state, and solver boundaries are in place.
