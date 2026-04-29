# P2 Hand Stabilization Checklist

This checklist is for `P2 - Control Rig Hand Stabilization` only.

Use it when tuning `CR_ClimbingBody` after `P1 - Hand Exploration And Hand Presentation` has already passed.

## Goal

Make the current hand-side FBIK setup stable and repeatable without changing gameplay authority.

The scope is limited to:

- pelvis plus hand FBIK stability
- hand-side unlock behavior
- exploration and locked target coexistence
- practical parameter normalization inside `CR_ClimbingBody`

Do not use this checklist to justify:

- foot gameplay work
- support polygon changes
- rotation polish beyond basic stability
- gameplay logic moving into Control Rig

## Required Gates

- `G0`
- `G6`

## Baseline Rig Rules

Before tuning, keep these structural rules true:

- Use a single `Full Body IK` node for the hand-side setup.
- Do not switch between two different FBIK solvers for locked vs exploration states.
- Keep gameplay authority in C++; Rig only consumes:
  - `PelvisOffset`
  - `ActiveProbeLimb`
  - `LeftHandTarget`
  - `RightHandTarget`
  - `LeftHandExplorationTarget`
  - `RightHandExplorationTarget`
- Use `pelvis` as the FBIK root unless a later, explicit P2 decision changes it.
- Keep `Rotation Alpha` and `Pin Rotation` at `0.0` during stabilization unless there is a concrete reason to change them.

## Recommended Initial Parameters

Use these as the first stable baseline:

- `Chain Depth = 4`
- `Pull Chain Alpha = 1.0`
- `Rotation Alpha = 0.0`
- `Pin Rotation = 0.0`

For locked hands:

- `Position Alpha = 1.0`
- `Strength Alpha = 1.0`

For exploration hands:

- `Position Alpha = 0.75`
- `Strength Alpha = 0.35`

For inactive hands:

- `Position Alpha = 0.0`
- `Strength Alpha = 0.0`

## Required Three-State Hand Logic

Each hand must behave in three states:

1. `Locked`
2. `Exploration`
3. `Inactive`

Recommended logic:

- `UseLeftLocked = LeftHandTarget.IsLocked`
- `UseLeftExploration = bIsClimbing AND ActiveProbeLimb == LeftHand AND NOT LeftHandTarget.IsLocked AND LeftHandExplorationTarget.HasTarget`

Mirror the same logic for the right hand.

Use a single FBIK node and switch the hand effector inputs with `Select` nodes:

- Translation:
  - locked target when not exploring
  - exploration target when exploring
- Position Alpha:
  - `1.0` locked
  - `0.75` exploration
  - `0.0` inactive
- Strength Alpha:
  - `1.0` locked
  - `0.35` exploration
  - `0.0` inactive

## Manual Validation Sequence

Run these checks in order.

### Check 1 - Idle Pose Safety

Expected:

- At startup or while not climbing, neither hand is pulled by FBIK.
- Base pose remains readable.

Fail if:

- one or both hands drift in idle
- the mesh starts in a twisted or collapsed pose

### Check 2 - Locked Hand Stability

Expected:

- When the left hand locks, the left arm reaches the target without pulling the mesh away from the capsule.
- Repeat for the right hand.

Fail if:

- the whole body gets dragged
- the locked arm pins into an obviously broken shape

### Check 3 - Exploration Stability

Expected:

- With one hand locked, the opposite free hand becomes active and follows probe input.
- Exploration remains visibly responsive without high-frequency jitter.

Fail if:

- the exploration hand jitters continuously
- the non-active hand moves
- the body oscillates when only the exploration target changes

### Check 4 - Transition Stability

Expected:

- Exploration to locked transition is readable.
- No major snap, pull-away, or solver pop when grip lock happens.

Fail if:

- the hand jumps a large distance on lock
- the torso pops hard when switching target authority

### Check 5 - Unlock Recovery

Expected:

- Unlocking a hand removes locked influence cleanly.
- The hand returns to base-pose influence unless it is now the active exploration hand.

Fail if:

- unlocked hand remains pinned
- unlocking causes visible mesh collapse

### Check 6 - Symmetry

Expected:

- Left-hand and right-hand behavior are meaningfully symmetric.

Fail if:

- one side is clearly stable and the other is not

## Tuning Order

Change parameters in this order only:

1. `Position Alpha`
2. `Strength Alpha`
3. `Chain Depth`

Do not start with rotation parameters.

Interpretation:

- If the hand does not reach enough, raise `Position Alpha` first.
- If the body is dragged too much, lower `Strength Alpha`.
- If the arm chain does not solve enough, inspect `Chain Depth`.

## Exit Criteria

`P2` is ready to pass when:

- locked-hand FBIK is stable
- exploration-hand FBIK remains stable at the same time
- unlocking a hand returns it to base-pose influence
- the same graph works for both left and right hands
- no gameplay authority moved into the Rig
