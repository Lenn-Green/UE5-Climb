# Post Playable Loop Plan

This document governs work that starts only after the original `docs/PLAYABLE_LOOP_1_PLAN.md` scope.

## Purpose

Playable Loop 1 proved the first hand-driven climbing loop. The next stage is not "more Playable Loop 1." It is a separate expansion phase focused on Control Rig usability, limb presentation, foot interaction, and future multi-limb support.

Use this plan for any work that exceeds the original Loop 1 scope, including:

- Control Rig FBIK asset tuning
- Limb-specific probe refinement
- Exploration IK for unlocked limbs
- Foot gameplay flow beyond placeholder data representation
- Future foot support or multi-limb support modeling

## Execution Order

The next phase should improve the arm / hand side first. Do not advance foot systems ahead of hand exploration and hand presentation polish unless the task is a narrow bug fix.

Recommended order:

1. Hand exploration and hand presentation
2. Hand-side Control Rig stabilization
3. Foot gameplay support
4. Multi-limb support modeling

## Boundary Rules

- Do not move work into this plan just because it feels adjacent to Loop 1.
- Finish the remaining Stage E manual verification before declaring Loop 1 closed.
- Do not add new gameplay mechanics here unless they are explicitly staged below.
- Keep Control Rig as presentation. Gameplay authority remains in C++.

## Stage P1 - Hand Exploration And Hand Presentation

Goal: make the active hand visibly search the wall before lock so probe input and arm motion finally match.

Scope:

- Add hand exploration target data in C++
- Bridge unlocked active-hand exploration targets into AnimInstance / Control Rig
- Make the active unlocked hand follow probe motion before grip lock
- Keep hand exploration on a wall-local 2D search plane with automatic depth, rather than treating probe as a free 3D aim ray
- Keep lock transition readable when switching from exploration target to locked contact target

Out of scope:

- Foot exploration IK
- Full-body authored climbing animation
- Attachment authority changes

Acceptance:

- Active unlocked left or right hand visibly follows probe motion
- Locking a hold transitions cleanly from exploration target to locked target
- Hand exploration does not break the validated hand attachment loop

Mapped gates: `G0`, `G4`, `G6`, `G7`

Status: passed and closed on 2026-04-29. Continue with `P2 - Control Rig Hand Stabilization`.

## Stage P2 - Control Rig Hand Stabilization

Goal: make the current `CR_ClimbingBody` usable and repeatable without changing gameplay authority.

Scope:

- Stabilize pelvis and hand FBIK hookup
- Standardize recommended rig parameters
- Keep locked-hand targets readable in animation
- Avoid mesh pull-away, limb pinning, and broken unlock behavior

Out of scope:

- Final limb rotation polish
- Foot gameplay support
- Support polygon gameplay

Acceptance:

- Existing hand targets remain stable under FBIK
- Unlocking a limb returns it to base pose influence
- Hand-side rig behavior remains stable while exploration IK is active

Mapped gates: `G0`, `G6`

Status: passed and closed on 2026-04-29. Continue with `P3 - Foot Gameplay Support`.

## Stage P3 - Foot Gameplay Support

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

Status: passed and closed on 2026-04-29. Continue with `P4 - Multi-Limb Support Model`.

## Stage P4 - Multi-Limb Support Model

Goal: begin treating feet as real support contributors instead of animation-only followers.

Scope:

- Revisit load distribution
- Decide when feet participate in attachment or stability models
- Expand solver assumptions beyond hand-only support
- Define how remaining free limbs continue exploration when hands and feet already share support
- Unify limb input semantics so `Q/E/Z/C` short-press activates exploration and long-press commits grip
- Bridge unlocked foot exploration targets into AnimInstance / Control Rig so feet can visibly search before lock
- Keep feet fully manual during this phase:
  - no auto-stance recommendation
  - no soft foot assist
  - no auto-lock for feet

Out of scope:

- Final competitive climbing move set
- Dyno / resonance swing / stamina integration
- Foot auto-assist, auto-stance, or auto-lock behavior

Acceptance:

- Any new support logic is explicitly reflected in solver inputs and debug state
- Attachment authority changes are documented before implementation
- State transitions remain explicit and debuggable

Mapped gates: `G0`, `G4`, `G5`, `G6`, `G7`

Current implementation order:

1. Introduce a support-aware frame for debug, probe, and exploration that can average locked hands and feet.
2. Keep movement attachment authority hand-driven until that first support-aware probe flow is validated.
3. Normalize `Q/E/Z/C` so pressing a limb first selects it for exploration and only a hold commits to locking.
4. Add unlocked foot exploration targets and visible foot-side exploration before changing attachment ownership.
5. Complete foot manual parity so feet match hands in full manual exploration / lock semantics before any later automation or attachment-authority changes.
6. Revisit attachment ownership only after the mixed-support exploration workflow is stable.

Current sub-stage focus:

- `P4.A` Mixed-support frame and unified manual input semantics: passed on 2026-04-30.
- `P4.B` Foot manual parity:
  - keep feet fully manual
  - make active unlocked feet follow the same exploration / lock / release contract as hands
  - do not introduce auto-stance, soft assist, or auto-lock

Status: passed and closed on 2026-04-30.

## Stage P5 - Support Polygon And Multi-Contact Solver

Goal: replace the current widest-pair approximation with a solver model that understands more than two simultaneous support contacts.

Scope:

- Add a support-polygon or equivalent multi-contact support representation in C++
- Feed all locked support contacts into the solver instead of collapsing to the widest pair
- Recompute stability and body-tension-facing debug values from the multi-contact model
- Keep feet fully manual
- Keep movement attachment authority unchanged in this stage

Out of scope:

- Automatic foothold recommendation
- Soft foot assist or auto-lock
- Changing movement attachment authority from hands to mixed supports
- Final authored climbing animation sets

Acceptance:

- `CurrentBodyTension`, `StabilityPercent`, and `bIsPoseStable` reflect three- and four-contact support cases without falling back to the current widest-pair shortcut
- Mixed-support debug remains finite and readable across `2 hands`, `1 hand + 2 feet`, and `2 hands + 2 feet`
- Solver changes remain isolated from trace ownership and Control Rig presentation

Mapped gates: `G0`, `G4`, `G5`, `G7`

Current implementation order:

1. Define a multi-contact support input shape for `UClimbingSolver`.
2. Add deterministic support-polygon or equivalent projection logic with edge-case handling and geometry comments.
3. Replace the current widest-pair debug shortcut with the new solver path.
4. Revalidate mixed-support debug and manual climbing flows without changing attachment authority.

Status: passed and closed on 2026-04-30.

## Stage P6 - Attachment Authority And Support Ownership

Goal: align gameplay attachment authority with the new multi-contact support model instead of keeping locomotion ownership hand-only.

Scope:

- Decide when feet become attachment-authority contributors instead of support-only contacts
- Define explicit transition rules between:
  - hand-driven attachment
  - mixed-support attachment
  - attachment loss and falling
- Update movement-facing attachment ownership without moving solver or trace logic into animation
- Keep feet fully manual unless a future stage explicitly changes that rule

Out of scope:

- Auto-stance, soft foot assist, or auto-lock
- Final authored climbing move set
- Stamina, dyno chaining, or resonance swing integration

Acceptance:

- Attachment ownership no longer assumes hands are the only support authority when mixed supports are valid
- Releasing the final hand does not automatically force falling if the remaining valid supports should keep the climber attached under the new explicit rules
- State transitions remain explicit, debuggable, and documented

Mapped gates: `G0`, `G4`, `G5`, `G7`

Current implementation order:

1. Define attachment-authority rules for hands, feet, and mixed supports.
2. Update movement-facing attachment refresh logic to consume the new ownership rules.
3. Revalidate release behavior, especially loss of final hand under supported mixed-contact cases.
4. Reconfirm manual hand/foot exploration and lock flows after ownership changes.

Status: deferred on 2026-05-07.

Deferral note:

- Pure support-balance ownership is not ready yet.
- The user decision is to keep the simpler rule for now:
  - releasing the final hand always exits climbing
  - feet remain support contributors, not attachment owners
- A later return to `P6` must include richer posture and hold-shape analysis instead of treating the current support solver as sufficient attachment authority by itself.

## Stage P7 - Motion Smoothing And Pose Stability

Goal: improve climbing presentation quality before introducing new gameplay semantics by smoothing transitions, reducing distortion, and avoiding obviously invalid poses.

Scope:

- Smooth exploration / lock / release target transitions
- Reduce popping across support-layout changes
- Reduce large-reach twisting, collapse, and corkscrew poses
- Add practical presentation-side limits or blends that reduce visible wall penetration and self-intersection
- Keep gameplay input semantics unchanged
- Keep feet fully manual

Out of scope:

- New hold semantics
- Attachment-authority redesign
- Auto-stance, soft assist, or auto-lock
- Final authored animation library

Acceptance:

- Exploration -> locked and locked -> released transitions are visibly smoother
- Large target changes remain readable and avoid obvious impossible poses
- Common validated workflows no longer produce frequent obvious wall penetration or severe self-intersection
- Manual hand / foot control semantics remain unchanged

Mapped gates: `G0`, `G4`, `G6`, `G7`

Current implementation order:

1. Identify target jump points in animation-facing bridge data.
2. Add smoothing / blend rules for exploration, lock, and release transitions.
3. Re-tune FBIK presentation parameters to reduce twist and collapse under large reaches.
4. Replace point-or-ray-centric limb probing with limb-specific reach planes for hands and feet.
5. Add practical pose constraints or presentation clamps to reduce visible penetration artifacts.
6. Revalidate all existing manual climbing flows after smoothing changes.

Current sub-stage focus:

- `P7.A` Bridge-side smoothing and first-pass pose clamps:
  - pelvis / target smoothing
  - surface clearance offsets
  - max reach clamps
- `P7.B` Limb Reach Plane Query:
  - define a hand reach plane instead of a pelvis-like probe origin + long aim line
  - define a foot reach plane instead of a single foot origin ray
  - keep each reach plane anchored to a stable body-facing work area rather than dragging the plane center with the active limb end effector
  - drive probe target selection from those local wall-relative reach planes
  - keep query ownership in `UClimbingHoldQueryComponent`
  - keep input semantics unchanged

`P7.B` intent:

- Hands and feet should search within their own reachable wall-local work area.
- The reach plane itself should be a stable work surface in front of the body, not a debug plane that follows the active limb tip.
- The system should stop depending on a single low body-origin ray to find chest-height hand holds.
- Query should center around a limb reach plane target first, then search locally for a candidate.

Status: open. `P7 - Motion Smoothing And Pose Stability` is the active stage.

## Documentation Rule

Before implementing a new stage from this plan:

1. Name the active stage in the task summary.
2. Update `docs/TEST_RECORDS.md` when the stage changes behavior.
3. Do not treat ad hoc fixes as permission to continue into the next stage automatically.
