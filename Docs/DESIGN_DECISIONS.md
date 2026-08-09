# Design Decisions

**Last Updated:** 2026-08-04

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
| Tear down stale local Steam session state before retrying Join | A joining process may retain a named `GameSession` after leaving or failing. A new in-game Join request destroys that local session first, then starts discovery only from the guarded completion callback; this is retry cleanup, not reconnect UX or gameplay authority. |
| Use UE 5.7 SteamSockets for Steam lobby P2P travel, with IP fallback | OnlineSubsystemSteam lobby connect strings use `steam.<id>` addresses that the prior IP-only driver contract could not carry. The fallback keeps editor and non-Steam/direct-connect diagnosis available when SteamSockets is unavailable. |
| Use a small C++ main menu for private playtest entry | The default map is a menu-only frontend. Local opens the established prototype map; online buttons delegate to the existing Steam helpers. This avoids an auto-start match and adds no gameplay or lobby authority. |
| Use a local gameplay submenu instead of Escape immediately leaving | Escape/controller-menu opens Resume, Controls, and an explicit Return action. It blocks only the invoking local controller's input and never pauses or controls an online match; Return reuses the established session teardown and frontend travel. |
| Online Party Hall uses a host-owned Ready Bell start gate | Online hosts and joining players remain free to move in Party Hall while the timer is frozen. After a second controller joins, P1/the listen host bonks the existing Ready Bell to begin the normal short countdown. Local play retains its existing all-active-player ready behavior. |
| Party Hall falls use immediate safety recovery, not Trial ring-out rules | Intermission is a staging/social space. Leaving its floor returns the wizard to their Party Hall spawn without score, Favor, attribution, delay, or ring-out telemetry; camera framing ignores the fallen wizard below the hall floor during recovery. |
| Replicate Party Hall board snapshots, not text-component behavior | GameMode authors the values on the server; the Party Hall actor replicates compact strings that clients apply visually. This supports late joins without giving board data gameplay authority. |
| Treat mouse delta separately from keyboard/gamepad turn rate | Mouse movement is already accumulated per frame and must not be multiplied by frame time. Q/E, right-stick, and fallback turning remain degrees-per-second inputs. Both retain the same gameplay impairment modifiers. |
| Decouple primary keyboard movement from wizard facing | WASD and arrow keys use the stable top-down camera basis so mouse aim can rotate the wizard/staff without rotating the player's movement frame. Left-stick and same-keyboard fallback behavior remain unchanged to avoid an unrequested controller/couch redesign. |
| Carry joiner facing through CharacterMovement control rotation | Autonomous facing should share Unreal's existing client-move channel instead of racing movement correction through a separate yaw RPC. Server validation remains bounded, and replicated Slosh may inform local movement prediction without becoming authoritative gameplay state. |
| Use project-owned frontend and gameplay maps with runtime presentation | `/Game/Maps/WizardStaff_MainMenu` is the frontend and `/Game/Maps/WizardStaff_Prototype` remains the gameplay map; together they replaced the engine OpenWorld fallback while runtime actors provide prototype spaces and fallback lighting. |
| Preserve local loose snapped physics; do not replicate debris physics | Online uses server-owned segment loss/count and minimal cues without client debris authority. |
| Use readable prototype presentation before production UI | Canvas HUD, markers, event feed, ritual actors, and fallback lighting remain scaffolding. |
| Tie Cauldron hazards to vial deposits | Each successful Speed/Burdening Power transfer gets one server-owned 25% matching slippery/sticky roll; unrelated timed hazards are removed. |
| Slippery puddles create a bounded Slosh-scaled skid | Server applies forward impulse and short low-friction carry-out without changing base Slosh or unrelated movement tuning. |
| Long-staff banking stays intentional | Validate server-confirmed Bonks against closest staff collision point while preserving active-side and bounded hold-range rules. |
| Cursed holder can feed the active intake | A server-confirmed intake Bonk creates eight telegraphed server blasts at 75% normal curse force. Jittered angular sectors and randomized distance spread danger broadly without a uniform ring; no score or vial shortcut. |
| Apply runtime physics mass after engine initialization | Native constructors define component defaults, while authority applies Cauldron ingredient mass in `BeginPlay`. This preserves the existing 16 kg tuning without querying physical material state during CDO construction, which UE 5.7 rejects during cook. |

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
