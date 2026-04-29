# P4 - Multi-Limb Support Checklist

Use this checklist only for `P4 - Multi-Limb Support Model`.

## Scope of this first P4 pass

- Keep movement attachment hand-driven.
- Allow free-limb probe and exploration to use a mixed support frame built from locked hands and feet.
- Reflect mixed support usage in debug and solver-facing runtime values.

## Manual checks

1. Lock `1 hand + 2 feet` and confirm the remaining free hand becomes the active exploration hand.
2. Move limb probe input and confirm the free hand searches from a support-aware area instead of a hand-only anchor.
3. Confirm the free hand can lock a valid tagged hold while the other hand and both feet remain locked.
4. Mirror the same test on the opposite side.
5. Confirm `CurrentBodyTension`, `StabilityPercent`, and `bIsPoseStable` remain finite and readable under mixed hand-foot support.
6. Confirm release and state transitions still behave explicitly:
   - releasing a foot only clears that foot
   - releasing the exploring hand does not clear existing supports
   - releasing the final hand still exits climbing because movement attachment authority has not changed yet

## Failing indicators

- Free hand exploration still behaves as if only locked hands exist.
- Free hand cannot naturally probe or lock while mixed supports remain active.
- Mixed support debug values stay stuck in the old hand-only behavior.
- Mixed support probing breaks already validated hand-only or foot-only flows.
