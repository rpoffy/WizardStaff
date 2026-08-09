# Known Issues and Verification Gaps

**Last Updated:** 2026-08-09

Items are categorized so untested work is not confused with a confirmed bug.

## Validation Gaps

| Item | Status | Why it matters |
| --- | --- | --- |
| Menu search cancellation, timeout, and late-callback handling | **Partially verified** | BuildID `24510908` successfully joined two accounts, but timeout messaging was not explicitly exercised. Current source adds guarded stale-session teardown plus an explicit host departure/rejoin contract; package and verify cancellation, timeout, active-Trial departure, Party Hall reset, and replacement-P2 join. |
| Menu Steam subsystem initialization | **Implemented and verified** | Steam-launched Build `24343969` completed a zero-compatible-session search and reported `No compatible game found` on 2026-07-22. The host path also created a waiting Party Hall session. Two-machine join remains unverified. |
| Local menu-to-gameplay input handoff | **Implemented and verified** | Menu travel had left its local controller in UI-only input mode with a focused widget. Current source removes the menu widget and restores game-only input immediately before Local, host, or join travel. `WizardStaffEditor` built and a human Local PIE full-loop playthrough verified keyboard control and normal gameplay on 2026-07-24. |
| Local-to-Online secondary player leakage | **Implemented but unverified** | Steam Build `24394181` preserved the Local P2/bot `ULocalPlayer` after returning to the menu, so a subsequent Host request entered Party Hall with an unintended second local participant. Inactive Build `24504173` trims all secondary local players immediately before Steam host/join setup, leaving Local With Bots and couch creation unchanged. The exact Local -> Escape -> Host sequence still needs a Steam retest. |
| Online Party Hall host-ready start gate | **Implemented but unverified** | Steam Build `24393938` verified that the host-only Party Hall timer remains frozen. Build `24394181` started after a host bell bonk, but an unintended local P2 satisfied the controller-count gate. Build `24504173` contains the leak cleanup; host-only bell rejection and real-P2 acceptance remain unverified. |
| Steam initial session search/join/travel between two accounts | **Implemented and verified** | BuildID `24510908` connected a host and remote friend through the in-game Steam flow on 2026-08-01. This verifies initial discovery/join/travel only, not reconnect or friends-list joining. |
| Versioned Steam session compatibility | **Implemented but unverified** | Host/search now use the Unreal network checksum plus `ProjectVersion=0.1.0-private.2` instead of permanent smoke constants. Version `.2` marks the vial-segment replication-layout change. Package one build and verify matching clients connect while an older private build is rejected with the beta/build mismatch message. Bump `ProjectVersion` for future network-incompatible packages. |
| Remote Cauldron vial-segment colors | **Implemented but unverified** | Remote staff rebuilds previously knew only segment count and rendered Speed/Burdening Power vial segments as generic alternating staff pieces. The server now replicates a compact visual type per segment position; clients recolor only and cannot infer tags or mutate the authoritative vial stack. Verify mixed ordering, bank/snap/spill removal, late visibility, and Trial cleanup in both listen-server windows. |
| Online snapped-segment cosmetic pop | **Implemented but unverified** | Online snaps previously showed the count loss, feed message, and staff shake but no detached segment moment. The existing replicated snap sequence now starts one 0.85-second local mesh arc per machine with collision, overlaps, navigation, replication, and physics disabled. Verify one cue per snap on host/client, cleanup on reset, and no duplicate cosmetic in standalone. Loose physics replication remains intentionally deferred. |
| Steam leaderboard write, flush, and read-back | **Implemented but unverified** | The write is now reachable only from an owner-only server result delivered to the exact local, non-bot PlayerState; unrelated code and mismatched proxies cannot call the private GameInstance seam. Verify one write per human owner, no bot/proxy write, flush success, KeepBest, and Steamworks read-back in a two-account private build. A client-mediated Steam leaderboard remains unsuitable as backend anti-cheat proof. |
| Automated test coverage | **Not found in repository inventory** | Current confidence relies on build output, manual PIE, direct-connect, and Steam smoke tests. |

## Confirmed Steam Playtest Issues

| Item | Status | Human observation / desired outcome |
| --- | --- | --- |
| Rejoin after leaving | **Implemented but unverified** | The joiner now destroys stale local `GameSession` state before searching. The host closes Steam advertisement and join-in-progress outside Party Hall, rejects direct late joins during gameplay, and treats a remote departure as an interrupted match: server gameplay is reset to a new generation in Party Hall and the session reopens for a replacement P2. The disconnected client is returned to the menu with a readable status. The 2026-08-09 editor build passed; two-account validation remains required. Identity/score restoration and true mid-Trial reconnect remain intentionally deferred. |
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
