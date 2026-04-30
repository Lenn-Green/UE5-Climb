# P5 - Support Polygon And Multi-Contact Solver Checklist

Use this checklist only for `P5 - Support Polygon And Multi-Contact Solver`.

## Scope

- Replace the current widest-pair support approximation with a true multi-contact support model.
- Keep feet fully manual.
- Keep movement attachment authority hand-driven in this stage.
- Keep trace ownership in `UClimbingHoldQueryComponent`.
- Keep Control Rig as presentation only.

## Command checks

1. Build `ClimbEditor Win64 Development`.
2. Run existing solver automation and extend it if the new solver surface changes:
   - `Climb.Solver.ProjectCenterOfMassToWallPlane`
   - `Climb.Solver.EstimateTwoPointStability`
   - `Climb.Solver.EstimateBodyTension`
   - any new multi-contact solver tests added in this stage
3. Statically inspect that multi-contact logic remains in `UClimbingSolver`.

## Manual checks

1. Validate `2 hands` support still produces stable, finite debug values.
2. Validate `1 hand + 2 feet` support produces finite debug values that differ from the old widest-pair behavior when support geometry changes.
3. Validate `2 hands + 2 feet` support produces finite debug values without collapsing to obviously hand-only behavior.
4. Move CoM across the wall-plane target and confirm stability/body tension respond continuously under three- and four-contact support.
5. Confirm manual hand and foot exploration / lock flows still work after solver integration.
6. Confirm releasing supports updates debug values explicitly and does not leave stale polygon state behind.

## Failing indicators

- Solver still chooses only the widest pair while ignoring the rest of the locked contacts.
- Three- or four-contact support produces NaN, Inf, or discontinuous debug output.
- Multi-contact solver logic leaks into `AClimbingCharacter`, `UClimbingAnimInstance`, or Control Rig graphs.
- Manual climbing flow regresses while adding the new solver path.
