---
name: climb-ue5-hardcore-climbing
description: Use when working in the Climb Unreal Engine 5.4 project on C++ climbing gameplay, hardcore climbing simulation mechanics, Enhanced Input, Sphere Trace hold detection, Control Rig animation handoff, AI project constraints, gates, or implementation planning.
---

# Climb UE5 Hardcore Climbing

Use this project-local skill for work inside the Climb UE5 project.

## Required Reading

Before implementation or review work, read:

- `README.md`
- `AGENTS.md`
- `MEMORY.md`
- `docs/GATES.md`
- `docs/IMPLEMENTATION_PLAN.md`

## Workflow

- Inspect the current repo state before changing files.
- Name the gate or gates affected by the task.
- Keep C++ gameplay logic separate from Control Rig presentation.
- Use Enhanced Input for climbing controls.
- Put Sphere Trace and hold detection in a component or query library.
- Put deterministic climbing math in `UClimbingSolver` or equivalent pure solver code.
- Keep `AClimbingCharacter` focused on state orchestration and component coordination.
- Add short geometric comments for complex vector, projection, force, or momentum calculations.
- Verify the relevant gate before reporting work complete.

## Current Priorities

1. Create the C++ module and folder skeleton.
2. Add the climbing character base class.
3. Add the custom climbing movement component.
4. Add climbing Enhanced Input actions and mappings.
5. Add reusable Sphere Trace hold detection.
6. Add minimal limb state and solver interfaces.
7. Add the animation bridge placeholder for future Control Rig data.
