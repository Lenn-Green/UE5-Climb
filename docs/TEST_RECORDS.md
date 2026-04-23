# Test Records

This file records phase gate checks that were completed during project setup and feature work.

## Phase 3 - Hold Query and Sphere Trace

Date: 2026-04-23

Gates checked: `G0`, `G1`, `G3`

Command verification:

- `ClimbEditor Win64 Development` build passed after adding `UClimbingHoldQueryComponent`.
- Static inspection confirmed `AClimbingCharacter` does not contain Sweep/Trace calls.
- Static inspection confirmed Sphere Trace / Sweep logic lives in `UClimbingHoldQueryComponent`.

Manual checks:

- Created `BP_ClimbingHold` test Blueprint.
- Added `ClimbingHold` tag to a test hold Actor.
- Placed tagged hold and untagged test geometry in the ThirdPerson test map.
- Enabled `HoldQueryComponent` debug draw on `BP_ClimbingCharacter`.
- PIE test confirmed the tagged hold is detected.
- PIE test confirmed untagged wall/geometry is rejected.
- Debug draw showed trace path, hit point, and normal direction.

Skipped checks:

- No automated gameplay test exists yet for hold query filtering.

Known risks:

- Hold validity currently depends on the literal `ClimbingHold` Actor or Component tag.
- Collision channel setup is manual and must block the configured query channel.
