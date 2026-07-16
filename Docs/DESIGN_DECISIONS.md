# Design Decisions

**Last Updated:** 2026-07-15

This file records durable choices, not implementation chronology. Current evidence belongs in [CURRENT_STATE.md](CURRENT_STATE.md).

## Active Decisions

| Decision | Rationale / evidence |
| --- | --- |
| Normal loop is Mug Run -> Staffs at Dawn -> Cauldron Catastrophe -> Grand Wizard Final, with Party Hall between phases | Human local and listen-server runs verified the handoffs. Destination presentation/collision must exist before one authoritative placement pass. |
| Separate Cauldron gameplay teardown from transition-floor lifetime | Results clears banking, curse, vials, hazards, bombs, arcs, timers, movement/readability effects, and callbacks immediately; the inert floor remains through intermission. |
| Keep local-first workflows while extending listen-server support | One-human-plus-bot, couch, shared camera, keyboard fallback, and local loose snapped physics must survive online changes. |
| Server owns gameplay truth | GameMode/server owns state, outcomes, scoring/Favor, combat, cleanup, Final, and reset. Replicated data, HUD, map actors, event messages, and Steam metadata are presentation/discovery only. |
| Presentation actors own explicit replicated phase-readable state | GameMode selects the phase; actors apply small idempotent replicated presentation states. OnRep paths never advance gameplay. |
| Keep reset, presentation, and staging responsibilities separate | Trial reset helpers reset owned state, presentation uses positive phase semantics, and staging teleports only after the destination is safe. |
| Dirty-check replicated readability mirrors | PlayerState forces updates only after real changes; display timers use 0.1-second and progress uses 1% readable thresholds. |
| Assigned PlayerState display slot is stable runtime identity | Preserve `WizardDisplaySlot`; new PlayerStates take the first unused slot. `PlayerId`/iterator order are pre-assignment fallbacks only. Reconnect restoration remains deferred. |
| Retain direct-connect as a development fallback | It is verified and remains faster to diagnose than the immature Steam join path. |
| Use the project-owned startup map with runtime presentation | `/Game/Maps/WizardStaff_Prototype` replaced the engine OpenWorld fallback; runtime actors provide most prototype spaces and fallback lighting. |
| Preserve local loose snapped physics; do not replicate debris physics | Online uses server-owned segment loss/count and minimal cues without client debris authority. |
| Use readable prototype presentation before production UI | Canvas HUD, markers, event feed, ritual actors, and fallback lighting remain scaffolding. |
| Tie Cauldron hazards to vial deposits | Each successful Speed/Burdening Power transfer gets one server-owned 25% matching slippery/sticky roll; unrelated timed hazards are removed. |
| Slippery puddles create a bounded Slosh-scaled skid | Server applies forward impulse and short low-friction carry-out without changing base Slosh or unrelated movement tuning. |
| Long-staff banking stays intentional | Validate server-confirmed Bonks against closest staff collision point while preserving active-side and bounded hold-range rules. |
| Cursed holder can feed the active intake | A server-confirmed intake Bonk creates eight telegraphed server blasts at 75% normal curse force. Jittered angular sectors and randomized distance spread danger broadly without a uniform ring; no score or vial shortcut. |

## Design Direction Requiring Care

| Direction | Current tension |
| --- | --- |
| Staff chaos should arise naturally | `UWizardStaffComponent` still has obstruction detection, control reduction, recovery, collision relief, and failsafes. Do not silently remove safety behavior without a tested replacement. |
| Longer staffs are power and a problem | Growth, collision, Stress, Slosh, snapping, and arena interaction support this; feel remains playtest-driven. |
| Readability should explain chaos | Keep causes visible through player markers, HUD mirrors, events, arcs/projectiles, Final states, and Cauldron cues. |

## Superseded Decisions

| Historical item | Current replacement |
| --- | --- |
| Engine OpenWorld direct-connect fallback | Project-owned startup/listen map. |
| AppID 480 development strategy | Real project Steam configuration; restore 480 only for an explicitly isolated test. |
| Loose ingredients as the main Cauldron loop | Vials, sequential banking, hazards, and curse risk. Ingredient actor remains legacy scaffolding. |
| Early rule prohibiting online multiplayer/replication | Listen-server/direct-connect/Steam smoke scaffolding exists; production online systems remain deferred. |
| Runtime-only arena workflow | Project-owned map plus authored/runtime fallback actors. |
| Prompt-by-prompt online architecture diary | Condensed milestone record; Git history retains chronology. |

## Documentation Rule

Update existing entries instead of appending prompt history. Put current evidence in `CURRENT_STATE.md`, risks in `KNOWN_ISSUES.md`, implementation ownership in `TECHNICAL_ARCHITECTURE.md`, and repeatable checks in `PLAYTEST_PLAN.md`.
