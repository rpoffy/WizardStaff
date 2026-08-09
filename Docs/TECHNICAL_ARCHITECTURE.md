# Wizard Staff Technical Architecture

**Last Updated:** 2026-08-04

## Project Baseline

- **Engine:** Unreal Engine 5.7 C++ project.
- **Module:** `Source/WizardStaff` runtime module.
- **Startup map:** `/Game/Maps/WizardStaff_MainMenu` (`Content/Maps/WizardStaff_MainMenu.umap`). It is a minimal project-owned frontend map with no gameplay arena ownership.
- **Gameplay map:** `/Game/Maps/WizardStaff_Prototype` (`Content/Maps/WizardStaff_Prototype.umap`). Its map-prefix GameMode override keeps the established gameplay GameMode for local, direct-connect, and listen-server travel.
- **Default classes:** `AWizardStaffMainMenuGameMode`, `AWizardStaffGameMode` (prototype-map override), and `UWizardStaffGameInstance` are configured in `Config/DefaultEngine.ini`.
- **Presentation approach:** the project-owned map is intentionally minimal; Party Hall, arenas, ritual/readability actors, and a fallback lighting actor are largely runtime-owned.
- **Automated tests:** no dedicated automated test suite was found in the repository inventory. Validation is build, PIE, direct-connect, Steam smoke, and human playtesting.

## Main Runtime Ownership

| System | Primary owner | Notes |
| --- | --- | --- |
| Match flow, Trials, timers, scoring, Favor, Final winner, reset | `AWizardStaffGameMode` on server | The authority boundary for local and online play. |
| Replicated match/readability data and bounded event feed | `AWizardStaffGameState` | Mirrors are display/readability data; they must not initiate gameplay on clients. |
| Per-player replicated identity/readability | `AWizardStaffPlayerState` | Display slot/color and score-style mirrors support remote HUDs. |
| Steam session and leaderboard smoke helpers | `UWizardStaffGameInstance` | Dev-oriented host/find/join commands; result submission scaffold. |
| Private-playtest frontend | `AWizardStaffMainMenuGameMode`, `AWizardStaffMainMenuPlayerController`, `UWizardStaffMainMenuWidget` | C++ UMG menu only; it starts local travel or delegates to the existing Steam host/find/join helpers. |
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

Legacy axis bindings intentionally separate input semantics. `KeyboardMoveForward`/`KeyboardMoveRight` use the local top-down camera yaw, so WASD and arrow movement remain stable while the wizard aims independently. `MoveForward`/`MoveRight` preserve the existing actor-relative left-stick path and the same-keyboard fallback remains unchanged. `MouseTurn` is a frame-independent mouse-delta path, while `Turn` remains a delta-time-scaled rate path for Q/E and the right stick. All paths retain the same Slosh, obstruction, reaction, and Staff Heft modifiers. The locally controlled wizard writes resulting aim yaw to controller control rotation, which CharacterMovement already carries in client moves; `FaceRotation` applies it immediately and retains bounded server validation for remote controllers. Respawn placement synchronizes actor and control yaw.

Authoritative `ManaSlosh` remains server-owned. Autonomous clients use the replicated Slosh mirror only when calculating local CharacterMovement/input modifiers so their speed, acceleration, braking, and turn prediction agrees with the server between mirror updates. Simulated/readability state still cannot apply gameplay or mutate Slosh.

## Map and Presentation Behavior

The project-owned frontend and gameplay maps replaced the old engine OpenWorld fallback. Runtime actors remain responsible for prototype presentation. The project has a fallback lighting actor because a Steam-delivered build exposed insufficient lighting in the minimal gameplay map; a follow-up private build was reported as visually fixed.

Current runtime spatial separation places prototype, Staffs at Dawn, Cauldron, and Party Hall at distinct offsets. Mug Run and Grand Wizard Final both reuse the prototype arena: Party Hall hides it, Mug Run reveals it before explicit current-phase staging, and Final restores it before ritual-circle/player setup. Cauldron owns a separate runtime arena retained through its following intermission to avoid teardown/teleport races.

Party Hall has a separate safety recovery from scored Trial ring-outs. During active intermission, GameMode immediately returns an out-of-bounds wizard to that controller's Party Hall spawn and does not publish ring-out events, telemetry, score, or Favor. The shared camera uses replicated Party Hall phase readability plus the local hall actor's floor height to stop tracking a wizard below the floor while the authoritative teleport replicates.

Party Hall board values use an explicit presentation replication contract. GameMode remains the sole author of standings, next-Trial/countdown, preset, and leader strings; `AWizardStaffPartyHall` replicates the latest snapshots and its RepNotify applies them to client text components. The strings are readability-only, dirty-checked by Unreal replication, and never feed gameplay decisions.

Authored `AWizardStaffPrototypeArena` and `AWizardStaffStaffsAtDawnArena` actors are preferred when their GameMode flags are enabled; runtime classes remain fallbacks for the minimal startup map. Blueprint marker prefixes are part of the editing contract: `PlayerSpawn*`, `MugSpawn*`, and `FuturePowerupSpawn*`. Marker names are sorted for predictable order. Arena `ArenaHalfSize` and Staffs' relative fall distance feed ring-out/respawn bounds, so layout edits must keep those values aligned with real playable surfaces.

`AWizardStaffPrototypeArena` owns an explicit replicated phase-presentation flag. GameMode authority changes that flag at phase boundaries; the actor applies it locally and through an idempotent `OnRep` to its block visibility and collision. Initial `BeginPlay` application and replicated state support late joins. This is a presentation/collision-readability contract only: clients cannot choose phases or use the flag to drive match gameplay.

### Transition Responsibilities

`AWizardStaffGameMode` keeps three transition operations separate:

- Trial-specific reset helpers reset their owned state and actors; Mug Run reset does not reset or place wizards.
- `SetPrototypeArenaPhasePresentationActive` expresses the server-selected presentation state directly and delegates replicated application to the arena actor.
- `StageWizardsForCurrentPhase` performs destination placement through the existing phase-aware spawn transform only when the caller has made the destination safe.

Wizard match-setup reset remains explicit and state-only. Party Hall, normal Trial countdowns, Cauldron activation, and defensive Trial-start fallbacks invoke staging at their own established boundaries. This is a responsibility cleanup around the existing state machine, not a match-flow redesign.

## Steam and Direct-Connect

### Private-playtest menu

**Partially implemented.** The default map now shows a small C++ UMG menu rather than starting a match immediately. `Play Local` opens the existing prototype map. `Host Online Game` and `Join Online Game` call the existing `UWizardStaffGameInstance` Steam helper paths, with a small request-state/token guard for stale callbacks and a cancel action for search/join. The menu owns no match, lobby, or gameplay authority; the prototype map retains the existing GameMode/server authority boundary.

The menu is intentionally a private-playtest frontend, not production UI. It supports mouse plus explicit keyboard/gamepad selection, displays a short controls panel and build label, and does not replace console/direct-connect debugging paths. Before Local, host, or join travel, `UWizardStaffGameInstance` asks the menu controller to remove its widget and restore game-only input; this prevents the menu's UI-only focus from surviving into gameplay. `ReturnToMenu` remains bound on the locally controlled wizard's proven gameplay input component, but now delegates to `AWizardStaffPlayerController`, which opens a local UMG submenu with Resume, Controls, and explicit Return to Main Menu. The submenu uses UI-only focus and paired local move/look suppression; it does not pause or mutate the server match. Explicit Return retains `UWizardStaffGameInstance::ReturnToMainMenu`, including Steam session teardown before travel. Steam Build `24394181` human-verified the older direct Local/hosted return route; the new submenu builds but awaits human validation. Current source also trims secondary local players before every Steam host/join request, preserving Local With Bots and couch creation while preventing local participants from leaking into online setup. Earlier Builds `24346104` and `24346698` remain superseded.

For online Party Hall only, `AWizardStaffGameMode` freezes the intermission timer without locking wizard input. Before a second controller joins, the bell reports that the host must wait. Once two controllers are present, only P1/the listen host may start the normal short countdown by bonking the existing Ready Bell. The server validates that request; the bell, HUD feedback, PlayerState ready mirror, and replicated state remain readable only. Local readiness retains its all-active-player behavior. Steam Build `24393938` verified the host-only frozen timer; the two-player Ready Bell start path still needs human validation.

### Direct-connect

**Implemented and verified.** Host opens the project map with `?listen`; client uses `open 127.0.0.1` or port-qualified equivalent. This remains the fast debugging fallback and must not become Steam-dependent.

### Steam smoke integration

**Partially implemented.** `OnlineSubsystemSteam` is enabled and uses the real app configuration currently stored in `DefaultEngine.ini`. UE 5.7 Steam lobby sessions resolve P2P addresses in `steam.<id>` form, so packaged Steam networking now selects `SteamSocketsNetDriver`; `IpNetDriver` remains the fallback when SteamSockets is unavailable, preserving non-Steam/editor/direct-connect diagnosis.

The host path creates a two-player advertised lobby/session with map/build metadata, then opens the project map as a listen server. The join helper requests Steam lobbies broadly, logs and locally filters map/build metadata, joins the first compatible result, resolves a connection string, and travels only after success. Before a retry search, it destroys any stale local named `GameSession`; the matching destroy callback alone may continue into the new search, and cancellation/timeout invalidates that continuation. Search and join each have a 30-second frontend deadline. Network/travel failures update the persistent menu status rather than leaving an indefinite `Joining` state.

On 2026-08-01, BuildID `24510908` completed a real two-machine/two-account initial host/search/join connection through the in-game buttons. Rejoining after departure failed in that build; the local named-session teardown correction now builds but still needs the same two-account retry. Steam friends-list invite acceptance is not wired and is not evidence for the supported in-game Join button. Steam remains discovery/connection only and must not alter gameplay authority.

### Leaderboard scaffold

`SubmitAuthoritativeSteamMatchResult` queues a KeepBest descending write to `WizardStaff_BestGrandWizardFavor` only for an active Steam session and guards against duplicate match-generation submissions. It is **implemented but unverified** until a private Steam build proves queue, flush, and Steamworks read-back behavior.

## Build and Configuration Notes

- `WizardStaff.Build.cs` depends on OnlineSubsystem, OnlineSubsystemUtils, UMG, Slate, and SlateCore; it dynamically loads OnlineSubsystemSteam. The project also enables Epic's UE 5.7 `SteamSockets` plugin and configures its net driver with an `IpNetDriver` fallback.
- The active Steam development/release AppID is in configuration. Do not replace it with AppID 480 or publish secrets in docs.
- Runtime physics values that require body/physical-material access must be applied after engine initialization. `AWizardStaffCauldronIngredient` keeps its component defaults in the constructor and applies the unchanged 16 kg authority mass override in `BeginPlay`; this is required for UE 5.7 commandlet cooking.
- SteamPipe and branch work belong to Steamworks operations, not gameplay code. See [CURRENT_STATE.md](CURRENT_STATE.md) for evidence boundaries.
