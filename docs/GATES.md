# Gate System

This project uses strict gates. A change is not complete because it compiles or appears to work once; it is complete when the relevant gate criteria pass and the failure modes are understood.

## Gate Rules

- Every implementation task must name the gate or gates it affects.
- A failed gate blocks completion of the related phase.
- Every phase must include both command/automated verification and a manual inspection checklist.
- A phase is incomplete if its tests are not documented, even when the implementation appears to work.
- If a gate cannot be executed yet, record the missing prerequisite and the next concrete unblocker.
- Do not weaken a gate to make a change pass.
- Do not mix unrelated refactors into gate work.
- Do not modify `.uproject`, `Config`, `Content`, or source files outside the active task without a clear reason.

## Required Test Record

Each completed phase must leave a short test record in the final implementation note or commit message:

- Gates checked.
- Commands run and whether they passed.
- Manual checks performed.
- Checks skipped, with the concrete reason and unblocker.
- Known residual risks.

## G0 Project Hygiene

Purpose: Keep the project structure understandable and prevent accidental asset or config churn.

Pass criteria:

- New files follow the documented folder and naming conventions.
- Markdown docs remain consistent with each other.
- Generated or transient files are not committed as source work.
- No unrelated assets, maps, config, or template files are changed.
- The task summary names all touched subsystems.

Forbidden:

- Moving existing assets only for cosmetic reasons.
- Creating duplicate naming schemes.
- Mixing documentation, gameplay code, and unrelated cleanup in one task.

Verification:

- Inspect changed file list.
- Re-read any changed docs.
- Confirm paths match `README.md`, `AGENTS.md`, and this file.

Failure handling:

- Revert only the agent's own unrelated changes.
- Split unrelated work into a separate task.

## G1 Build Gate

Purpose: Ensure the C++ project can generate, compile, and load.

Pass criteria:

- `Source/Climb` module exists when C++ work begins.
- Module dependencies include required Unreal modules explicitly.
- Enhanced Input dependencies are declared before input C++ code lands.
- Project files can be generated.
- Editor or command-line build succeeds for the active target.

Forbidden:

- Adding include paths to hide poor module boundaries.
- Depending on Blueprint-only behavior for required C++ systems.
- Leaving generated code errors unresolved.

Verification:

- Generate project files when module structure changes.
- Build the editor target.
- Record any build command that cannot run locally.

Failure handling:

- Fix module rules, includes, or target configuration first.
- Do not continue to gameplay implementation while the module cannot build.

## G2 Input Gate

Purpose: Ensure climbing controls are implemented through Enhanced Input and are testable.

Pass criteria:

- Input Actions exist for center-of-mass movement, limb probe movement, left grip, and right grip.
- Input Mapping Context maps gamepad left stick, right stick, L2, and R2.
- C++ bindings use `UEnhancedInputComponent`.
- Mapping context is added through `UEnhancedInputLocalPlayerSubsystem`.
- Input values are stored in clear, inspectable fields before being consumed by movement or trace logic.

Forbidden:

- Legacy input bindings for core climbing controls.
- Reading raw keys directly in gameplay tick.
- Combining left and right grip into ambiguous state without preserving side.

Verification:

- Inspect assets or C++ setup.
- Run in editor when available and verify each action fires.
- Log or debug-display input vectors and grip values during initial integration.

Failure handling:

- Fix input asset type, mapping, or subsystem setup before implementing dependent gameplay.

## G3 Trace Gate

Purpose: Ensure hold detection is modular, debuggable, and independent of character orchestration.

Pass criteria:

- Sphere Trace logic is implemented in a component or static query library, not directly in `AClimbingCharacter`.
- Trace radius, distance, channels, and debug draw mode are tunable.
- Query returns structured hold candidates or a clear no-hit result.
- Trace filtering ignores the owning character.
- Hit location, normal, actor, component, and confidence or distance score are available to callers.

Forbidden:

- Hard-coded magic trace distances inside input handlers.
- Character-owned trace code that cannot be reused by hands and feet.
- Treating any wall hit as a valid climbing hold without an explicit rule.

Verification:

- Unit-level validation where possible for result sorting/filtering.
- In-editor debug trace visualization.
- Manual test against at least one valid hold and one invalid wall surface.

Failure handling:

- Move trace code out of the character before continuing.
- Add debug output before tuning gameplay feel.

## G4 Climbing State Gate

Purpose: Ensure climbing state transitions are explicit and reproducible.

Pass criteria:

- States include at minimum grounded / falling, attached or climbing, and jump or dyno preparation.
- Grip press, grip release, valid hold found, invalid hold, and loss of stability have explicit transition paths.
- Movement mode changes are centralized through the movement component or a narrow character API.
- State transitions can be observed through logs, debug display, or Blueprint-readable state.

Forbidden:

- Boolean piles that allow contradictory states such as climbing and falling simultaneously.
- Triggering physics release from multiple unrelated call sites.
- Letting animation state be the authority for gameplay state.

Verification:

- Simulate grip attach and release.
- Verify invalid hold does not enter climbing state.
- Verify release returns to falling or appropriate movement mode.

Failure handling:

- Replace ambiguous booleans with an enum or explicit state object.
- Add transition logging before adding more mechanics.

## G5 Physics Solver Gate

Purpose: Keep climbing math deterministic, isolated, and explainable.

Pass criteria:

- Center-of-mass projection, body tension, stability, and dyno velocity calculations live in `UClimbingSolver` or equivalent pure solver code.
- Complex vector calculations include a short geometric comment.
- Solver inputs and outputs are plain data, not world queries or actor mutations.
- Edge cases such as missing contacts, coincident contacts, and zero-length vectors are handled.
- Calculations are repeatable with fixed inputs.

Forbidden:

- Math hidden inside animation graphs or tick handlers.
- Solver code that reads player input or performs traces.
- Using arbitrary constants without exposing them as named tuning values.

Verification:

- Add unit-style tests or repeatable debug scenarios when automation is not yet available.
- Compare known inputs against expected stability and launch outputs.
- Check for NaN, infinity, and zero-vector failure cases.

Failure handling:

- Extract math from gameplay classes.
- Add named intermediate values and comments before tuning.

## G6 Animation Bridge Gate

Purpose: Keep the C++ to Control Rig boundary clean.

Pass criteria:

- AnimInstance or a dedicated bridge receives finalized gameplay targets from C++.
- Control Rig variables represent effector targets, body offsets, and optional debug values.
- Control Rig performs FBIK solving and presentation only.
- Gameplay state remains authoritative in C++.
- Missing Control Rig assets fail gracefully during early framework work.

Forbidden:

- Control Rig deciding whether a hand is attached, released, or valid.
- AnimInstance performing Sphere Traces or stability math.
- Duplicating solver constants in animation assets.

Verification:

- Inspect variable names and data flow.
- In editor, confirm target values change as C++ state changes once assets exist.
- Confirm gameplay still has a clear fallback when animation assets are missing.

Failure handling:

- Move gameplay decisions back into C++.
- Narrow the bridge to data transfer and presentation state.
