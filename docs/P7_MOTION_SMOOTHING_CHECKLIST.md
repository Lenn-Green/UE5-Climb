# P7 - Motion Smoothing And Pose Stability Checklist

Use this checklist only for `P7 - Motion Smoothing And Pose Stability`.

## Scope

- Improve transition smoothness between:
  - exploration -> locked
  - locked -> released
  - one support layout -> another support layout
- Reduce visible popping, snapping, and over-aggressive target jumps.
- Reduce obvious limb twisting and torso collapse during large reach changes.
- Prevent or reduce obvious self-intersection and wall-penetration artifacts where feasible within the current Control Rig / FBIK setup.
- Replace pelvis-like or single-ray limb probing with limb-specific reachable wall-local query planes.
- Keep gameplay input semantics unchanged.
- Keep feet fully manual.

## Out of scope

- New hold semantics or hold-type gameplay rules
- Attachment authority redesign
- Auto-stance, soft assist, or auto-lock
- Final authored animation set

## Command checks

1. Build `ClimbEditor Win64 Development`.
2. Statically inspect that smoothing remains in animation-facing bridge / presentation logic and does not move gameplay decisions into Control Rig.
3. Statically inspect that any anti-twist or anti-penetration constraints do not rewrite trace ownership or solver ownership.
4. Statically inspect that reach-plane targeting still routes actual world queries through `UClimbingHoldQueryComponent`.

## Manual checks

1. Exploration -> locked transitions no longer visibly snap.
2. Release -> free pose transitions no longer visibly pop.
3. Large hand reach changes do not produce severe arm inversion or torso corkscrew poses.
4. Large foot reach changes do not produce severe leg inversion or knee collapse.
5. Mixed-support transitions remain readable when supports are added or removed.
6. Character limbs do not obviously tunnel through the wall during normal exploration and lock distances.
7. Self-intersection is reduced enough that poses remain readable during normal validated workflows.
8. Existing manual hand / foot exploration and grip flows still behave correctly after smoothing changes.
9. Chest-height hand holds can be reached through the hand reach plane without relying on a low body-origin ray.
10. Foot exploration remains foot-centered and does not regress while hand probing moves to a reach-plane model.
11. Reach-plane debug stays anchored to a stable body-facing work area instead of following the active limb end position.

## Failing indicators

- Exploration targets still jump directly into locked targets with no visible blend.
- Releasing a limb produces a hard snap or violent recoil.
- Large target changes create obvious impossible twists, folded limbs, or torso inversion.
- Limbs frequently penetrate the wall or body during ordinary validated moves.
- Smoothing changes break grip timing or manual control semantics.
- Hand or foot probing still depends on a low pelvis-like origin that misses obviously reachable holds at the correct limb height.
