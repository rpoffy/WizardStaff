# Wizard Staff Technical Architecture

**Last Updated:** 2026-07-16

## Project Baseline

- **Engine:** Unreal Engine 5.7 C++ project.
- **Module:** `Source/WizardStaff` runtime module.
- **Startup map:** `/Game/Maps/WizardStaff_Prototype` (`Content/Maps/WizardStaff_Prototype.umap`).
- **Default classes:** `AWizardStaffGameMode` and `UWizardStaffGameInstance` are configured in `Config/DefaultEngine.ini`.
- **Presentation approach:** the project-owned map is intentionally minimal; Party Hall, arenas, ritual/readability actors, and a fallback lighting actor are largely runtime-owned.
- **Automated tests:** no dedicated automated test suite was found in the repository inventory. Validation is build, PIE, direct-connect, Steam smoke, and human playtesting.

## Main Runtime Ownership

| System | Primary owner | Notes |
| --- | --- | --- |
| Match flow, Trials, timers, scoring, Favor, Final winner, reset | `AWizardStaffGameMode` on server | The authority boundary for local and online play. |
| Replicated match/readability data and bounded event feed | `AWizardStaffGameState` | Mirrors are display/readability data; they must not initiate gameplay on clients. |
| Per-player replicated identity/readability | `AWizardStaffPlayerState` | Display slot/color and score-style mirrors support remote HUDs. |
| Steam session and leaderboard smoke helpers | `UWizardStaffGameInstance` | Dev-oriented host/find/join commands; result submission scaffold. |
| Wizard movement/combat/readability | `AWizardStaffWizardCharacter` | Owns wizard-side replicated state and server RPC seams. |
| Staff geometry, segments, stress, snaps | `UWizardStaffComponent` | Runtime visual/collision chain, local loose snapped segment behavior, and replicated count/readability integration. |
| HUD and framing | `AWizardStaffHUD`, `AWizardStaffSharedCamera` | Canvas/debug-style HUD and top-down shared camera. |

## Session Modes

`EWizardPrototypeSessionMode` separates setup behavior:

- `LocalPrototype`
- `LocalWithBots`
- `OnlineListenServer`
- `OnlineClient`

This is setup/diagnostic gating, not a production session architecture. Local player creation, keyboard fallback, and playtest bot fill must remain local-only. Direct-connect and Steam-hosted listen servers use the online host mode; joining clients use `OnlineClient`.

## Networking Model

The intended model is a listen server:

- **GameMode/server owns gameplay truth:** match and Trial states, timers, player slots, score/Favor, pickups/rewards, combat outcomes, respawns, Mega Staff, snapping, Final state, lifecycle cleanup, and event publication.
- **GameState/PlayerState/character fields replicate readable mirrors:** player rows, staff count, Slosh/Stress, reward state, combat/Final labels, pickup state, and a bounded gameplay event feed.
- **Clients request actions through existing server seams; they do not submit hit targets, bounce claims, scores, snaps, or Final outcomes.**
- **Visual-only data must remain visual-only.** HUD, runtime presentation actors, replicated events, and map state cannot grant gameplay authority.

### Mirror Synchronization

GameMode still gathers authoritative readable state at its established boundaries and end-of-tick fallback, but mirror setters are dirty-checked:

- `AWizardStaffPlayerState::SetWizardPlayerMirror` returns whether any clamped mirror field changed. GameMode calls `ForceNetUpdate` only for a changed mirror, not every server tick.
- `AWizardStaffGameState` assigns enum, index, boolean, and text mirrors only when values differ.
- Display-only countdown/remaining-time mirrors update at a minimum 0.1-second delta; Final/Cauldron progress mirrors update at a minimum 1% delta, with zero/nonzero endpoints applied immediately.
- Clearing an already-empty replicated gameplay feed is a no-op rather than creating a redundant event sequence and forced update.

These are bandwidth/readability guards only. The authoritative timers and progress continue updating at their existing server cadence and still drive gameplay exclusively in GameMode.

### Player Slot Identity

`AWizardStaffPlayerState::WizardDisplaySlot` is the stable runtime identity source after assignment. GameMode preserves an existing slot and allocates the first unused nonnegative slot only for a newly observed PlayerState. Wizard lookup, PlayerState lookup, controller/spawn indexing, score/Favor attribution, and Arcane Pinball labels prefer that assigned slot.

`APlayerState::PlayerId` and player-controller iterator order remain narrow compatibility fallbacks for setup before a display slot exists. They are not the normal gameplay identity source and must not overwrite an assigned slot during pawn restart or match reset. A two-player listen-server PIE session human-verified this slot-first behavior on 2026-07-15. This cleanup does not implement reconnect identity restoration, account identity, or persistent player reservations.

For the condensed online milestone record, see [OnlineMultiplayerArchitecturePlan.md](OnlineMultiplayerArchitecturePlan.md); superseded decisions remain in [DESIGN_DECISIONS.md](DESIGN_DECISIONS.md).

## Runtime Actors

Current runtime actor categories include:

- `WizardStaffPrototypeArena`, `WizardStaffStaffsAtDawnArena`, and `WizardStaffPartyHall`
- `WizardStaffManaMugPickup`, `WizardStaffStaffsAtDawnPowerupPickup`
- `WizardStaffArcanePinballProjectile`
- `WizardStaffFinalRitualCircle`
- `WizardStaffCauldronArena`, `WizardStaffCauldronVialPickup`, `WizardStaffCauldronIngredient`, `WizardStaffCauldronHazard`, and `WizardStaffCauldronDepositArc`
- `WizardStaffPrototypeLighting`

Actor lifecycle cleanup is a specific project concern: trial transitions, rematch/reset, destruction, timers, overlap delegates, hidden/collision states, and match generation boundaries must remain safe. See [KNOWN_ISSUES.md](KNOWN_ISSUES.md) and [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md).

## Map and Presentation Behavior

The current startup map replaced the old engine OpenWorld fallback. Runtime actors remain responsible for prototype presentation. The project has a fallback lighting actor because a Steam-delivered build exposed insufficient lighting in the minimal map; a follow-up private build was reported as visually fixed.

Current runtime spatial separation places prototype, Staffs at Dawn, Cauldron, and Party Hall at distinct offsets. Mug Run and Grand Wizard Final both reuse the prototype arena: Party Hall hides it, Mug Run reveals it before explicit current-phase staging, and Final restores it before ritual-circle/player setup. Cauldron owns a separate runtime arena retained through its following intermission to avoid teardown/teleport races.

Authored `AWizardStaffPrototypeArena` and `AWizardStaffStaffsAtDawnArena` actors are preferred when their GameMode flags are enabled; runtime classes remain fallbacks for the minimal startup map. Blueprint marker prefixes are part of the editing contract: `PlayerSpawn*`, `MugSpawn*`, and `FuturePowerupSpawn*`. Marker names are sorted for predictable order. Arena `ArenaHalfSize` and Staffs' relative fall distance feed ring-out/respawn bounds, so layout edits must keep those values aligned with real playable surfaces.

`AWizardStaffPrototypeArena` owns an explicit replicated phase-presentation flag. GameMode authority changes that flag at phase boundaries; the actor applies it locally and through an idempotent `OnRep` to its block visibility and collision. Initial `BeginPlay` application and replicated state support late joins. This is a presentation/collision-readability contract only: clients cannot choose phases or use the flag to drive match gameplay.

### Transition Responsibilities

`AWizardStaffGameMode` keeps three transition operations separate:

- Trial-specific reset helpers reset their owned state and actors; Mug Run reset does not reset or place wizards.
- `SetPrototypeArenaPhasePresentationActive` expresses the server-selected presentation state directly and delegates replicated application to the arena actor.
- `StageWizardsForCurrentPhase` performs destination placement through the existing phase-aware spawn transform only when the caller has made the destination safe.

Wizard match-setup reset remains explicit and state-only. Party Hall, normal Trial countdowns, Cauldron activation, and defensive Trial-start fallbacks invoke staging at their own established boundaries. This is a responsibility cleanup around the existing state machine, not a match-flow redesign.

## Steam and Direct-Connect

### Direct-connect

**Implemented and verified.** Host opens the project map with `?listen`; client uses `open 127.0.0.1` or port-qualified equivalent. This remains the fast debugging fallback and must not become Steam-dependent.

### Steam smoke integration

**Partially implemented.** `OnlineSubsystemSteam` is enabled and uses the real app configuration currently stored in `DefaultEngine.ini`. `UWizardStaffGameInstance` exposes non-shipping commands including `WizardSteamHost` and `WizardSteamJoinFirstSession`.

The host path creates a two-player advertised lobby/session with map/build metadata, then opens the project map as a listen server. The join helper searches compatible lobby results, joins the first compatible result, resolves a connection string, and travels only after success.

A host session was observed. Same-machine multi-process joining encountered Steam client/server API limitations. Two-machine/two-account validation is still required before treating Steam join as ready. Steam is discovery/connection only; it must not alter gameplay authority.

### Leaderboard scaffold

`SubmitAuthoritativeSteamMatchResult` queues a KeepBest descending write to `WizardStaff_BestGrandWizardFavor` only for an active Steam session and guards against duplicate match-generation submissions. It is **implemented but unverified** until a private Steam build proves queue, flush, and Steamworks read-back behavior.

## Build and Configuration Notes

- `WizardStaff.Build.cs` depends on OnlineSubsystem and OnlineSubsystemUtils and dynamically loads OnlineSubsystemSteam.
- The active Steam development/release AppID is in configuration. Do not replace it with AppID 480 or publish secrets in docs.
- Runtime physics values that require body/physical-material access must be applied after engine initialization. `AWizardStaffCauldronIngredient` keeps its component defaults in the constructor and applies the unchanged 16 kg authority mass override in `BeginPlay`; this is required for UE 5.7 commandlet cooking.
- SteamPipe and branch work belong to Steamworks operations, not gameplay code. See [CURRENT_STATE.md](CURRENT_STATE.md) for evidence boundaries.
