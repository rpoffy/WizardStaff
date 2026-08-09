# Known Issues and Verification Gaps

**Last Updated:** 2026-08-04

Items are categorized so untested work is not confused with a confirmed bug.

## Validation Gaps

| Item | Status | Why it matters |
| --- | --- | --- |
| Menu search cancellation, timeout, and late-callback handling | **Partially verified** | BuildID `24510908` successfully joined two accounts, but timeout messaging was not explicitly exercised. Leaving and attempting to reconnect failed, including after another session was hosted, so retry and late-callback/session-cleanup behavior still require diagnosis. |
| Menu Steam subsystem initialization | **Implemented and verified** | Steam-launched Build `24343969` completed a zero-compatible-session search and reported `No compatible game found` on 2026-07-22. The host path also created a waiting Party Hall session. Two-machine join remains unverified. |
| Local menu-to-gameplay input handoff | **Implemented and verified** | Menu travel had left its local controller in UI-only input mode with a focused widget. Current source removes the menu widget and restores game-only input immediately before Local, host, or join travel. `WizardStaffEditor` built and a human Local PIE full-loop playthrough verified keyboard control and normal gameplay on 2026-07-24. |
| Local-to-Online secondary player leakage | **Implemented but unverified** | Steam Build `24394181` preserved the Local P2/bot `ULocalPlayer` after returning to the menu, so a subsequent Host request entered Party Hall with an unintended second local participant. Inactive Build `24504173` trims all secondary local players immediately before Steam host/join setup, leaving Local With Bots and couch creation unchanged. The exact Local -> Escape -> Host sequence still needs a Steam retest. |
| Online Party Hall host-ready start gate | **Implemented but unverified** | Steam Build `24393938` verified that the host-only Party Hall timer remains frozen. Build `24394181` started after a host bell bonk, but an unintended local P2 satisfied the controller-count gate. Build `24504173` contains the leak cleanup; host-only bell rejection and real-P2 acceptance remain unverified. |
| Steam initial session search/join/travel between two accounts | **Implemented and verified** | BuildID `24510908` connected a host and remote friend through the in-game Steam flow on 2026-08-01. This verifies initial discovery/join/travel only, not reconnect or friends-list joining. |
| Steam leaderboard write, flush, and read-back | **Implemented but unverified** | Existing code queues the Favor result only for an active Steam session. Private Steam validation is still required. |
| Automated test coverage | **Not found in repository inventory** | Current confidence relies on build output, manual PIE, direct-connect, and Steam smoke tests. |

## Confirmed Steam Playtest Issues

| Item | Status | Human observation / desired outcome |
| --- | --- | --- |
| Rejoin after leaving | **Implemented but unverified** | The joining process retained a local named `GameSession`, so a later join could collide with stale OnlineSubsystem state. The join path now destroys that local session before searching, continues through a guarded callback, and cancels late teardown continuations on timeout/cancel. The editor build passed; two-account validation remains required. |
| Intermission ring-out recovery and camera | **Implemented and verified** | A two-account test found that a player leaving the Party Hall floor fell forever while the camera followed beneath the map. The server-owned Party Hall safety recovery and below-floor camera exclusion were subsequently human-verified on 2026-08-01. |
| Party Hall board dynamic values | **Implemented but unverified** | Root cause was explicit: the Party Hall actor replicated, but server-side `UTextRenderComponent::SetText` calls did not. The actor now replicates four server-owned text snapshots and clients apply them visually. The editor build passed; verify all dynamic values in both host and joining-client windows. |
| Online movement/facing smoothness | **Implemented but unverified** | The joining player experienced stronger jitter, clunkier control, and unreliable visual facing. Two concrete correction sources were fixed without retuning: facing now uses CharacterMovement control rotation rather than a separate yaw RPC, and autonomous movement uses replicated Slosh modifiers instead of predicting as sober while the server applies Slosh penalties. The editor build passed; two-machine sober/Sloshed movement and rapid-facing validation remains required before closing this issue. |
| Mouse sensitivity differs by network role | **Implemented but unverified** | Root cause in the shared input path was frame dependence: mouse delta was multiplied by `DeltaSeconds` as though it were keyboard/controller rate input. Mouse X now uses a dedicated frame-independent path preserving the approximate prior 60 FPS baseline. Verify comparable travel/feel on host and joiner; remaining facing jitter belongs to the separate movement/facing diagnosis. |
| Keyboard movement rotates with wizard aim | **Implemented but unverified** | Tester feedback identified actor-relative WASD as janky because mouse turning also rotated the movement frame. Primary WASD/arrow axes now use the top-down camera yaw while mouse/Q/E continue aiming the wizard and staff. The editor build passed on 2026-08-04; verify diagonal movement, Slosh, sticky/slippery effects, bonks, and host/joiner behavior before closing. |
| Escape behavior during gameplay | **Implemented but unverified** | Escape/controller-menu now opens a local gameplay submenu with Resume, Controls, and explicit Return to Main Menu. Opening it suppresses only that local controller's gameplay input; online authority and match time continue. The editor build passed; verify input restoration, control-panel back behavior, and local/host/joiner return travel. |

## Design Tensions

| Tension | Status | Required care |
| --- | --- | --- |
| Emergent physics versus scripted staff obstruction recovery | **Open design question** | The desired game identity favors natural staff problems, while current code contains stuck-state and recovery/failsafe behavior. Do not silently remove it; gather playtest evidence and obtain explicit direction. |
| Steam branch history versus production readiness | **Expected limitation** | A private build can install and run while Steam join, stats, tester access, store presence, and release requirements remain incomplete. |

## Not Bugs: Intentional Deferrals

The following absences are deliberate, not defects: public lobby browser, matchmaking, friend invites, production lobby UI, reconnect UX, host migration, dedicated servers, replicated loose segment physics, prediction/rewind/lag compensation, final UI/cosmetics/progression, and extra approved gameplay content.

For test steps, see [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md). For suggested sequencing, see [ROADMAP.md](ROADMAP.md).
