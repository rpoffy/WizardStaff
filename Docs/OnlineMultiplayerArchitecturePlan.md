# Online Multiplayer Milestone Record

**Last Updated:** 2026-07-15
**Purpose:** Concise historical record of the online spike. Current truth lives in [CURRENT_STATE.md](CURRENT_STATE.md), [TECHNICAL_ARCHITECTURE.md](TECHNICAL_ARCHITECTURE.md), and [KNOWN_ISSUES.md](KNOWN_ISSUES.md).

This replaces the former prompt-by-prompt architecture diary. Git history retains that chronology. Do not append implementation prompts or repeat full smoke-test instructions here.

## Current Baseline

| Area | Status | Evidence boundary |
| --- | --- | --- |
| Local one-human-plus-bot and couch play | **Implemented and verified** | Preserved through the online spike and current loop testing. |
| Two-player listen-server PIE | **Implemented and verified** | Full loop, mechanics, transition presentation, mirror dirty-checking, and slot-first identity were human-verified on 2026-07-15. |
| Separate-process direct-connect | **Implemented and verified** | Host/client full-loop smoke test passed; direct-connect remains the preferred network-debug fallback. |
| Steam host session creation | **Implemented and verified** | Steam subsystem loaded, session creation succeeded, and listen travel occurred in prior smoke logs. |
| Steam session search/join/travel | **Partially implemented** | Helper exists; same-machine testing hit Steam API limitations and no valid two-machine/two-account result is recorded. |
| Steam private build | **Implemented and verified** | Private build installed through Steam; a later build fixed runtime lighting and passed a full playthrough. |
| Steam leaderboard submission | **Implemented but unverified** | Authoritative result queue/flush scaffold exists; no successful write/read-back evidence is recorded. |
| Production online UX | **Deferred** | No production lobby, browser, matchmaking, invites, reconnect, host migration, or dedicated-server architecture. |

## Authority Contract

Steam and direct-connect only discover or connect players. They do not change gameplay authority.

The listen server/GameMode owns:

- match and Trial state, timers, Trial order, Results, Final, and rematch/reset;
- player-slot assignment, score, Favor, staff growth/loss, Slosh, Stress, and snaps;
- pickups, rewards, Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, powerups, Mega Staff, and Cauldron outcomes;
- Candidate, steal progress, winner selection, event publication, and actor cleanup.

Clients request owned actions and display replicated state. GameState, PlayerState, character mirrors, event messages, runtime presentation actors, Steam metadata, and HUD data never drive gameplay.

## Implemented Milestones

| Milestone | Current result |
| --- | --- |
| GameState and PlayerState mirrors | Match/Trial/timer/result and per-player identity/score readability replicate. Setters dirty-check values; display timers use 0.1-second and progress uses 1% readable thresholds. |
| Session-mode separation | `LocalPrototype`, `LocalWithBots`, `OnlineListenServer`, and `OnlineClient` gate local helpers without forming a production session architecture. |
| Pawn/control scaffold | Host and client each receive one controlled wizard; local-only player creation, keyboard fallback, and bot fill stay offline-only. |
| Shared camera and presentation | Host/client use the top-down shared camera. Runtime Party Hall, Trial arenas, Final ritual presentation, and fallback lighting are readable remotely. |
| Stable player identity | Assigned `AWizardStaffPlayerState::WizardDisplaySlot` is preserved and preferred over controller iteration. New PlayerStates take the first unused slot; reconnect restoration is not implemented. |
| Staff count and snapped segments | Server owns online staff count and snap decisions. Clients rebuild staff visuals and receive a snap cue; loose snapped physics remains standalone-only. |
| Slosh and Stress | Server owns values and effects; clients receive readable mirrors. |
| Mug Run pickups and rewards | Server owns collection, respawn, staff growth, Slosh, reward grant, and Use Reward validation. |
| Arcane Pinball | Server owns spawn, movement truth, bounce count/speed-up, hit confirmation, effects, telemetry, and cleanup. Clients display replicated movement/readability. |
| Quick Bonk | Owned requests reach the server; server owns timing, overlap, knockback, Stress, and telemetry. |
| Staff Clash | Server owns eligibility, participants, countdown, mash acceptance, resolution, winner knockback, and cleanup. |
| Ring-out/respawn | Server owns bounds, pending respawn, transform, attribution, scoring effects, and cleanup. |
| Staffs powerup and Mega Staff | Server owns pickup, grant, temporary segments, expiration, snap accounting, and cleanup. |
| Grand Wizard Final | Server owns Candidate, vulnerability, steal eligibility/progress, timer, swap, winner, and reset. Clients display readable mirrors. |
| Gameplay event feed | Bounded, sequence-based GameState feed mirrors selected server events. It is display-only and clears at phase/generation boundaries. |
| Reset and actor lifecycle | Timers, delegates, collision, pickups, projectiles, combat state, powerups, Final state, HUD mirrors, and replicated actors have focused cleanup boundaries. |
| Transition presentation | Runtime arena phase visibility is an actor-owned replicated contract. Reset, presentation, and wizard staging responsibilities are separate. |

## Connection Paths

### PIE listen server

1. Set `Number of Players = 2`.
2. Set `Net Mode = Play As Listen Server`.
3. Use separate windows when practical.
4. Follow [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md) or the beginner-oriented [TwoClientListenServerTestGuide.md](TwoClientListenServerTestGuide.md).

### Direct-connect

Host map:

```text
/Game/Maps/WizardStaff_Prototype?listen
```

Client console:

```text
open 127.0.0.1
```

Port-qualified fallback: `open 127.0.0.1:7777`.

### Steam smoke helpers

Non-shipping commands on `UWizardStaffGameInstance`:

```text
WizardSteamHost
WizardSteamJoinFirstSession
```

The host advertises a small two-player session and opens the project map as listen server. The client helper searches compatible results, joins, resolves a connect string, and travels only after success. No browser or production failure UI exists.

## Map and Packaging History

- `/Game/Maps/WizardStaff_Prototype` is the project-owned Editor/Game/Server default map.
- The old `/Engine/Maps/Templates/OpenWorld` fallback is superseded and must not return.
- Runtime actors still create most prototype presentation; the map is intentionally minimal.
- A private Steam build exposed missing lighting. `AWizardStaffPrototypeLighting` now provides a runtime fallback, and a follow-up Steam playthrough verified the fix.
- SteamPipe configuration produced a private Windows build and depot manifest. Steamworks branch/package administration remains an external release operation, not gameplay code.

## Superseded or Historical Assumptions

| Historical assumption | Current replacement |
| --- | --- |
| Local-only prototype; no replication | Listen-server scaffolding exists while local workflows remain supported. |
| Two-Trial normal loop | Current loop includes Cauldron Catastrophe before the Final. |
| Engine OpenWorld startup fallback | Project-owned startup map. |
| AppID 480 local test strategy | Superseded by the real project Steam configuration. Do not restore 480 without an explicitly isolated test. |
| Readability-only Arcane Pinball shell | Server-owned gameplay projectile scaffold. |
| Controller iterator as player identity | Assigned PlayerState display slot. |
| Append every online prompt to this file | Current docs plus this condensed milestone record. |

## Known Gaps

- Valid Steam search/join/travel on two machines and two accounts.
- Steam leaderboard write, flush, and read-back.
- Cross-machine internet/NAT/firewall behavior and packaged Steam multiplayer QA.
- Disconnect/reconnect recovery, host migration, dedicated servers, and production join-failure UX.
- Prediction, rewind, lag compensation, exact cross-client physics, and replicated loose segment debris.

## Maintenance Rule

Update this file only for a major online milestone or changed authority/connection decision. Put current status in [CURRENT_STATE.md](CURRENT_STATE.md), active risks in [KNOWN_ISSUES.md](KNOWN_ISSUES.md), implementation ownership in [TECHNICAL_ARCHITECTURE.md](TECHNICAL_ARCHITECTURE.md), and repeatable checks in [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md).
