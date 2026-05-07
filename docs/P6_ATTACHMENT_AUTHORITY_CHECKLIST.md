# P6 - Attachment Authority And Support Ownership Checklist

Use this checklist only for `P6 - Attachment Authority And Support Ownership`.

## Scope

- Keep feet fully manual.
- Do not introduce auto-stance, soft assist, or auto-lock.
- Align movement attachment ownership with the existing multi-contact solver.
- Allow mixed supports to keep climbing attachment when explicit ownership rules say they should.
- Keep trace ownership in `UClimbingHoldQueryComponent`.
- Keep Control Rig as presentation only.

## Command checks

1. Build `ClimbEditor Win64 Development`.
2. Statically inspect that attachment ownership rules remain in `AClimbingCharacter` / movement-facing gameplay code.
3. Statically inspect that solver logic remains in `UClimbingSolver`.

## Manual checks

1. Lock `2 hands` and confirm attachment still behaves like the validated hand-driven baseline.
2. Lock `1 hand + 2 feet`, then release the hand and confirm climbing does not immediately fall if the remaining foot support should still own attachment.
3. Lock `2 feet`, confirm the character can remain attached only when the new explicit ownership rules allow it.
4. Force an unstable feet-only or under-supported case and confirm the character exits climbing explicitly instead of silently hanging forever.
5. Recreate `1 hand + 2 feet + 1 free hand` and confirm the free hand can still manually explore and lock after ownership changes.
6. Reconfirm release semantics:
   - releasing one foot only clears that foot
   - releasing the final valid support exits climbing
   - releasing the final hand no longer guarantees falling if valid feet ownership remains
7. Recheck `CurrentBodyTension`, `StabilityPercent`, and `bIsPoseStable` during support transitions and confirm values remain finite and readable.

## Failing indicators

- Releasing the final hand always forces `Falling` even when valid supported feet should keep attachment.
- Feet can keep attachment in obviously unsupported or unstable cases.
- Mixed-support ownership changes break already validated manual exploration / lock flows.
- Attachment ownership logic leaks into Control Rig or trace code.
