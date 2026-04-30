# P4 - Multi-Limb Support Checklist

Use this checklist only for `P4 - Multi-Limb Support Model`.

## Scope of the current P4 pass

- Keep movement attachment hand-driven.
- Allow free-limb probe and exploration to use a mixed support frame built from locked hands and feet.
- Reflect mixed support usage in debug and solver-facing runtime values.
- Use unified limb input semantics:
  - short press on `Q/E/Z/C` activates the matching limb for exploration
  - hold past the lock delay attempts grip lock
  - releasing a locked limb releases that limb only
- Bridge `LeftFootExplorationTarget` and `RightFootExplorationTarget` to AnimInstance / Control Rig.
- Keep feet fully manual in this pass.
  - No foot auto-stance recommendation
  - No soft assist toward candidate footholds
  - No automatic foot locking
- Complete foot manual parity with hands.
  - short press selects the matching foot for exploration
  - active unlocked foot visibly explores before lock
  - long press commits lock only after the hold threshold
  - release clears only that foot

## Manual checks

1. Short press `Q`, `E`, `Z`, and `C` and confirm each one only activates the matching limb for exploration without immediately locking.
2. Hold `Q`, `E`, `Z`, and `C` past the lock delay and confirm each matching limb attempts to lock only after the hold threshold.
3. Lock `1 hand + 2 feet` and confirm the remaining free hand becomes the active exploration hand.
4. Move limb probe input and confirm the free hand searches from a support-aware area instead of a hand-only anchor.
5. Confirm the free hand can lock a valid tagged hold while the other hand and both feet remain locked.
6. Mirror the same test on the opposite side.
7. Confirm `CurrentBodyTension`, `StabilityPercent`, and `bIsPoseStable` remain finite and readable under mixed hand-foot support.
8. Confirm release and state transitions still behave explicitly:
   - releasing a foot only clears that foot
   - releasing the exploring hand does not clear existing supports
   - releasing the final hand still exits climbing because movement attachment authority has not changed yet
9. In `ABP_ClimbingCharacter`, confirm `LeftFootExplorationTarget` and `RightFootExplorationTarget` update for the active unlocked foot.
10. In `CR_ClimbingBody`, confirm the active unlocked foot can visibly search using the new exploration target before lock.
11. Confirm feet still require explicit manual exploration and manual lock input; no automatic foothold behavior should occur.
12. Confirm foot manual behavior now matches hand manual behavior closely enough that feet are not treated as a weaker placeholder interaction path.

## Failing indicators

- Short press still causes an immediate lock attempt.
- Hold press does not commit a grip after the delay.
- Free hand exploration still behaves as if only locked hands exist.
- Free hand cannot naturally probe or lock while mixed supports remain active.
- Active unlocked foot still has no exploration target data, or the rig cannot consume it.
- Mixed support debug values stay stuck in the old hand-only behavior.
- Mixed support probing breaks already validated hand-only or foot-only flows.
- Feet begin auto-selecting or auto-locking footholds without explicit player input.
