# Wizard's Staff Online Multiplayer Architecture Plan

Last updated: July 3, 2026

Scope: this is an online multiplayer readiness audit and migration plan for the current local multiplayer prototype. It is not an implementation pass. Do not treat this document as permission to add Steam lobbies, matchmaking, a third Trial, new spells, new powerups, Hammer Time, final UI, progression, or broad gameplay retuning.

Current release direction: Wizard's Staff should eventually support full online multiplayer with player-hosted lobbies, Steam friend invites, public joinable lobbies, match flow for multiple players, and possible bot or empty-slot helpers. The current locked slice must remain playable locally while online support is introduced.

## 1. Current Local-Only Assumptions

The codebase is still local-first. A first replication scaffold now exists, but the current playable loop still works because all important gameplay is running in one process with direct object references. The scaffold should be treated as observable mirror state for future clients, not as online gameplay support yet.

Implemented scaffold as of July 3, 2026:

- `AWizardStaffGameState` exists and is configured on `AWizardStaffGameMode`.
- `AWizardStaffPlayerState` exists and is configured on `AWizardStaffGameMode`.
- GameMode remains authoritative and mirrors selected state into GameState/PlayerState.
- The Canvas HUD prefers replicated mirrors for safe header/player-row fields when available, while preserving local GameMode fallbacks.
- Basic readable gameplay mirrors now exist for staff segment count, Mana Slosh, Staff Stress, Mug Run mug pickup active/respawn state, carried brew reward display, Staffs at Dawn powerup pickup active/respawn state, Mega Staff active state, Grand Wizard Final Candidate/steal readability, and a small replicated gameplay event feed.
- Arcane Pinball now has server-owned network gameplay scaffolding for validated reward use, projectile spawn, bounce/speed-up, hit confirmation, hit effects, replicated movement readability, and cleanup.
- Risky gameplay systems are still mostly deferred.

Important local-only assumptions found in the current code:

- `AWizardStaffGameMode` owns most match state directly: party state, Trial state, timers, scores, Favor, Final Candidate, ready state, spawned mugs, Staffs at Dawn powerups, loose snapped segments, telemetry, and rematch cleanup.
- GameMode is server-only in Unreal networking. Any state currently read by HUD, clients, or world visuals will need a replicated mirror, likely in a new `AWizardStaffGameState` and a custom `AWizardStaffPlayerState`.
- Player identity is usually local index based. `GetPlayerIndexForWizard`, `GetWizardForPlayerIndex`, `GetControllerIndex`, `GetCurrentWizards`, `PlayerRoundWins`, `PlayerGrandWizardFavor`, `StaffsAtDawnScores`, ready arrays, and telemetry arrays all assume a stable 0-based player slot.
- Several systems fall back to `PlayerState->GetPlayerId()` for display or attribution. That is acceptable for local display, but should not become the authoritative online identity model.
- `EnsureLocalPlayers`, `DesiredLocalPlayerCount`, `DesiredHumanPlayers`, `DesiredTotalPlayers`, keyboard fallback controls, shared camera assignment, and playtest bot filling are local-session tools.
- The shared camera follows all local PlayerControllers. Online clients will likely need their own camera behavior, even if local split/shared-camera mode remains.
- HUD still reads from local `GetAuthGameMode` for detailed local/debug panels, telemetry summary, feedback messages, and some richer scoring clarity panels. Playtest/Minimal headers, compact player rows, and core Final Candidate/steal readability now have safer GameState/PlayerState mirror fallbacks, but remote-client HUD support is not complete.
- Wizard input is bound directly on `AWizardStaffWizardCharacter`. Movement may partially benefit from `ACharacter` replication later, but actions like Quick Bonk, Use Reward, Staff Clash mash, and debug actions need ownership-aware RPC seams.
- Quick Bonk is contact-driven using the current staff collision box at the moment of impact. It directly overlaps actors and applies reactions in the same process.
- Staff segments are runtime component chains on `UWizardStaffComponent`; `SegmentCount`, `StaffStress`, obstruction state, stuck state, and snapped segment spawning are not replicated.
- Loose snapped segments are spawned as runtime physics actors and tracked by GameMode. They are funny but high-risk online because physics diverges easily.
- Arcane Pinball has local standalone gameplay plus server-owned listen-server gameplay scaffolding. The server owns validated cast, movement truth, bounces, speed-up, hit confirmation, Slosh/Stress/knockback effects, telemetry, and cleanup; clients display replicated projectile movement/trails and event-feed messages only.
- Mug Run mug pickups and Staffs at Dawn powerup pickups now replicate active/hidden/respawn readability and validate collection on the server for listen-server smoke testing.
- Mega Staff Brew has server-owned effect scaffolding and replicated active/remaining/temp-segment readability, but exact online polish and full cross-client effects still need hardening.
- Staff Clash now has server-owned request/state scaffolding, but exact cross-client combat fidelity, prediction, and stale-state testing remain important.
- Ring-out detection, respawn delay, score credit, broom boost save telemetry, and staff reset on ring-out are GameMode-driven with replicated respawn/readability scaffolding.
- Rematch/reset cleanup has a focused hardening pass, including explicit mug pickup respawn-timer teardown on actor end play.
- Playtest bots are component-driven helpers that use existing wizard input/action paths but are still local prototype helpers, not network AI architecture.
- Debug controls and console-friendly functions are intentionally local/dev tools. They should be disabled or host-authority gated online.

## 2. Server Authority Plan

Online Wizard's Staff should use a server-authoritative gameplay model. A listen server is the likely first target because the final release direction includes player-hosted lobbies. Clients should predict or display where useful, but the server should own final truth.

Recommended authority ownership:

- Match state: server authoritative in GameMode, replicated observable state in GameState.
- Trial state: server authoritative in GameMode, replicated to GameState for HUD/world displays.
- Timers: server authoritative; clients display replicated remaining times or synchronized start/end timestamps.
- Scoring: server authoritative. Trial scores, round wins, and final winner should live in replicated GameState/PlayerState mirrors.
- Grand Wizard Favor: server authoritative, stored per player in PlayerState or a replicated player-stat struct.
- Final Candidate: server authoritative, replicated through GameState. Candidate swaps should be server events with replicated feedback.
- Ready Bell state: server authoritative. Clients send a ready-bell bonk/request only through validated staff contact or a server-side hit result.
- Mug pickup collection: server authoritative overlap/collection. Pickup active state and respawn timing should replicate.
- Staff segment count: server authoritative. Segment count should replicate to all clients and rebuild client visuals through `OnRep`.
- Mana Slosh: server authoritative for gameplay. Replicate enough to drive movement penalties, HUD meters, and visual wobble.
- Staff Stress: server authoritative. Replicate enough for HUD meters, rattle feedback, and snap warnings.
- Staff snapping: server authoritative. Server decides stress threshold and snap event, decrements segment count, replicates visual rebuild, and spawns loose segment actor if enabled.
- Loose segment spawning/cleanup: server authoritative. Replicate actor spawn/destruction and either keep physics modest or simulate locally as cosmetic after server-confirmed spawn.
- Arcane Pinball spawning/hits/cleanup: server authoritative for spawn, bounces that affect speed/count, hit confirmation, stress/slosh/knockback, and destruction. Clients can display replicated movement/trails.
- Mega Staff Brew pickup/effect/expiration: server authoritative. Replicate active state, remaining time or end timestamp, temporary segment count, and cleanup.
- Staff Clash start/resolve: server authoritative. Server validates timing, direction, participants, locks both wizards, counts mash RPCs, resolves winner/tie, and applies the winning bonk.
- Ring-out detection and respawn: server authoritative. Server owns out-of-bounds checks, credit attribution, respawn timers, score/Favor/staff reset, and final transforms.
- Rematch/reset boundaries: server authoritative. Reset should be a single state-machine transition that clears replicated actors and per-player replicated state predictably.

Recommended replicated architecture:

- Add `AWizardStaffGameState` for party/trial state, active Trial type, timers, current preset display name, Final Candidate state, winner messages, ready counts, world event feed, and lightweight replicated score summaries.
- Add `AWizardStaffPlayerState` for stable player identity, display index/color, round wins, Grand Wizard Favor, Trial score, ready state, bot flag if needed, and match-summary values that clients need.
- Keep `AWizardStaffGameMode` as the server-side orchestrator, but stop expecting clients to query it directly.
- Keep `AWizardStaffWizardCharacter` as the replicated pawn/character. Add server RPCs for player actions and replicated properties/RepNotifies for staff/slosh/stress/reward/powerup/clash state.
- Keep local-only helpers behind explicit local/offline checks so the current prototype workflow remains available.

## 3. Replication Risk List

Ranked from hardest/highest risk to more straightforward:

1. Staff collision and bonk hit confirmation
   - Why risky: bonk is currently based on a live attached staff collision box, local overlap timing, staff length, slosh/heft effects, and contact at the end of the animation. Client and server can disagree about staff pose, target location, and impact timing.
   - Safer approach: client sends `ServerRequestQuickBonk`; server owns bonk start/impact time and performs the staff overlap at the impact moment. Replicate bonk state for visuals. Do not trust client hit claims in the first online pass.

2. Staff Clash
   - Why risky: clash depends on near-simultaneous bonks, facing/direction thresholds, mash counts, position locks, hit immunity, broom boost cancellation, and a strong resolved knockback.
   - Safer approach: server alone detects/starts clashes from active server bonk states, records participants, locks movement server-side, accepts mash RPCs only from owning clients, replicates clash state, and resolves once.

3. Loose physics segments
   - Why risky: loose segments are intentionally chaotic physics actors. Fully deterministic networked physics is expensive and fragile, and the bug-like launches are part of the identity.
   - Safer approach: server spawns and owns loose segments. Replicate initial impulse and lifetime. Keep chaos effects server-authoritative. Consider making far-away loose segment motion cosmetic or lower fidelity for clients. Preserve local comedy, but avoid requiring exact cross-client physics.

4. Arcane Pinball projectiles
   - Why risky: projectile bounces, accelerates after every bounce, can self-hit, applies stress/slosh/knockback, has max bounces/lifetime, and locks to launch height.
   - Safer approach: server spawns projectile and owns hit/bounce logic. Use replicated projectile movement or a custom replicated transform/speed. Clients display trails and impact feedback from replicated state.

5. Mega Staff temporary segments
   - Why risky: temporary segments are real staff segments that can snap during the timed effect. Expiration must remove only remaining temporary segments without deleting earned permanent segments.
   - Safer approach: server tracks permanent vs temporary segment counts. Replicate total segment count plus Mega Staff active/end-time state. Expire server-side and rebuild client visuals from replicated count.

6. Ring-out attribution
   - Why risky: credit windows use recent bonk state and respawn timing. Broom boost can save or fail during the window. Network delay can make "who caused this" feel unfair if client-side.
   - Safer approach: server records last valid attacker and timestamp on bonk hit, owns out-of-bounds checks, and awards score/Favor/staff growth only after server-confirmed respawn.

7. Final Candidate stealing
   - Why risky: circle occupancy, Candidate vulnerable/safe state, steal progress, and knockback timing must be readable and fair.
   - Safer approach: server owns circle checks and steal progress. Replicate candidate index, vulnerable/safe status, stealing player index, progress alpha, final timer, and winner.

8. Rematch cleanup
   - Why risky: the local loop already has many cleanup boundaries: mugs, Arcane Pinball, Mega Staff, Staff Clash, broom boost, loose segments, candidate state, Favor, scores, bots, and HUD messages. Online adds actor replication/destruction ordering.
   - Safer approach: create explicit reset phases and server cleanup functions before allowing clients to re-enter Party Hall. Replicate a match generation ID to ignore stale client events.

9. Bots
   - Why risky: bots currently fill local missing players and drive input/action paths. Online bots create ownership, identity, and authority questions.
   - Safer approach: keep bots server-owned for online. For the first online spike, disable bot filling unless deliberately testing server bot slots. Preserve local bot workflow separately.

## 4. Minimal Online Spike Target

The smallest useful online spike should prove that the locked slice can become network-aware without solving Steam, matchmaking, or every chaos system.

Spike goal:

- Host starts a listen server.
- A second client joins through a direct or simple session test method.
- Two wizards spawn.
- Each client controls only their own wizard.
- Movement and turning replicate acceptably.
- Quick Bonk replicates enough that one client can bonk the other and both clients see the result.
- Staff segment count replicates and rebuilds visible staff segments on both clients.
- One mug pickup collection replicates: pickup disappears/respawns and the collecting wizard gains a staff segment/slosh.
- One staff snap replicates: segment count decreases and clients see the snapped/removed top segment. Loose physics actor can be simplified if needed.
- One Arcane Pinball cast either replicates with server-owned projectile/hit behavior or is explicitly deferred and disabled online for the spike.
- One ring-out/respawn either replicates or is explicitly deferred after movement/bonk/pickup/snap are working.
- Rematch from final/winner state back to Party Hall does not crash or leave duplicate actors.

Recommended deferrals for the first spike:

- Staff Clash can be disabled online until Quick Bonk replication is stable.
- Loose segment chaos effects can be disabled online until basic snapping is stable.
- Arcane Pinball can be deferred if projectile replication delays the core spike.
- Mega Staff Brew can be deferred until normal staff count replication is correct.
- Bots can remain local-only until server-owned bot slots are deliberately designed.

Success criteria:

- The local one-human-versus-bot workflow still works.
- The local couch multiplayer workflow still works.
- A listen-server host and one remote client can complete a tiny network smoke path without crashes.
- No client can directly grant itself score, Favor, staff segments, or winner state.

## 5. Steam Lobby Phase Plan

Steam lobby work should be a later phase, after the minimal listen-server gameplay spike is stable.

Later Steam phase tasks:

- Configure Steam Online Subsystem and required project settings.
- Decide listen-server hosting model and whether dedicated servers are out of scope.
- Create player-hosted lobby.
- Invite Steam friends.
- List and join public/open lobbies.
- Support lobby privacy: friends-only, invite-only, public.
- Decide host migration policy. Recommendation for this scale: no host migration at first; if host leaves, return remaining players to lobby/menu with a clear message.
- Handle disconnects during Party Hall, Trials, Final, and rematch.
- Return players to lobby after match or on host cancellation.
- Add basic failure messages for join failure, full lobby, host left, version mismatch, and network timeout.
- Preserve local/offline mode as a separate entry path.

Do not start this phase before the gameplay replication spike proves that movement, bonk, staff count, pickups, and reset boundaries can work online.

## 6. Local Mode Preservation

Online support must not destroy the current proven local prototype workflow.

Preservation rules:

- Local PIE testing should remain possible.
- One-human-versus-bot should remain possible.
- Local couch multiplayer should remain possible.
- Shared camera can remain the default for local mode.
- Online clients can later use per-player camera behavior, but that should be a separate camera mode, not a replacement for local shared camera.
- Keyboard fallback controls should remain local-only unless explicitly redesigned.
- Local controller indexing should not leak into online identity.
- Player display order/color can still use a stable assigned slot, but that slot should come from replicated PlayerState, not local PlayerController order.
- Debug commands should stay available in local/dev mode and be host-authority gated or disabled in online sessions.
- Playtest bots should remain prototype helpers. If online bot slots are added, they should be server-owned and clearly separated from local bot filling.

Recommended mode separation:

- `EWizardPrototypeSessionMode` now exists for lightweight setup/diagnostic separation: LocalPrototype, LocalWithBots, OnlineListenServer, OnlineClient.
- Keep local code paths working while adding replicated paths incrementally.
- Avoid deleting local helper functions until online equivalents are proven.

## 7. Recommended Implementation Order

Start with infrastructure that has low gameplay risk. Do not begin with Steam lobbies or chaotic physics.

1. Identify replicated actors and ownership
   - Decide which classes replicate: wizard character, mug pickup, Staffs at Dawn powerup pickup, Arcane Pinball projectile, loose snapped segment actor if kept, GameState, PlayerState.
   - Decide what remains server-only: GameMode, authoritative scoring, match transitions, spawn selection, cleanup decisions.

2. Add `AWizardStaffGameState` and `AWizardStaffPlayerState` - completed as initial scaffold
   - Mirror party state, Trial state, active Trial type, timers, preset, winner/candidate state, player slots, scores, Favor, round wins, ready state, and important messages.
   - Leave GameMode logic in place, but start writing state to replicated mirrors.

3. Separate local-player index helper from network player identity
   - Keep a display slot for colors/HUD.
   - Store authoritative player slot on PlayerState.
   - Make arrays index by replicated player slot only after slot assignment is server-controlled.

4. Make wizard movement/input network-safe
   - Rely on Character movement replication where possible.
   - Add server RPCs for non-movement actions: Quick Bonk, Use Reward, Staff Clash mash, Ready Bell interaction if needed, debug/host commands where allowed.
   - Gate local-only input helpers and keyboard fallback.

5. Replicate staff segment count/readable visuals
   - Server owns `SegmentCount`.
   - Replicate segment count to wizard or component.
   - Client `OnRep` rebuilds visual segments and staff collision/readability as needed.
   - Keep the first version simple: count replication before per-segment physics replication.

6. Replicate Mana Slosh and Staff Stress
   - Server owns gameplay values.
   - Replicate to clients for HUD, wobble, stress meter, and snap warning.
   - Avoid client authority over stress or slosh changes.

7. Replicate mug pickup collection
   - Server validates overlap/collection.
   - Server calls the mug/staff growth path.
   - Replicate pickup active state and respawn.

8. Replicate scoring/Favor
   - Move display state into PlayerState/GameState mirrors.
   - Server awards score/Favor.
   - HUD reads replicated state instead of GameMode.

9. Replicate Trial transitions
   - Server owns Party Hall -> Countdown -> Trial -> Results -> Party Hall/Final.
   - Clients observe replicated state and display transitions.
   - Verify no false start and no stale state.

10. Replicate ring-out/respawn
   - Server owns bounds checks, respawn timers, staff reset, scoring credit, and transforms.
   - Replicate feedback state to clients.

11. Replicate Final Candidate state
   - Server owns Candidate, vulnerable/safe state, steal player, steal progress, final timer, and final winner.
   - Clients display replicated HUD/world markers.

12. Handle risky physics/comedy systems one by one
   - Quick Bonk hit confirmation first.
   - Staff Clash second.
   - Arcane Pinball projectile third, unless deferred.
   - Mega Staff Brew fourth.
   - Loose snapped segment physics and chaos effects last.

13. Run local regression after every online step
   - Local one-human-versus-bot.
   - Local couch multiplayer.
   - Listen-server host plus one client.
   - Rematch cleanup.

## 8. Do Not Do Yet

Do not implement these in the first online spike:

- Steam lobbies.
- Steam friend invites.
- Matchmaking.
- Public lobby browser.
- Ranked or competitive systems.
- Player accounts.
- Cosmetics.
- Progression.
- Shops.
- Inventory.
- Full UI or UMG conversion.
- Networked final art/VFX pass.
- Third Trial.
- All 12 Trials.
- New spells.
- New Staffs at Dawn powerups.
- Hammer Time.
- Full physics-chain staff.
- Dedicated server architecture unless a later release decision requires it.
- Host migration.
- Large refactors not required for the spike.
- Full loose segment network physics fidelity.

## 9. Implemented Scaffold And Remaining Local Reads

The first scaffolding pass added:

- `Source/WizardStaff/Public/WizardStaffGameState.h`
- `Source/WizardStaff/Private/WizardStaffGameState.cpp`
- `Source/WizardStaff/Public/WizardStaffPlayerState.h`
- `Source/WizardStaff/Private/WizardStaffPlayerState.cpp`

GameState currently mirrors:

- party match state
- active Trial state
- active Trial type
- active tuning preset
- Mug Run, Staffs at Dawn, countdown, results, intermission, and Final timer values
- completed Trial count
- Final Candidate index
- Final winner index
- current result/winner message
- lightweight match session generation

PlayerState currently mirrors:

- stable display slot
- color index
- round wins
- Grand Wizard Favor
- current Trial score
- Staffs at Dawn score
- Party Hall ready state
- playtest bot slot flag
- a lightweight summary string

Remaining local-only reads and assumptions:

- GameMode is still the gameplay authority and still owns the actual state machine, scoring, Favor, ring-outs, Final stealing, cleanup, bots, and all Trial logic.
- HUD FullDebug, detailed scoring panels, match summary telemetry, Final steal detail, and several feedback strings still read from GameMode. That is acceptable for the local locked slice, but remote clients will need additional GameState/PlayerState mirrors or replicated event/feed state.
- Carried reward, Mega Staff active state, Staff Clash state, loose segment chaos, exact staff collision contacts, pickup collection, ring-outs, Final stealing, and most detailed gameplay events are still local/server-only systems. Staff segment count, Mana Slosh, and Staff Stress now have replicated readability mirrors, but those mirrors are not full online gameplay implementations.
- Local player slot assignment still relies heavily on PlayerController order and existing arrays. A server-written PlayerState display slot now exists for connected controllers, but broad local-index cleanup is still future work.
- The shared camera, keyboard fallback, local player creation, and playtest bot fill behavior remain local-prototype workflows and are intentionally gated to standalone local sessions for the listen-server scaffold.
- The replicated mirrors are written every tick for scaffolding simplicity. Later online work should move timers to synchronized timestamps or only replicate changed state where practical.

## 10. Listen-Server Identity And Pawn-Control Spike Status

Implemented in the second online spike:

- `AWizardStaffWizardCharacter` now enables actor replication and movement replication for a basic listen-server smoke test.
- Wizard movement still uses the existing `ACharacter` movement path. Movement prediction, latency compensation, and server-side action validation have not been solved.
- Client turning now has a small owner RPC seam, `ServerSetFacingYaw`, so a joining client can rotate its own wizard during the pawn-control spike.
- `AWizardStaffGameMode` now keeps standalone-only helpers explicit:
  - auto-enabled PIE playtest bots only run in standalone local prototype sessions
  - `EnsureLocalPlayers` only creates extra local players in standalone local prototype sessions
  - keyboard fallback only runs in standalone local prototype sessions
  - shared camera spawn/assignment only runs in standalone local prototype sessions
- `AWizardStaffGameMode` writes a stable display slot/color mirror into `AWizardStaffPlayerState` on login/restart.
- `AWizardStaffGameState` and `AWizardStaffPlayerState` include low-noise client Output Log hooks to confirm that a non-host client receives mirrored state.
- Local one-human-versus-bot and local couch workflows are preserved by keeping their setup paths local-only instead of deleting them.

What now replicates for the spike:

- Wizard pawn actor existence and basic Character movement.
- Basic wizard facing yaw through an owner RPC.
- GameState mirror fields from the previous scaffold: party state, Trial state/type, timers, preset, Final Candidate/winner, result message, and match generation.
- PlayerState mirror fields from the previous scaffold: display slot, color index, round wins, Favor, current score mirrors, Party Hall ready flag, bot flag, and summary text.

What is still local-only or intentionally deferred:

- Quick Bonk server hit confirmation was deferred in this spike and received basic scaffolding later in Section 17.
- Staff Clash start, mash, immunity, and resolve were deferred in this spike and received scaffolding later in Section 19.
- Mana Slosh and Staff Stress gameplay authority/prediction beyond readable mirrors.
- Staff snapping and loose snapped segment physics/chaos replication.
- Arcane Pinball spawning, bounces, hits, and cleanup.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Mug pickup collection replication was deferred in this spike and completed later in Section 13.
- Staffs at Dawn powerup pickup replication.
- Ring-out attribution and respawn replication were deferred in this spike and received scaffolding later in Section 18.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

Two-client listen-server smoke test path:

1. In the Unreal Editor Play dropdown, open Advanced Settings.
2. Set Number of Players to `2`.
3. Set Net Mode to `Play As Listen Server`.
4. Enable separate windows if possible, so host and client input/view are easier to inspect.
5. Start PIE.
6. Expected result for this spike: the listen-server host and the joining client each receive a wizard pawn, each controls only their own pawn, movement and turning replicate acceptably for a smoke test, and Output Log shows client mirror messages from `WizardStaffGameState` and `WizardStaffPlayerState`.
7. Known limitation: the full two-Trial gameplay loop is still local-authoritative. Do not use this test to judge online bonks, Staff Clash, pickups, projectiles, powerups, ring-outs, Final stealing, or rematch correctness yet.

Direct command-line style smoke path, if needed later:

- Launch the host map as a listen server with `?listen`.
- Launch a second client and connect with `open 127.0.0.1`.
- This is still a direct-connect development test only. Steam sessions and lobby UX are a later phase.

## 11. Replicated Staff Segment Count And Readable Visuals Spike Status

Implemented in the third online spike:

- `AWizardStaffWizardCharacter` now owns a replicated staff segment count mirror: `ReplicatedStaffSegmentCount`.
- The real staff segment count remains server-authoritative in `UWizardStaffComponent`.
- `UWizardStaffComponent` notifies the owning wizard whenever the server changes the segment count through normal paths:
  - add staff segment
  - remove top segment
  - clear segments
  - rebuild segment count
  - reset staff state
  - snap top segment
- `ReplicatedStaffSegmentCount` uses `ReplicatedUsing` and `OnRep_ReplicatedStaffSegmentCount`.
- Clients rebuild placeholder staff visuals from the replicated count with `UWizardStaffComponent::RebuildStaffSegmentsForCount`.
- The rebuild path uses the existing socket/fallback segment chain, so future real sockets still fit the same model.
- Client visual rebuilds do not record telemetry, add Mana Slosh, add Staff Stress, score points, collect pickups, spawn loose segments, or grant rewards.
- Remote clients keep staff collision disabled for these rebuilt visuals. The server still owns real staff collision and stress behavior.
- Existing local standalone debug segment controls still work because standalone local play has authority.
- Non-authority client-side staff grant/debug/action paths are guarded until proper action RPCs exist.

What is authoritative versus visual-only:

- Authoritative: the server's `UWizardStaffComponent::SegmentCount`.
- Replicated mirror: `AWizardStaffWizardCharacter::ReplicatedStaffSegmentCount`.
- Visual-only: client-side runtime segment components rebuilt from the replicated count.
- Not replicated: individual segment components, loose snapped physics actors, snapped segment chaos effects, exact staff collision contacts, and full Staff Stress/Mana Slosh gameplay prediction.

Listen-server staff-count smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. In the listen-server/host context, use `DebugServerAddStaffSegmentToPlayer 0`.
5. Confirm both host and client see P1's staff grow.
6. Use `DebugServerAddStaffSegmentToPlayer 1`.
7. Confirm both host and client see P2's staff grow.
8. Use `DebugServerRemoveStaffSegmentFromPlayer 0` or `DebugServerClearStaffSegmentsForPlayer 0`.
9. Confirm both host and client see P1's staff shrink/rebuild without duplicate or stale segments.
10. Use `DebugServerSnapTopStaffSegmentForPlayer 1` if P2 has segments.
11. Confirm both host and client see P2's visible staff count shrink. The loose snapped physics segment is intentionally not replicated yet.

Still local-only or intentionally deferred after this spike:

- Mug pickup collection replication was deferred in this spike and completed later in Section 13.
- Mana Slosh and Staff Stress gameplay authority/prediction beyond readable mirrors.
- Quick Bonk server hit confirmation was deferred in this spike and received basic scaffolding later in Section 17.
- Staff Clash start, mash, immunity, and resolve were deferred in this spike and received scaffolding later in Section 19.
- Loose snapped segment physics and chaos effects replication.
- Arcane Pinball spawning, bounces, hits, and cleanup.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Staffs at Dawn powerup pickup replication.
- Ring-out attribution and respawn replication were deferred in this spike and received scaffolding later in Section 18.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 12. Replicated Mana Slosh And Staff Stress Readability Spike Status

Implemented in the fourth online spike:

- `AWizardStaffWizardCharacter` now owns replicated readability mirrors for:
  - `ReplicatedManaSlosh`
  - `ReplicatedMaxManaSlosh`
  - `ReplicatedStaffStress`
  - `ReplicatedMaxStaffStress`
- The real Mana Slosh value remains server-authoritative in `AWizardStaffWizardCharacter::ManaSlosh`.
- The real Staff Stress value remains server-authoritative in `UWizardStaffComponent::StaffStress`.
- Slosh and Stress mirrors use RepNotify/clamped client values for HUD/readability only.
- Replicated Slosh updates are thresholded at roughly `0.5` Slosh so normal decay does not spam tiny per-frame replication.
- Replicated Staff Stress updates are thresholded at roughly `0.5` Stress for the same reason.
- Forced sync is used for resets, preset application, snapping, debug clears, and other state boundaries where exact readability matters.
- Canvas HUD Slosh/Stress meters now use readable getters:
  - authority/local standalone reads the real values
  - non-authority clients read replicated mirrors
- Remote client Slosh visual wobble reads the replicated Slosh mirror for readability.
- Clients still cannot authoritatively grant themselves Mana Slosh or Staff Stress.

What is authoritative versus display-only:

- Authoritative Slosh: server-side `AWizardStaffWizardCharacter::ManaSlosh`.
- Display mirror: `AWizardStaffWizardCharacter::ReplicatedManaSlosh` and `ReplicatedMaxManaSlosh`.
- Authoritative Stress: server-side `UWizardStaffComponent::StaffStress`.
- Display mirror: `AWizardStaffWizardCharacter::ReplicatedStaffStress` and `ReplicatedMaxStaffStress`.
- Display-only clients: HUD meters and Slosh visual wobble read mirrors.
- Not implemented: client-authoritative Slosh/Stress changes, movement prediction based on replicated Slosh, server RPC action validation, full stress/snap effect replication, and exact staff collision contact replication.

Listen-server Slosh/Stress smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. In the listen-server/host context, use `DebugServerAddManaSloshToPlayer 0 25`.
5. Confirm both host and client can read P1's updated Slosh meter.
6. Use `DebugServerAddManaSloshToPlayer 1 25`.
7. Confirm both host and client can read P2's updated Slosh meter.
8. Use `DebugServerAddStaffStressToPlayer 0 25`.
9. Confirm both host and client can read P1's updated Staff Stress meter.
10. Use `DebugServerAddStaffStressToPlayer 1 25`.
11. Confirm both host and client can read P2's updated Staff Stress meter.
12. Use `DebugServerClearManaSloshForPlayer 0` and `DebugServerClearStaffStressForPlayer 1` to confirm readable values clear on both windows.
13. Confirm a normal remote client cannot directly grant itself authoritative Slosh or Stress through local debug inputs.

Still local-only or intentionally deferred after the Slosh/Stress readability spike, with later spike updates called out below:

- Quick Bonk server hit confirmation was deferred in this spike and received basic scaffolding later in Section 17.
- Staff Clash start, mash, immunity, and resolve were deferred in this spike and received scaffolding later in Section 19.
- Loose snapped segment physics and chaos effects replication.
- Arcane Pinball spawning, bounces, hits, and cleanup.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Mug Run brew reward slot/use replication.
- Staffs at Dawn powerup pickup replication.
- Ring-out attribution and respawn replication were deferred in this spike and received scaffolding later in Section 18.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 13. Server-Authoritative Mug Run Pickup Replication Spike Status

Implemented in the fifth online spike:

- `AWizardStaffManaMugPickup` now replicates as an actor for listen-server smoke testing.
- The pickup owns a replicated active-state mirror, `bIsPickupActive`, with `OnRep_PickupActive`.
- Clients rebuild the pickup's readable state from that mirror:
  - active mugs are visible and query-overlap enabled
  - collected mugs are hidden and collision disabled
  - respawned mugs become visible/collectible again when the server flips the active state
- The active-state application path is idempotent, so reset, collect, respawn, and RepNotify all converge on the same visible/collision state.
- `Collect`, `Respawn`, `SetPickupActive`, and the overlap collection path are authority-gated.
- The server owns the respawn timer. Clients do not independently respawn pickups.
- `AWizardStaffGameMode::ResetMugRunPickups` and `SetMugRunPickupsActive` still use the same pickup API, preserving the local reset/rematch flow.

Server validation and reward chain:

- Pickup overlap is ignored on non-authority clients.
- When the server sees a wizard overlap an active mug, it calls the existing `Collect` path.
- `Collect` calls the existing server-side wizard path, `DrinkMug`.
- `DrinkMug` adds one staff segment through the staff component and adds Mana Slosh for staff growth.
- The existing replicated mirrors then cover readability:
  - `ReplicatedStaffSegmentCount` updates and clients rebuild staff visuals
  - `ReplicatedManaSlosh` updates for HUD/readability
- Mug collection telemetry and Mug Run brew reward grant still run through `AWizardStaffGameMode` on the server.

What is authoritative versus visual-only:

- Authoritative: server-side pickup overlap, collection, active state, respawn timer, staff growth, Mana Slosh gain, telemetry, and brew reward grant.
- Replicated mirror: `AWizardStaffManaMugPickup::bIsPickupActive`.
- Visual-only: client pickup visibility/collision state, rebuilt staff segment visuals, and HUD Slosh/staff count readability.
- Not implemented in the pickup spike itself: client-authoritative collection, replicated carried brew reward display, networked Use Reward input, Arcane Pinball projectile replication, and any online scoring/Favor redesign. Carried reward display was implemented later in Section 14.

Brew reward behavior after this pass:

- Mug Run brew reward grants remain server-owned through the existing `TryGrantMugRunBrewReward` path.
- The carried reward slot was not yet a dedicated replicated online mirror in the pickup spike. It was implemented later in Section 14.
- Arcane Pinball projectile spawning, bounces, hits, self-hits, cleanup, and Use Reward input replication are still deferred.
- For online smoke testing, use mug collection to prove server-owned collection, staff growth, Slosh, and pickup hide/respawn visibility. Do not judge networked brew reward play yet.

Listen-server mug pickup smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. Enter Mug Run through the normal prototype flow, or force it with existing safe local/dev flow if needed.
5. Have the host wizard collect a mug.
6. Confirm the host and client both see the host wizard's staff grow.
7. Confirm the host and client both see the host wizard's Slosh meter update.
8. Confirm the collected mug hides/deactivates on both host and client.
9. Wait for the server respawn delay and confirm the mug reappears on both host and client.
10. Have the client-owned wizard collect a mug.
11. Confirm the server validates the collection and both windows see the client-owned wizard's staff grow and Slosh update.
12. Confirm a normal remote client cannot directly grant itself staff segments, Slosh, score, Favor, or brew rewards without server validation.

Still local-only or intentionally deferred after this spike:

- Replicated carried brew reward display was deferred in the pickup spike and completed later in Section 14. Network-safe Use Reward input is still deferred.
- Arcane Pinball projectile spawning, bounces, hits, self-hit, and cleanup.
- Quick Bonk server hit confirmation was deferred in this spike and received basic scaffolding later in Section 17.
- Staff Clash start, mash, immunity, and resolve were deferred in this spike and received scaffolding later in Section 19.
- Loose snapped segment physics and chaos effects replication.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Staffs at Dawn powerup pickup replication.
- Ring-out attribution and respawn replication were deferred in this spike and received scaffolding later in Section 18.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 14. Replicated Carried Brew Reward Display Spike Status

Implemented in the sixth online spike:

- `AWizardStaffWizardCharacter` now owns a replicated carried brew reward mirror, `ReplicatedCarriedBrewReward`.
- The real carried reward remains server-authoritative in `AWizardStaffWizardCharacter::CarriedBrewReward`.
- `TryGrantBrewReward` is authority-gated, so clients cannot grant or replace their own reward.
- `ClearCarriedBrewReward` is authority-gated, so state boundaries clear the real reward and mirror from the server.
- `SyncReplicatedCarriedBrewRewardFromAuthority` writes the current authoritative reward into the replicated mirror and forces a net update.
- `OnRep_ReplicatedCarriedBrewReward` refreshes the spellbook visual on clients.
- `GetCarriedBrewReward`, `HasCarriedBrewReward`, and `GetCarriedBrewRewardName` now read:
  - authoritative `CarriedBrewReward` on the server/standalone authority
  - `ReplicatedCarriedBrewReward` on non-authority clients
- The existing Canvas HUD reward text uses those getters, so remote clients can read the held reward without a HUD rewrite.
- The existing placeholder spellbook glow uses those getters, so remote clients see the fuchsia Arcane Pinball glow when the replicated mirror says a wizard is holding Arcane Pinball.
- The spellbook update remains idempotent and reuses existing mesh/material instances. It does not add duplicate glow components.

Reward grant and clear flow:

- Mug Run pickup collection remains server-owned.
- When `AWizardStaffGameMode::TryGrantMugRunBrewReward` succeeds, it calls `AWizardStaffWizardCharacter::TryGrantBrewReward` on the server.
- The server updates `CarriedBrewReward`, syncs `ReplicatedCarriedBrewReward`, updates the local/server spellbook visual, records telemetry, and pushes the existing local gameplay message.
- Clients receive `ReplicatedCarriedBrewReward` and update the spellbook/HUD readability.
- When rewards are cleared at Mug Run end, rematch/reset, or any existing server state boundary, the server clears `CarriedBrewReward`, syncs `ReplicatedCarriedBrewReward` to `None`, and clients remove the readable reward glow/text.

Use Reward behavior after this pass:

- Networked `UseReward` was deferred in this carried-reward display spike and gained a request seam later in Section 15.
- No Use Reward RPC exists yet in this section.
- Arcane Pinball projectile spawning, bounces, hits, self-hits, cleanup, and telemetry replication are still deferred.
- In networked play, `UseReward` is safely ignored until Use Reward RPC and projectile replication exist.
- Standalone local play keeps the existing local Arcane Pinball casting behavior.

What is authoritative versus visual-only:

- Authoritative: server-side `CarriedBrewReward`, reward grant rules, replacement rule, telemetry, and reward clear boundaries.
- Replicated mirror: `AWizardStaffWizardCharacter::ReplicatedCarriedBrewReward`.
- Visual/readability-only: non-authority client spellbook glow and Canvas HUD reward text.
- Not implemented: client-authoritative reward grants, networked reward casting, Arcane Pinball projectile replication, replicated reward gameplay messages, or online reward telemetry correctness.

Listen-server carried reward smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. Enter Mug Run through the normal prototype flow, or force it with existing safe local/dev flow if needed.
5. Have the host wizard collect mugs until a brew reward is granted.
6. Confirm the server owns the reward grant.
7. Confirm both host and client can see/read that the host wizard is holding Arcane Pinball.
8. Have the client-owned wizard collect mugs until a brew reward is granted.
9. Confirm the server owns the reward grant.
10. Confirm both host and client can see/read that the client-owned wizard is holding Arcane Pinball.
11. Confirm reward display clears at Mug Run end, rematch, reset, or another existing reward clear boundary.
12. Confirm a normal remote client cannot directly grant, replace, or cast its reward without server authority.

Still local-only or intentionally deferred after the carried-reward display spike, with later spike updates called out below:

- Use Reward RPC was deferred in this spike and added later as request scaffolding in Section 15. Full network-safe reward casting is still deferred.
- Arcane Pinball projectile spawning, bounces, hits, self-hit, cleanup, and telemetry replication.
- Quick Bonk server hit confirmation was deferred in this spike and received basic scaffolding later in Section 17.
- Staff Clash start, mash, immunity, and resolve were deferred in this spike and received scaffolding later in Section 19.
- Loose snapped segment physics and chaos effects replication.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Staffs at Dawn powerup pickup replication.
- Ring-out attribution and respawn replication were deferred in this spike and received scaffolding later in Section 18.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 15. Network-Safe Use Reward Request Seam Status

Implemented in the seventh online spike:

- `AWizardStaffWizardCharacter` now has an ownership-aware server RPC seam, `ServerUseReward`.
- `UseReward` keeps the standalone local path unchanged, so local Arcane Pinball casting still uses the existing direct gameplay path in standalone play.
- In networked play, `UseReward` routes through the server seam:
  - non-authority owning clients call `ServerUseReward`
  - listen-server authority calls the same server handler directly
  - non-owned network proxies are ignored
- `ServerUseReward_Implementation` calls `HandleNetworkedUseRewardRequestOnServer`.
- The server-side handler validates:
  - the request is running on authority
  - the wizard has a controller and that controller possesses this wizard
  - prototype input is not locked
  - the wizard is not in Staff Clash
  - the authoritative `CarriedBrewReward` is not `None`
- Unreal actor ownership supplies the RPC ownership boundary: a normal client can only call the Server RPC on an actor it owns. The server-side controller/pawn check is an extra sanity guard, not the only protection.

Server stub behavior:

- Valid online Arcane Pinball Use Reward requests are accepted by the server stub in this spike. A later spike adds a replicated readability shell in Section 16.
- In this spike, the stub consumes/clears the carried reward with `ClearCarriedBrewReward`.
- `ClearCarriedBrewReward` syncs `ReplicatedCarriedBrewReward` to `None`, so both host and clients clear HUD/spellbook readability through the existing mirror.
- The stub logs that projectile gameplay is deferred.
- The stub does not spawn Arcane Pinball in this section. Section 16 adds a server-owned readability shell.
- The stub does not simulate bounces, hits, self-hits, cleanup, stress, Slosh, or telemetry for online Arcane Pinball.

What is authoritative versus visual-only:

- Authoritative: server-side Use Reward request validation, authoritative carried reward value, and reward clear.
- Replicated mirror: `AWizardStaffWizardCharacter::ReplicatedCarriedBrewReward`.
- Visual/readability-only: client HUD reward text and spellbook glow clearing after the mirror reaches `None`.
- Not implemented: client-authoritative reward use, projectile authority, projectile prediction, bounce/hit confirmation, online Arcane Pinball telemetry correctness, or gameplay effects from the online stub.

Listen-server Use Reward smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. Enter Mug Run through the normal prototype flow, or force it with existing safe local/dev flow if needed.
5. Have the host wizard collect mugs until Arcane Pinball is granted.
6. Press Use Reward on the host wizard.
7. Confirm the request is handled by the server seam and the reward clears on host/client readability.
8. Have the client-owned wizard collect mugs until Arcane Pinball is granted.
9. Press Use Reward on the client-owned wizard.
10. Confirm the owning client request reaches the server, passes validation, and clears the replicated reward mirror.
11. Confirm a normal remote client cannot use another wizard's reward.
12. Confirm no real replicated Arcane Pinball projectile gameplay is expected yet.

Still local-only or intentionally deferred after this spike:

- Full Arcane Pinball projectile spawning, bounces, hits, self-hit, cleanup, and telemetry replication.
- Quick Bonk server hit confirmation was deferred in this spike and received basic scaffolding later in Section 17.
- Staff Clash start, mash, immunity, and resolve were deferred in this spike and received scaffolding later in Section 19.
- Loose snapped segment physics and chaos effects replication.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Staffs at Dawn powerup pickup replication.
- Ring-out attribution and respawn replication were deferred in this spike and received scaffolding later in Section 18.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 16. Server-Owned Arcane Pinball Readability Shell Status

Implemented in the eighth online spike:

- The existing `AWizardStaffArcanePinballProjectile` class was reused instead of adding a second projectile actor.
- The projectile now supports a replicated readability-only mode through `bReadabilityOnlyShell`.
- The actor now replicates and uses replicated movement for listen-server readability tests.
- `InitializeArcanePinball` remains the full local gameplay initializer.
- `InitializeArcanePinballReadabilityShell` initializes the same actor as a gameplay-inert network shell.
- The shell uses the existing magenta projectile mesh and trail visual style.
- The shell locks to launch height and travels forward using the existing projectile movement component.
- The shell disables collision and bounce gameplay while in readability-only mode.
- Projectile hit/bounce callbacks now guard on authority and ignore readability-only shells, so network clients cannot run local-only hit effects by accident.
- Existing `CleanupArcanePinballProjectiles` still cleans shell actors at Mug Run end, Staffs at Dawn start, Final setup, rematch/reset, and other existing Arcane Pinball cleanup boundaries.

Validated Use Reward to shell flow:

- Networked `UseReward` still routes through the validated `ServerUseReward` seam.
- When the server accepts an authoritative Arcane Pinball reward request, it calls `SpawnArcanePinballReadabilityShell`.
- The server spawns the projectile shell, initializes it with the caster, tuning, launch direction, and readability-only mode, then forces the actor to replicate.
- Only after the shell spawns successfully does the server call `ClearCarriedBrewReward`.
- `ClearCarriedBrewReward` syncs `ReplicatedCarriedBrewReward` to `None`, so HUD/spellbook readability clears on host and clients.
- If shell spawn fails, the reward is not consumed and the server logs a low-noise warning.

What replicates:

- The projectile actor spawn.
- The actor transform/movement through replicated movement.
- The readability-only shell flag.
- The existing projectile mesh and trail visibility as part of the replicated actor.
- Lifetime/destruction through normal server-owned actor lifespan/destruction.

What remains deferred:

- Full Arcane Pinball bounce authority.
- Arcane Pinball hit confirmation.
- Self-hit behavior.
- Knockback, Staff Stress, Mana Slosh, scoring, or telemetry effects from online projectile hits.
- Client-side projectile authority, prediction, or latency compensation.

Standalone preservation:

- Standalone local play still uses `FireArcanePinball` and `InitializeArcanePinball`.
- The local full Arcane Pinball behavior remains intact: bounces, hits, self-hits, stress, Slosh, cleanup, and telemetry still run through the existing local authority path.
- Local one-human-versus-bot and local couch workflows continue through the standalone path.

Listen-server Arcane Pinball shell smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. Enter Mug Run through the normal prototype flow, or force it with existing safe local/dev flow if needed.
5. Have the host wizard collect mugs until Arcane Pinball is granted.
6. Press Use Reward on the host wizard.
7. Confirm the request is validated server-side.
8. Confirm the reward clears on host and client HUD/spellbook readability.
9. Confirm the server spawns a visible Arcane Pinball shell.
10. Confirm both host and client can see the shell spawn and move/read correctly.
11. Repeat with the client-owned wizard.
12. Confirm no full bounce/hit/knockback/self-hit gameplay is expected yet.
13. Confirm no stale projectile shell survives rematch/reset.

Still local-only or intentionally deferred after this spike, with later spike updates called out below:

- Full Arcane Pinball bounce/hit gameplay replication.
- Arcane Pinball self-hit, knockback, Staff Stress, Mana Slosh effects, scoring, and telemetry correctness online.
- Quick Bonk server hit confirmation was deferred in this spike and received basic scaffolding later in Section 17.
- Staff Clash start, mash, immunity, and resolve were deferred in this spike and received scaffolding later in Section 19.
- Loose snapped segment physics and chaos effects replication.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Staffs at Dawn powerup pickup replication.
- Ring-out attribution and respawn replication were deferred in this spike and received scaffolding later in Section 18.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 17. Server-Authoritative Quick Bonk Request And Basic Hit Scaffold Status

Implemented in the ninth online spike:

- `AWizardStaffWizardCharacter` now has an ownership-aware server RPC seam, `ServerRequestQuickBonk`.
- In networked play, `QuickBonk` routes through the server seam:
  - non-authority owning clients call `ServerRequestQuickBonk`
  - listen-server authority calls the same server handler directly
  - non-owned network proxies are ignored
- Standalone local play keeps the existing direct Quick Bonk path, including Staff Clash and the previous local feel.
- `ServerRequestQuickBonk_Implementation` calls `HandleQuickBonkRequestOnServer`.
- The server-side handler validates:
  - the request is running on authority
  - the wizard has a controller and that controller possesses this wizard
  - prototype input is not locked
  - Staff Clash is not active
  - `CanQuickBonk` passes existing cooldown/active-swing checks
- Unreal actor ownership supplies the RPC ownership boundary; the server-side controller/pawn check is an extra sanity guard.

Replicated bonk readability:

- `ReplicatedQuickBonkSequence` increments whenever the server starts an approved networked bonk.
- `ReplicatedQuickBonkVisualDuration` carries the approved server swing duration for client readability.
- `ReplicatedQuickBonkCancelSequence` increments when server reset/respawn code needs clients to cancel any stale bonk visual.
- `OnRep_ReplicatedQuickBonkSequence` starts only the client-side swing visual timer.
- Non-authority clients mark `bQuickBonkHitResolved` true, so they never run hit detection from replicated readability state.
- `OnRep_ReplicatedQuickBonkCancelSequence` clears client-side swing readability.

Server-owned impact timing and basic hit confirmation:

- The server starts the existing bonk state through `StartQuickBonkOnAuthority`.
- The server owns `QuickBonkImpactTimeRemaining` and resolves impact through the existing `UpdateBonkAttack` path.
- At impact time, the server reuses `PerformQuickBonkHitDetection`.
- The impact check uses the authoritative staff collision box overlap against pawns/world-dynamic objects.
- Server-confirmed hits use the existing `ApplyBonkToTarget` path for hit reaction/knockback.
- Existing server-side bonk telemetry and Staff Stress paths remain in place.
- Clients do not submit target lists or hit results.
- In networked play, Staff Clash is intentionally skipped in `ApplyBonkToTarget`; standalone local Staff Clash is preserved.

Cleanup and reset:

- `CancelQuickBonkStateForReset` clears local bonk timers/hit sets and syncs a cancel sequence when authority is networked.
- `ResetForNewMatch`, arena respawn, and Final candidate respawn now clear bonk state so clients do not keep stale swing poses through state boundaries.
- Normal swing visuals still self-clear through the existing visual timer.

What is authoritative versus visual-only:

- Authoritative: server-side request validation, bonk start, impact timing, hit overlap, hit reaction, knockback, Staff Stress, and telemetry.
- Replicated readability: bonk sequence, visual duration, and cancel sequence.
- Visual-only: client swing pose/readability driven by replicated sequence/duration.
- Not implemented in this spike: client hit prediction, lag compensation, client-reported hits, Staff Clash replication, ring-out/respawn scaffolding, or exact cross-client staff collision fidelity.

Listen-server Quick Bonk smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. Host presses Quick Bonk.
5. Confirm host and client both see the host wizard bonk.
6. Client presses Quick Bonk.
7. Confirm host and client both see the client wizard bonk.
8. Place wizards close together.
9. Host bonks the client-owned wizard.
10. Confirm the server owns the hit check and both windows see the hit/knockback if overlap succeeds.
11. Client bonks the host wizard.
12. Confirm the owning client request reaches the server and the server owns the hit check.
13. Confirm a normal remote client cannot bonk as another wizard or directly apply hit/knockback.
14. Confirm no Staff Clash is expected yet in networked play.
15. Confirm no stale bonk state remains after Trial transition/rematch/respawn.

Still local-only or intentionally deferred after this spike:

- Staff Clash start, mash, immunity, and resolve replication.
- Ring-out attribution and respawn replication were deferred in this spike and received scaffolding later in Section 18.
- Quick Bonk prediction/rewind/lag compensation.
- Exact cross-client staff collision fidelity.
- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Staffs at Dawn powerup pickup replication.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 18. Server-Authoritative Ring-Out And Respawn Scaffold Status

Implemented in the tenth online spike:

- `AWizardStaffGameMode` still owns out-of-arena detection through `UpdateOutOfArenaRespawns`.
- Because `GameMode` only exists on authority, clients cannot directly declare themselves or another player ringed out.
- The existing bounds logic remains unchanged:
  - horizontal bounds come from `GetCurrentPlayBoundsCenter` and `GetCurrentPlayBoundsHalfSize`
  - `OutOfArenaRespawnTuning.HorizontalOutOfBoundsPadding` remains the padding value
  - fall checks still use `GetCurrentOutOfArenaFallZThreshold`
  - Staffs at Dawn can still raise the fall threshold from the active Staffs at Dawn arena
- `PendingOutOfArenaRespawns` remains the server-owned pending respawn timer map.
- A new `ClearPendingOutOfArenaRespawns` helper clears pending server timers and mirrors that cleared state to any affected wizards.

Replicated ring-out/respawn readability:

- `AWizardStaffWizardCharacter` now mirrors a minimal readable respawn state:
  - `bReplicatedOutOfArenaRespawning`
  - `ReplicatedOutOfArenaRespawnRemainingTime`
- `SyncReplicatedOutOfArenaRespawnStateFromAuthority` is the only write path for this mirror.
- The mirror is low-noise; remaining time only forces updates when the value changes enough to matter.
- Non-authority clients treat this as display/readability only.
- `OnRep_ReplicatedOutOfArenaRespawnState` clears stale client bonk visuals when a pawn enters pending respawn.

Server-owned respawn flow:

- When the server first detects a wizard out of bounds, it starts one pending timer for that wizard and mirrors the pending respawn state.
- If the wizard returns to the play area before the timer expires and `bCancelRespawnIfPlayerReturns` is enabled, the server cancels the pending timer and clears the mirrored respawn state.
- When the server timer expires, the server records the out-of-arena respawn, chooses the spawn transform through the existing `GetSpawnTransformForController` path, teleports the wizard, restores walking movement, clears pending respawn readability, and forces a net update.
- `CancelMovementStateForRespawn` clears quick bonk, Staff Clash, broom boost, and slosh turn carry state at the actual respawn moment.
- Broom boost remains available during the pending delay so players can still attempt saves before the server commits to respawn.

Attribution and staff reset:

- Ring-out attribution uses the existing server-side recent-bonk maps:
  - `RecentBonkAttackerPlayerIndexes`
  - `RecentBonkTimes`
  - `RecentBonkWasStaffClash`
- These maps are written from server-confirmed bonk/Staff Clash telemetry paths, not client hit claims.
- Staffs at Dawn ring-out scoring/Favor/staff-growth attribution remains unchanged.
- If a Staffs at Dawn respawn clears the victim's staff, `UWizardStaffComponent::ClearStaffSegments` already notifies the replicated staff segment count mirror.
- Staff Stress mirrors clear through the existing staff component notification path.
- Mana Slosh behavior is not retuned by this spike.

Cleanup and stale-state safety:

- Trial transitions, Staffs at Dawn reset, Mug Run reset, Party Hall intermission, Final setup, disabled respawn mode, and rematch all clear pending respawn timers through `ClearPendingOutOfArenaRespawns`.
- Respawn completion clears the mirrored pending state and forces a character net update so host/client should see the corrected transform.
- Quick Bonk networking is unchanged except that respawn now clears stale bonk state through the existing reset helper.
- Staff Clash networking remains deferred.

What is authoritative versus visual-only:

- Authoritative: bounds detection, pending respawn timer, cancel-if-returned decision, spawn transform, teleport/respawn, Staffs at Dawn ring-out attribution through server recent-bonk maps, staff reset on ring-out, scoring/Favor through existing GameMode paths.
- Replicated readability: out-of-arena respawning flag and approximate remaining respawn time.
- Visual-only: client-side awareness that a wizard is pending respawn.
- Not implemented: client-side ring-out authority, client-declared attackers, prediction, lag compensation, Staff Clash replication, or exact client-side save validation beyond the server's existing return-to-arena check.

Listen-server ring-out/respawn smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. Move the host wizard out of bounds.
5. Confirm the server starts pending respawn and both windows see the host wizard respawn.
6. Move the client-owned wizard out of bounds.
7. Confirm the server starts pending respawn and both windows see the client-owned wizard respawn.
8. In Staffs at Dawn, bonk a player out of bounds.
9. Confirm attribution is only based on server-recorded recent bonks if scoring credit is awarded.
10. Confirm Staffs at Dawn staff reset after ring-out still updates visible staff segment count on both windows.
11. Confirm a remote client cannot directly force score, Favor, ring-out credit, or respawn.
12. Confirm no stale pending respawn state remains after Trial transition/rematch.

Still local-only or intentionally deferred after this spike:

- Staff Clash start, mash, immunity, and resolve replication were deferred in this spike and received scaffolding later in Section 19.
- Quick Bonk prediction/rewind/lag compensation.
- Exact cross-client staff collision fidelity.
- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Staffs at Dawn powerup pickup replication.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 19. Server-Owned Staff Clash Request And State Scaffold Status

Implemented in the eleventh online spike:

- Staff Clash can now start in networked play only from server-confirmed Quick Bonk impact flow.
- Clients still cannot directly start a clash, submit clash hit claims, force another wizard into clash, or declare a winner.
- `ApplyBonkToTarget` now permits `TryStartStaffClashWith` on authority, so standalone local play and listen-server authority both use the same core clash detection rules.
- Existing local Staff Clash tuning is preserved:
  - near-simultaneous bonk timing
  - facing/direction thresholds
  - participant validity checks
  - mash duration and winner knockback tuning
- Standalone local Staff Clash remains on the existing direct local path.

Mash request scaffold:

- No separate mash RPC was added in this pass.
- The existing owned `ServerRequestQuickBonk` seam now doubles as the network mash request path.
- When the server receives a valid owned Quick Bonk request from a wizard already in Staff Clash, it calls `RegisterStaffClashMash` instead of starting a new bonk.
- Server validation still requires the wizard to be possessed by the requesting controller.
- Clients cannot directly set mash counts or declare a clash winner.

Replicated clash readability:

- `AWizardStaffWizardCharacter` now mirrors minimal Staff Clash readable state:
  - `bReplicatedStaffClashActive`
  - `ReplicatedStaffClashOpponent`
  - `ReplicatedStaffClashSequence`
  - `ReplicatedStaffClashRemainingTime`
  - `ReplicatedStaffClashMashCount`
  - `ReplicatedStaffClashLockedLocation`
- `OnRep_ReplicatedStaffClashState` applies only local readability:
  - clash pose/readable lock
  - local mash count display/visual shake support
  - stale quick-bonk visual cleanup
  - broom boost visual cleanup
- Non-authority clients never resolve the clash or apply clash knockback from replicated state.

Server-owned clash resolve:

- The server owns clash countdown through the existing `UpdateStaffClash` path.
- The server owns winner/tie determination through the existing `ResolveStaffClash` path.
- The server owns winner knockback through the existing `ApplyStaffClashWinningBonkToTarget` path.
- Staffs at Dawn ring-out consequences from a clash-winning bonk still flow through the existing server-owned ring-out/respawn scaffold and recent-bonk attribution.
- Client prediction, rewind, and latency compensation remain deferred.

Cleanup and stale-state safety:

- `ClearStaffClashState` now syncs a cleared replicated clash state when called on authority.
- `CancelMovementStateForRespawn`, trial resets, rematch resets, and respawn cleanup still clear Staff Clash state through the existing reset paths.
- If one participant is cleared by respawn/state transition, the other participant exits clash on the server the next update when the opponent link is invalid.
- Normal Quick Bonk replicated visuals are cleared when clients enter a replicated Staff Clash state so stale swing poses do not overlap the clash readability.

What is authoritative versus visual-only:

- Authoritative: clash start, participant validation, mash count, countdown, winner/tie selection, winner knockback, Staff Stress, telemetry, and any resulting ring-out consequences.
- Replicated readability: active flag, opponent reference, sequence, remaining time, mash count, and locked location.
- Visual-only: client clash pose/readable lock and mash count readability.
- Not implemented: client-side clash start, client-declared mash counts, client-declared winner, prediction/rewind/lag compensation, or production clash UI.

Listen-server Staff Clash smoke test path:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm the host and joining client each possess their own wizard.
4. Place host and client wizards close together and facing each other.
5. Trigger near-simultaneous Quick Bonks.
6. Confirm the server starts Staff Clash only if the timing/direction checks pass.
7. Confirm both host and client see the clash readable state.
8. Mash Quick Bonk on both owning clients.
9. Confirm mash counts are accepted only for each owned participant.
10. Confirm a remote client cannot start, resolve, or mash for another wizard.
11. Confirm the server resolves the clash or ties it safely.
12. Confirm movement/input lock clears after resolve.
13. Confirm no stale clash state remains after respawn, Trial transition, or rematch.

Still local-only or intentionally deferred after this spike:

- Quick Bonk prediction/rewind/lag compensation.
- Exact cross-client staff collision fidelity.
- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Staffs at Dawn powerup pickup replication.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 20. Online Combat Cleanup And Stale-State Hardening Status

Implemented in the twelfth online spike:

- This pass did not add a new gameplay system.
- The pass focused on the interaction between:
  - server-owned Quick Bonk
  - server-owned Staff Clash
  - server-owned ring-out/respawn
  - trial/rematch reset boundaries

Stale-state bug found and fixed:

- A wizard could enter pending out-of-arena respawn while still linked to an active Staff Clash.
- Before this cleanup, the clash would usually clear later through respawn/update cleanup, but the opponent could briefly remain in a stale linked clash state.
- `CancelStaffClashStateForReset` now clears the local wizard and, if still linked, the opponent wizard too.
- `UpdateOutOfArenaRespawns` now cancels paired Staff Clash state immediately when the server starts a pending ring-out respawn timer.
- The Grand Wizard Final candidate placement path now uses `CancelMovementStateForRespawn` instead of only clearing Quick Bonk, so clash/broom/turn-carry state clears consistently there too.

Checks performed where no code change was needed:

- Quick Bonk remains server-requested through `ServerRequestQuickBonk`; clients still do not submit hit targets.
- Quick Bonk visuals already clear through the replicated cancel sequence, Staff Clash OnRep, ring-out pending OnRep, respawn cleanup, and match reset.
- Staff Clash mashing already routes through the owning Quick Bonk RPC seam, so no separate mash RPC was needed.
- Staff Clash resolve remains server-owned; non-authority clients only display replicated clash readability.
- Ring-out timers remain server-owned in `PendingOutOfArenaRespawns`.
- Ring-out attribution still uses server-side recent-bonk maps; clients still cannot force score, Favor, staff growth, or winner state.
- Staffs at Dawn ring-out staff reset still updates the replicated staff segment count through the staff component notification path.

Authority safety after this pass:

- Authoritative: Quick Bonk request validation, hit overlap, Staff Clash start, mash counts, clash resolve, winner knockback, ring-out detection, respawn timers, respawn transforms, Staffs at Dawn ring-out attribution, scoring/Favor through GameMode paths.
- Replicated readability only: Quick Bonk swing/cancel sequence, Staff Clash active/opponent/time/mash/locked-location state, out-of-arena respawn pending/remaining-time state.
- Clients still cannot directly start Quick Bonk for another wizard, submit hit targets, start Staff Clash, set mash counts, resolve Staff Clash, declare ring-out, force respawn, award score/Favor, or assign winner state.

Updated listen-server combat stress test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm host/client movement and facing still replicate.
4. Confirm host/client Quick Bonk readability works.
5. Confirm server-confirmed hit/knockback works when overlap succeeds.
6. Trigger near-simultaneous bonks and confirm Staff Clash starts only from server-confirmed bonk state.
7. Mash Quick Bonk on both clients and confirm only owned clash participants increment mash.
8. Confirm Staff Clash resolves or ties safely.
9. Confirm movement/input lock clears after clash.
10. Knock or clash-launch a wizard out of bounds.
11. Confirm server-owned ring-out/respawn handles the consequence.
12. Confirm Staffs at Dawn staff reset after ring-out still updates replicated staff count.
13. Transition Trials and rematch.
14. Confirm no stale Quick Bonk, Staff Clash, or ring-out state remains.
15. Confirm a remote client cannot force combat authority.

Still local-only or intentionally deferred after this cleanup:

- Quick Bonk prediction/rewind/lag compensation.
- Exact cross-client staff collision fidelity.
- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Mega Staff Brew pickup, temporary segment tracking, and expiration.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 21. Staffs At Dawn Powerup Pickup Replication Status

Implemented in the thirteenth online spike:

- `AWizardStaffStaffsAtDawnPowerupPickup` is now a replicated actor with replicated movement for listen-server smoke testing.
- The pickup replicates its active/hidden state through `bIsPickupActive`.
- The pickup replicates its readable powerup type through `PowerupType`, so clients can keep placeholder color/readability in sync.
- Client RepNotify handlers only update visual/collision readability. They do not grant powerups or run gameplay effects.

Server collection validation:

- Pickup overlap is ignored on non-authority instances.
- `AWizardStaffGameMode::HandleStaffsAtDawnPowerupPickedUp` now exits unless it is running on authority, the Trial is active, the pickup is valid and active, and the wizard maps to a valid player index.
- Clients cannot directly collect Staffs at Dawn powerups, grant themselves Mega Staff Brew, force pickup respawn, force score/Favor, or assign winner state.

Active/hidden/respawn replication:

- On collection, the server marks the pickup inactive with `SetPickupActive(false)`.
- When respawn is enabled, the actor is kept alive but hidden/inactive while the GameMode-owned respawn timer counts down.
- When the timer expires, the server chooses the next spawn marker, moves the same pickup actor, sets the powerup type, and reactivates it.
- If respawn is disabled or the Trial is ending, the server can still hide and destroy the pickup through the existing reset path.
- Clients should see the pickup disappear after collection and reappear at the server-owned transform when reactivated.

Mega Staff Brew behavior at this pickup-only pass boundary:

- Standalone local play keeps the existing full Mega Staff Brew behavior.
- Listen-server/networked play validates the pickup and replicates hide/respawn readability, but the actual Mega Staff Brew gameplay effect is intentionally deferred.
- In networked play, the server shows a small "online effect deferred" feedback message instead of granting temporary segments, stress multipliers, knockback multipliers, temporary segment tracking, or expiration behavior.
- This keeps the pickup authority path proven without pretending the full Mega Staff effect has been safely replicated.

Updated listen-server Staffs powerup smoke test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm host/client movement and possession still work.
4. Enter Staffs at Dawn through the normal prototype flow or an existing safe dev flow.
5. Confirm Staffs at Dawn powerup pickup appears on both host and client.
6. Have the host wizard overlap the pickup.
7. Confirm the server validates collection and both windows see the pickup hide.
8. Confirm both windows see the pickup respawn/reactivate if respawn is enabled.
9. Have the client-owned wizard overlap the pickup.
10. Confirm the owning client does not grant itself the powerup directly.
11. Confirm the server validates collection and both windows see the same hide/respawn state.
12. Confirm no stale pickup remains after Trial transition or rematch.

Still local-only or intentionally deferred after this pass:

- Full Mega Staff Brew effect replication, including temporary segment tracking and expiration. This is addressed by the next online spike section below.
- Staffs at Dawn powerup gameplay effect authority beyond pickup collection/readability.
- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 22. Mega Staff Brew Effect Replication Scaffolding Status

Implemented in the fourteenth online spike:

- Mega Staff Brew is granted by the server after a validated Staffs at Dawn powerup pickup.
- The previous online "effect deferred" pickup branch was removed.
- The existing authority-only `ActivateMegaStaffBrew` path is reused, so the same temporary segment grant, Mana Slosh gain, stress multiplier, knockback multiplier, expiration, and telemetry path remains the source of truth.
- Standalone local behavior remains on the same path and should keep the same Mega Staff timing, segment count, stress, knockback, and cleanup feel.

Replicated Mega Staff state:

- `AWizardStaffWizardCharacter` now mirrors Mega Staff readability through:
  - `bReplicatedMegaStaffActive`
  - `ReplicatedMegaStaffRemainingTime`
  - `ReplicatedMegaStaffTemporarySegmentCount`
  - `ReplicatedMegaStaffSequence`
- `SyncReplicatedMegaStaffStateFromAuthority` is called by the server on grant, countdown updates, temporary segment snap/loss, and clear/expiration.
- Non-authority clients use `OnRep_ReplicatedMegaStaffState` only for HUD/readability and placeholder marker/hat visual feedback.
- Clients do not add temporary staff segments, remove staff segments, expire Mega Staff, or apply gameplay modifiers from the replicated state.

Temporary segment and staff-count behavior:

- The server still grants temporary Mega Staff segments through the existing staff component path.
- The already-existing replicated staff segment count mirror makes clients see the staff grow/shrink.
- `MegaStaffTemporarySegmentsRemaining` is replicated as readable state so clients can tell whether the active Mega Staff still has temporary segments remaining.
- If temporary segments snap before expiration, `NotifyMegaStaffSegmentSnapped` reduces the server-owned temporary segment count and syncs the mirror.
- Expiration removes only the remaining temporary Mega Staff segments. Already-snapped/lost temporary segments are not removed again, so earned permanent segments are protected.

Expiration and cleanup:

- The server owns Mega Staff countdown and expiration.
- Non-authority clients may locally count down the displayed remaining time, but they do not clear the authoritative effect themselves.
- `ClearMegaStaffBrew` is authority-only online and syncs inactive Mega Staff state after cleanup.
- Existing cleanup boundaries that clear Mega Staff, such as Staffs at Dawn end, ring-out staff reset behavior, Trial transitions, and rematch reset, now also update the replicated Mega Staff mirror.

Mega Staff modifiers online:

- Server-owned Quick Bonk and Staff Clash paths can use the existing Mega Staff knockback multiplier because those hit/knockback calculations already run on authority.
- Server-owned bonk stress can use the existing Mega Staff stress multiplier because Staff Stress is already server-owned and mirrored for readability.
- Deferred: exact client-side prediction, special Mega Staff VFX, loose snapped segment physics replication, and any non-authoritative gameplay modifier behavior.

Updated listen-server Mega Staff smoke test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Enter Staffs at Dawn through the normal prototype flow or an existing safe dev flow.
4. Confirm Staffs at Dawn powerup pickup appears on both windows.
5. Have the host wizard collect Mega Staff Brew.
6. Confirm the server validates collection and grants Mega Staff.
7. Confirm both host and client see the host wizard staff grow and Mega Staff readability activate.
8. Wait for expiration.
9. Confirm the server expires Mega Staff and removes only remaining temporary segments.
10. Confirm both windows see staff count/readability update after expiration.
11. Repeat with the client-owned wizard collecting Mega Staff Brew.
12. Confirm the owning client cannot grant, extend, expire, or directly modify Mega Staff.
13. Confirm no stale Mega Staff state remains after Trial transition or rematch.

Still local-only or intentionally deferred after this pass:

- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.
- Prediction, rewind, lag compensation, and exact cross-client physics fidelity.

## 23. Staffs Powerup And Mega Staff Cleanup Hardening Status

Completed as the fifteenth online spike:

- This was a focused code-level audit and documentation pass for Staffs at Dawn powerup pickup replication plus Mega Staff Brew effect replication scaffolding.
- No new gameplay system, powerup, UI, VFX, tuning change, or Steam/session feature was added.
- No new code fix was required by the static audit. The existing server-owned pickup and Mega Staff paths already covered the stale-state risks checked in this pass.

Checks performed where no code change was needed:

- Staffs at Dawn powerup pickups are replicated actors and keep active/hidden readability in `bIsPickupActive`.
- Non-authority pickup overlap is ignored, so clients cannot directly collect or force respawn.
- GameMode validates that Staffs at Dawn is active, the pickup is valid/active, and the wizard maps to a valid player before granting the effect.
- Hidden pickups keep collision disabled through `ApplyPickupActiveState`.
- Server-owned respawn timers reactivate or respawn pickups without client authority.
- Staffs at Dawn reset destroys tracked pickups and clears spawn/timer arrays, protecting Trial transition and rematch boundaries from duplicate actors.
- Mega Staff grant remains server-owned through `ActivateMegaStaffBrew`.
- Mega Staff replicated readability is display-only through `bReplicatedMegaStaffActive`, `ReplicatedMegaStaffRemainingTime`, `ReplicatedMegaStaffTemporarySegmentCount`, and `ReplicatedMegaStaffSequence`.
- Non-authority clients do not add/remove staff segments, apply combat modifiers, or expire Mega Staff from `OnRep_ReplicatedMegaStaffState`.
- Server expiration still removes only remaining temporary Mega Staff segments.
- `NotifyMegaStaffSegmentSnapped` reduces the server-owned temporary segment count and syncs the readable mirror.
- Staffs at Dawn ring-out staff reset clears Mega Staff state through the existing server path before clearing staff segments.
- Staffs at Dawn end clears active Mega Staff through the existing server path.
- Existing staff segment replication updates client staff visuals after grant, snap/loss, expiration, ring-out reset, and rematch reset.

Bugs found and fixed:

- None in this pass.

Authority after this pass:

- Authoritative: pickup collection validation, pickup active/hidden/respawn timing, Mega Staff grant, temporary segment tracking, Mega Staff countdown, expiration, staff segment changes, stress/knockback modifiers through server-owned combat paths, ring-out staff reset, scoring/Favor.
- Visual/readability only: pickup active/type display on clients, Mega Staff active flag, remaining-time display, temporary segment count display, marker/hat placeholder feedback, staff visual rebuilds from replicated segment count.

Updated listen-server Staffs/Mega Staff stress test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Enter Staffs at Dawn through the normal prototype flow or an existing safe dev flow.
4. Confirm the Staffs powerup appears on both windows.
5. Collect Mega Staff as host and confirm both windows see pickup hide, staff growth, and Mega Staff readability.
6. Quick Bonk while Mega Staff is active and confirm combat consequences remain server-owned.
7. Trigger Staff Clash while one wizard has Mega Staff if practical and confirm resolve/cleanup remains safe.
8. Knock or launch a wizard out of bounds while Mega Staff is active and confirm ring-out/respawn plus staff reset clears Mega Staff safely.
9. Let Mega Staff expire and confirm only remaining temporary segments are removed.
10. Repeat with the client-owned wizard collecting Mega Staff.
11. Repeat pickup collection/respawn several times.
12. Transition Trials and rematch.
13. Confirm no stale pickup, Mega Staff, temp segment, timer, Quick Bonk, Staff Clash, or ring-out state remains.
14. Confirm a remote client cannot force pickup or Mega Staff authority.

Still local-only or intentionally deferred after this pass:

- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Full Grand Wizard Final stealing replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.
- Prediction, rewind, lag compensation, and exact cross-client physics fidelity.

## 24. Grand Wizard Final Candidate Readable State Replication Status

Implemented in the sixteenth online spike:

- GameMode remains authoritative for entering the Grand Wizard Final, choosing/changing Candidate, timer countdown, winner selection, Favor, score, round wins, rematch/reset, and all steal rules.
- `AWizardStaffGameState` already mirrored `ReplicatedFinalCandidateIndex`, `ReplicatedFinalWinnerIndex`, `ReplicatedFinalRoundRemainingTime`, and `ReplicatedResultMessage`.
- This pass added a small readable Final safety mirror:
  - `bReplicatedFinalCandidateVulnerable`
  - `ReplicatedFinalReadableSequence`
- `SyncReplicatedObservableState` now writes the server-owned Candidate vulnerable/safe result into GameState alongside the existing Candidate, winner, result, and timer mirrors.
- The replicated vulnerable flag is forced false when the Final is not active, the Final is in Results, or a winner is already locked.

HUD/readability behavior:

- Canvas HUD Playtest and Minimal modes can now use GameState fallback data when GameMode is unavailable on a client.
- Remote clients can read:
  - current party state
  - Grand Wizard Final timer
  - current Candidate index
  - Candidate SAFE/VULNERABLE readability
  - final winner/result message
- Compact player rows already use GameState Candidate/winner mirrors when GameMode is unavailable, so client player rows can mark `CANDIDATE` and `WINNER`.
- The client-side SAFE/VULNERABLE fallback is readability-only. It does not drive circle occupancy, stealing, Candidate swaps, scoring, Favor, or winner selection.

Authoritative versus visual-only:

- Authoritative: GameMode Final state, Candidate assignment, Candidate vulnerable calculation, timer, winner, scoring/Favor, steal rules, Trial transitions, rematch/reset.
- Visual/readability only: GameState Final mirrors, Canvas HUD Candidate labels, SAFE/VULNERABLE text, Final timer display, winner/result display, final readable sequence.

Still local-only or intentionally deferred after this pass:

- Full Grand Wizard Final stealing replication.
- Client steal request RPCs.
- Steal progress replication.
- Client circle occupancy authority.
- Final-specific prediction/lag compensation.
- Final circle visual replication beyond existing local/server placeholder behavior.
- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Replicated gameplay event/message feed.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

Listen-server Final readable-state smoke test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm host/client movement and possession still work.
4. Progress to the Grand Wizard Final through the normal prototype flow, or use an existing safe dev flow.
5. Confirm the server enters the Final and chooses the initial Candidate.
6. Confirm both host and client can see/read the Candidate in player rows and Final text.
7. Confirm both host and client can see/read the Final timer.
8. Move the Candidate in/out of safety through existing server-owned local logic and confirm client SAFE/VULNERABLE readability updates.
9. Let the Final end and confirm both host/client see the final winner/result message.
10. Rematch/reset and confirm Candidate, timer, winner, and SAFE/VULNERABLE readability clear or reset.
11. Confirm a remote client cannot directly assign Candidate, winner, timer, score, Favor, or round wins.
12. Confirm full Final stealing replication is not expected yet.

## 25. Grand Wizard Final Stealing Request/Progress Scaffolding Status

Implemented in the seventeenth online spike:

- No client request RPC was added. The server can already infer Final steal intent from authoritative pawn positions, circle checks, and existing server-owned Final logic.
- GameMode remains authoritative for steal eligibility, progress accumulation, progress clearing, Candidate swaps, Final timer, winner selection, scoring, Favor, and rematch/reset.
- The existing local/server Final steal rules were reused:
  - challenger must be inside the ritual circle
  - Candidate must be outside the safe/control radius when that rule is enabled
  - leader start bonus still blocks stealing while active
  - existing hold duration still decides Candidate swap
- No Final tuning was changed.

Server validation and stale-state protection:

- `IsWizardEligibleForGrandWizardSteal` centralizes server-side steal eligibility checks.
- A wizard is not eligible to steal if it is invalid, the Final is not active, it is the current Candidate, it is pending out-of-arena respawn, it is already readable as respawning, or it is currently out of arena.
- `GetBestGrandWizardCircleChallenger` now uses this eligibility helper before considering a challenger for steal progress.
- `UpdateGrandWizardFinalRound` revalidates the current challenger before accumulating progress.
- Steal progress still clears through existing GameMode rules when no valid challenger is stealing, when the Candidate becomes safe, when the leader start bonus blocks stealing, when a Candidate swap happens, when the Final ends, or during reset/rematch.

Replicated Final steal readability:

- `AWizardStaffGameState` now mirrors:
  - `ReplicatedFinalStealingPlayerIndex`
  - `ReplicatedFinalStealProgressAlpha`
- These mirrors are clamped and forced inactive outside an active Final, after a winner is locked, or when no server-owned stealer is active.
- `ReplicatedFinalReadableSequence` advances when important readable Final state changes, including Candidate/winner/safety/stealer changes.
- Clients can display who is stealing and how close the steal is, but they cannot set progress or force success.

HUD/readability behavior:

- Canvas HUD compact player rows can tag the active stealer with `STEALING` from GameState fallback.
- Playtest mode can show `P# stealing X%` and a small steal progress bar from the mirrored GameState values when GameMode is unavailable.
- Minimal mode can show mirrored steal progress for remote-client smoke testing.
- Local/host HUD behavior still uses GameMode for richer Final status and existing debug/readability messages.

Authoritative versus visual-only:

- Authoritative: Final steal eligibility, circle checks, Candidate safe/vulnerable checks, hold time, progress reset, Candidate swap, Final timer, winner, scoring/Favor, rematch/reset.
- Visual/readability only: mirrored stealing player index, mirrored steal progress alpha, client HUD `STEALING` tag, client progress text/bar.

Still local-only or intentionally deferred after this pass:

- Production Final UI/VFX.
- Client-side prediction, rewind, or lag compensation for Final stealing.
- Broad replicated gameplay event/message feed.
- Final circle/marker art pass beyond current placeholder readability.
- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

Listen-server Final stealing scaffold smoke test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Progress to the Grand Wizard Final through the normal prototype flow or an existing safe dev flow.
4. Confirm both windows show Candidate, SAFE/VULNERABLE state, and Final timer.
5. Move the non-Candidate wizard into the ritual circle while the Candidate is vulnerable.
6. Confirm the server recognizes the eligible stealer without a client progress claim.
7. Confirm both windows show `P# stealing X%` and the progress bar.
8. Leave the circle or make the Candidate safe and confirm progress clears.
9. Complete a steal and confirm the server changes Candidate.
10. Confirm Candidate and `STEALING` readability update on both windows.
11. Bonk/ring out the stealing wizard if practical and confirm steal progress clears when the server marks the wizard respawning/out of arena.
12. Let the Final end and confirm both windows show the server-owned final winner.
13. Rematch/reset and confirm Candidate, SAFE/VULNERABLE, timer, winner, stealing player, and progress clear/reset.
14. Confirm a remote client cannot directly assign Candidate, winner, timer, steal progress, score, Favor, or round wins.

## 26. Grand Wizard Final Readable/Steal Cleanup Hardening Status

Completed as the eighteenth online spike:

- This was a focused code-level audit and hardening pass for the current Grand Wizard Final Candidate/timer/winner mirrors and steal progress readability scaffold.
- No new gameplay system, Trial, UI, VFX, tuning change, Steam/session feature, prediction, rewind, or lag compensation was added.
- The existing server-owned Final rules remain unchanged.

Bugs/stale-state risks found and fixed:

- `ReplicatedFinalReadableSequence` now advances on quantized steal progress milestones. This keeps the sequence useful for meaningful Final readability changes without incrementing every frame.
- Playtest HUD now shows the small steal progress bar only when a server-owned stealer is active. This prevents an empty progress bar from hanging around during safe/non-stealing Final states.

Checks performed where no code change was needed:

- GameMode remains the only authority for Candidate selection, Candidate swaps, SAFE/VULNERABLE calculation, steal eligibility, steal progress, Final timer, winner, scoring/Favor, and rematch/reset.
- No client Final steal request RPC exists. The server still infers steal intent from authoritative pawn position/circle checks.
- `IsWizardEligibleForGrandWizardSteal` blocks invalid, Candidate, respawning, pending-respawn, and out-of-arena wizards from accumulating steal progress.
- `GetBestGrandWizardCircleChallenger` and `UpdateGrandWizardFinalRound` both use server-owned eligibility before progress can accumulate.
- Candidate swap still clears the active stealer and hold time through existing GameMode logic.
- Final end still clears the active stealer and hold time before winner/result readability is mirrored.
- Full rematch/start-match reset still calls `ResetGrandWizardFinalRoundState`, clearing Candidate, winner, SAFE/VULNERABLE, stealing player, hold time, Final timer, feedback, and circle visibility.
- Client HUD fallback displays mirrored Candidate, SAFE/VULNERABLE, stealing player, steal progress, and winner state, but does not trigger gameplay.
- Compact player rows use mirrored state for `CANDIDATE`, `STEALING`, and `WINNER` tags when GameMode is unavailable.

Authority after this pass:

- Authoritative: Final active state, Candidate index, SAFE/VULNERABLE calculation, circle/eligibility checks, steal progress, Candidate swap, Final timer, final winner, score/Favor, round wins, rematch/reset.
- Visual/readability only: GameState mirrored Candidate/winner/timer, Candidate vulnerable flag, stealing player index, steal progress alpha, Final readable sequence, Canvas HUD tags/text/bar.

Updated listen-server Final stress test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Progress to the Grand Wizard Final through the normal prototype flow or an existing safe dev flow.
4. Confirm both windows show Candidate, SAFE/VULNERABLE state, and Final timer.
5. Move the non-Candidate into the ritual circle while the Candidate is vulnerable.
6. Confirm server-owned steal progress starts and both windows show `P# stealing X%` plus the progress bar.
7. Leave the ritual circle or make Candidate safe and confirm stealing player/progress/bar clear.
8. Complete a steal if practical and confirm the server swaps Candidate.
9. Confirm both host and client see Candidate, `STEALING`, and SAFE/VULNERABLE readability update correctly.
10. Bonk, clash, knock away, ring out, or respawn a Final participant if practical and confirm steal progress does not become stale.
11. Let the Final end and confirm both host/client see the server-owned winner.
12. Rematch/reset and confirm Candidate, SAFE/VULNERABLE, stealing player, steal progress, timer, winner, and result readability clear/reset.
13. Confirm a remote client cannot force Final authority.

Still local-only or intentionally deferred after this pass:

- Production Final UI/VFX.
- Client-side prediction, rewind, or lag compensation.
- Broad replicated gameplay event/message feed.
- Loose snapped segment physics and chaos effects replication.
- Full Arcane Pinball bounce/hit gameplay replication.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 27. Rematch And Reset Cleanup Hardening Status

Completed as the nineteenth online spike:

- This was a focused code-level audit and hardening pass for match, Trial, Results, Party Hall, Final, rematch, full-party reset, pawn reset, and replicated actor cleanup boundaries.
- No new gameplay system, Trial, spell, powerup, UI, VFX, tuning change, Steam/session feature, prediction, rewind, or lag compensation was added.
- The existing two-Trial local loop and current online scaffolding remain the baseline.

Bugs/stale-state risks found and fixed:

- `AWizardStaffManaMugPickup` now clears its `RespawnTimerHandle` during `EndPlay`; a later actor lifecycle pass expanded this teardown to remove the overlap delegate, disable collision/overlap events, mark the pickup inactive, and reapply hidden/inactive readability.
- Normal mug collection still hides the pickup and schedules respawn exactly as before.
- GameMode reset paths already deactivate mugs and clear their respawn timers through `SetPickupActive(false)`, while `EndPlay` protects destroyed or trimmed mug actors during rematch/full reset from stale timer, overlap, collision, or active/readable-state leftovers.

Checks performed where no code change was needed:

- `StartPartyMatch` already bumps `ReplicatedMatchSessionGeneration`, resets completed Trials, round wins, Favor, telemetry, Final state, Staffs state, Mug Run state, Arcane Pinball shells, and wizard state before returning to Party Hall.
- `ResetGrandWizardFinalRoundState` already clears Candidate, winner, result feedback, Final timer, stealing player index, steal progress, leader bonus, and circle visibility.
- `ResetWizardsForNewMatch` already resets staff/slosh/stress/reward/combat/broom/Mega Staff/readability state and respawns existing wizards rather than spawning duplicate pawns.
- `ResetMugRunPickups` already reuses or trims mug actors, reapplies tuning, moves them back to spawn locations, and deactivates them before the next Mug Run.
- `ResetStaffsAtDawnPowerups` already hides/destroys Staffs powerup actors and clears spawn/timer arrays.
- `CleanupArcanePinballProjectiles` already destroys existing Arcane Pinball projectile/readability actors at state boundaries that can leave Mug Run.
- `EndMugRunMatch` already clears carried brew rewards and reward readability.
- `EndStaffsAtDawnTrial` already clears Staffs powerups, recent ring-out attribution, broom recovery tracking, and active Mega Staff effects.
- `ClearPendingOutOfArenaRespawns` and `RespawnWizardInArena` already clear replicated respawn readability and movement/combat lock state.
- Quick Bonk and Staff Clash reset helpers already clear active swing/clash state and force replicated cancel/readability updates when appropriate.
- GameState Final mirrors already sanitize stealing player/progress outside an active Final and clear winner/result readability after `StartPartyMatch`.
- PlayerState mirrors are refreshed through `SyncReplicatedObservableState` and `SyncAllReplicatedPlayerStates`.

Authority safety after this pass:

- Authoritative: match state transitions, Trial transitions, rematch/reset, player round wins, Grand Wizard Favor, Trial scores, Final Candidate/winner/steal progress, pickup collection/respawn, carried reward ownership, Mega Staff grant/expiration, Quick Bonk, Staff Clash, ring-out/respawn, and replicated actor cleanup.
- Visual/readability only: GameState/PlayerState mirrors, HUD labels/timers/messages, replicated staff count visuals, Slosh/Stress meters, reward display, pickup active state, Mega Staff active/remaining/temp-segment readability, Quick Bonk/Staff Clash/readable respawn state, Final Candidate/steal/winner HUD state.
- Remote clients still cannot force reset/rematch authority, score, Favor, winner, Candidate, steal progress, pickup state, reward state, Mega Staff state, respawn state, or combat authority.

Updated listen-server rematch/reset stress test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm host/client possession and movement still work.
4. Complete or force the current two-Trial loop into the Grand Wizard Final.
5. During the run, exercise mug pickup, carried reward display, Arcane Pinball readability shell, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup pickup, Mega Staff, Final Candidate readability, and Final steal progress readability if practical.
6. Let the Final choose a winner.
7. Trigger rematch/reset through the existing flow.
8. Confirm both windows return to a clean Party Hall/intermission state.
9. Confirm each player has exactly one possessed wizard and stable display slot/color readability.
10. Confirm no stale mugs, Staffs powerups, Arcane Pinball shells, Mega Staff state, temporary segments, Quick Bonk, Staff Clash, respawn state, Candidate, steal progress, winner, reward, score, Favor, or HUD readability carries into the next match.
11. Start the next loop and confirm the first Trial begins cleanly.
12. Confirm a remote client cannot force reset authority or carry stale action authority across rematch.

Still local-only or intentionally deferred after this pass:

- Production online UI and Steam/session/lobby/invite flows.
- Replicated gameplay event/message feed.
- Full Arcane Pinball bounce/hit gameplay replication.
- Loose snapped segment physics and chaos effects replication.
- Prediction, rewind, or lag compensation.
- Broad scoring/Favor redesign, match-flow redesign, or final rematch architecture overhaul.
- New Trials, spells, Staffs at Dawn powerups, or Hammer Time.

## 28. Replicated Gameplay Event Feed Scaffolding Status

Completed as the twentieth online spike:

- This pass added a lightweight replicated gameplay event/message feed scaffold for listen-server readability.
- No gameplay tuning, new Trial, new spell, new powerup, UMG, production UI, Steam/session feature, prediction, rewind, lag compensation, loose physics replication, or full Arcane Pinball bounce/hit gameplay was added.
- GameMode/server remains the only authority that publishes events. Clients only display replicated event data.

Event feed location and fields:

- The feed lives in `AWizardStaffGameState`.
- The bounded replicated array is `ReplicatedGameplayEvents`.
- The feed sequence is `ReplicatedGameplayEventSequence`.
- Each `FWizardReplicatedGameplayEvent` stores:
  - `Sequence`
  - `EventType`
  - `PrimaryPlayerIndex`
  - `SecondaryPlayerIndex`
  - `NumericValue`
  - `ServerWorldTimeSeconds`
  - `MatchSessionGeneration`
  - `DisplayText`
- Event payloads use player slots/indices instead of actor references.
- Display text is capped and the feed is capped to 8 recent events.

Server publish path:

- `AWizardStaffGameMode::PublishReplicatedGameplayEvent` is the server-side publishing gate.
- `AWizardStaffGameMode::ClearReplicatedGameplayEventFeed` clears the feed through GameState.
- `AWizardStaffGameState::IncrementMatchSessionGeneration` clears the feed on full rematch/new-match generation changes.
- The feed is also cleared at new Trial countdown, Party Hall re-entry after completed Trials, and Grand Wizard Final entry so old mug/powerup/combat messages do not linger in the wrong phase.
- Some events also feed the existing local Canvas HUD message list so standalone and listen-server host readability stays intact.
- Existing local GameMode/HUD feedback remains in place where it already existed; replicated feed display is mainly used by remote clients without GameMode access to avoid duplicate host messages.

Currently routed event types:

- `RematchStarted`: new match started.
- `MugPickup`: server-validated mug pickup collected.
- `BrewRewardGranted`: server-owned brew reward grant.
- `ArcanePinballShell`: legacy/fallback event for the old server-owned Arcane Pinball readability shell path.
- `ArcanePinballCast`: validated network Use Reward spawned the server-owned Arcane Pinball gameplay projectile.
- `ArcanePinballHit`: server-confirmed Arcane Pinball hit or self-hit.
- `RingOutPending`: server started an out-of-arena respawn delay.
- `RespawnComplete`: server completed a respawn.
- `MegaStaffGranted`: server-validated Staffs at Dawn powerup granted Mega Staff Brew.
- `MegaStaffExpired`: server-owned Mega Staff Brew expiration completed.
- `StaffClashStarted`: server-owned Staff Clash started.
- `StaffClashResolved`: server-owned Staff Clash win/tie resolved.
- `GrandWizardCandidateChanged`: initial Final Candidate and Candidate swaps.
- `FinalWinner`: server-owned Grand Wizard Final winner selected.

Intentionally not routed:

- Slosh decay ticks.
- Staff Stress decay ticks.
- timer ticks.
- steal progress ticks.
- repeated overlap checks.
- every telemetry counter.
- loose snapped segment chaos events.
- every Arcane Pinball bounce, movement update, hit cooldown check, or overlap check.

HUD/readability behavior:

- The existing Canvas HUD remains the only HUD system.
- Playtest and FullDebug modes keep using the existing local HUD feed on host/standalone.
- Remote clients without GameMode/local HUD feedback draw the replicated GameState feed in the same lower-left footprint.
- Minimal mode can show the newest replicated events for remote smoke testing without adding a larger panel.
- Event feed data never drives gameplay, score, Favor, Candidate state, winner state, pickup state, reward state, Mega Staff, respawn, or combat effects.

Authority after this pass:

- Authoritative: GameMode match state, Trial state, scoring, Favor, pickup collection, reward ownership, Arcane Pinball gameplay projectile spawn/hit effects, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup collection, Mega Staff grant/expiration, Final Candidate/steal/winner, rematch/reset.
- Visual/readability only: replicated gameplay events, event sequence, player indices in event payloads, numeric event values, display text, and Canvas HUD event-feed rendering.
- Remote clients cannot add events, edit events, or use event data to force gameplay state.

Listen-server event feed smoke test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm host/client possession and movement still work.
4. Enter the current two-Trial loop.
5. Trigger several routed server-owned events: collect a mug, grant a brew reward, cast Arcane Pinball if practical, hit with Arcane Pinball if practical, Quick Bonk/Staff Clash if practical, ring-out/respawn if practical, collect Staffs powerup, grant/expire Mega Staff if practical, reach Final, swap Candidate if practical, choose Final winner, and rematch/reset.
6. Confirm the remote client sees the replicated event feed without needing GameMode access.
7. Confirm the listen-server host does not get duplicate messages from local feedback plus replicated feed.
8. Confirm the feed does not spam per-frame messages.
9. Confirm old events clear at Trial transitions, Final entry, Party Hall re-entry, and rematch/new-match generation.
10. Confirm a remote client cannot author fake events or force gameplay through events.

Still local-only or intentionally deferred after this pass:

- Production online UI and UMG conversion.
- Steam sessions, lobbies, matchmaking, invites, and public lobby browser.
- Arcane Pinball prediction, exact cross-client projectile visual fidelity, and production VFX/polish.
- Loose snapped segment physics and chaos effects replication.
- Prediction, rewind, or lag compensation.
- Broad scoring/Favor redesign, match-flow redesign, or production notification system.
- Chat, emotes, analytics pipeline, new Trials, new spells, new powerups, or Hammer Time.

## 29. Arcane Pinball Server-Owned Bounce/Hit Gameplay Scaffolding Status

Completed as the twenty-first online spike:

- Networked Arcane Pinball no longer stops at a readability-only shell after a validated `ServerUseReward` request.
- The existing `AWizardStaffArcanePinballProjectile` is reused for networked gameplay instead of adding a second projectile class.
- The old `InitializeArcanePinballReadabilityShell` path remains available as a fallback/historical scaffold, but network reward use now calls the full gameplay initializer through `FireArcanePinball`.
- No new spell, reward, powerup, VFX, production UI, tuning change, prediction, rewind, lag compensation, Steam/session feature, loose physics replication, or match-flow redesign was added.

Server-owned network path:

- The owning client still routes Use Reward through the validated `ServerUseReward` seam.
- The server validates possession, input lock, Staff Clash lockout, and authoritative carried reward state.
- If the carried reward is Arcane Pinball, the server spawns one gameplay projectile with the existing tuning values.
- The carried reward is cleared only after the server projectile spawn succeeds.
- If spawn fails, the reward remains held and a low-noise warning is logged.
- Cast stress, cast telemetry, and replicated reward readability clear through the existing authority paths.

Projectile movement, bounce, and speed-up:

- The server owns projectile movement truth through the existing projectile movement component and replicated actor movement.
- The server owns bounce detection, bounce count, speed-up after bounce, max bounce destruction, lifetime destruction, and height-lock behavior.
- Clients display the replicated projectile transform/trail and do not author movement, bounces, speed, or cleanup.
- Projectile collision is now disabled on non-authority clients so the replicated projectile cannot cause client-side collision or gameplay side effects.

Hit confirmation and gameplay effects:

- The server owns hit detection and hit confirmation through the existing projectile hit callback.
- Client-side hit callbacks return without gameplay effects.
- The server respects the existing self-hit setting, hit cooldown, repeat-hit prevention, caster tracking, and destroy-on-player-hit tuning.
- Server-confirmed hits apply the existing Arcane Pinball knockback, Mana Slosh, Staff Stress, hit telemetry, and optional destroy-on-hit behavior.
- Clients cannot submit hit targets, fake hit results, force knockback, grant Slosh/Stress, or force cleanup through projectile visuals.

Replicated event feed integration:

- `ArcanePinballCast` is published when a validated network Use Reward spawns the gameplay projectile.
- `ArcanePinballHit` is published when the server confirms a hit or self-hit.
- The older `ArcanePinballShell` event remains available for the fallback shell path.
- Bounces, movement updates, cooldown checks, and overlap checks are intentionally not routed to the feed.
- Event-feed messages remain display-only and never drive gameplay.

Cleanup and reset behavior:

- Projectile lifetime, max bounces, and existing destroy-on-hit behavior still destroy the projectile on the server.
- Existing GameMode cleanup still destroys Arcane Pinball projectiles at Mug Run end, Staffs at Dawn start, Grand Wizard Final setup, Results/Party Hall transitions, rematch/reset, and full party restart boundaries.
- Destroyed server projectiles replicate destruction to clients.
- Stale projectiles should not survive into the next Trial or match generation.

Standalone local preservation:

- Standalone local play still uses the existing full `FireArcanePinball` path.
- Local Arcane Pinball bounces, hits, self-hits, Slosh/Stress effects, telemetry, cleanup, one-human-versus-bot play, and couch multiplayer behavior were not intentionally retuned.
- Local HUD feedback remains in place.

Listen-server Arcane Pinball gameplay smoke test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm host/client possession and movement still work.
4. Enter Mug Run through the normal prototype flow or an existing safe dev flow.
5. Have the host wizard collect mugs until Arcane Pinball is granted.
6. Use Reward on the host wizard.
7. Confirm the server validates the reward use, clears reward readability, and spawns one gameplay Arcane Pinball projectile.
8. Confirm both host and client see the projectile spawn and move.
9. Bounce the projectile if practical and confirm bounces/speed-up are server-owned.
10. Hit the client-owned wizard if practical and confirm the server applies knockback plus Slosh/Stress effects.
11. Confirm Slosh/Stress mirrors update after the hit.
12. Repeat with the client-owned wizard using Arcane Pinball.
13. Confirm self-hit still follows existing rules if practical.
14. Confirm cleanup after hit, max bounces, lifetime, Trial transition, and rematch/reset.
15. Confirm a remote client cannot spawn, hit, bounce, apply effects, or clean up Arcane Pinball directly.
16. Confirm event-feed messages stay low-noise and do not list every bounce.

Still local-only or intentionally deferred after this pass:

- Prediction, rewind, and lag compensation.
- Exact cross-client projectile visual fidelity beyond replicated movement readability.
- Production Arcane Pinball VFX/UI polish.
- Loose snapped segment physics replication.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 30. Arcane Pinball Cleanup/Stale-State Hardening Status

Completed as the twenty-second online spike:

- This was a focused cleanup pass for the server-owned Arcane Pinball projectile path.
- No new gameplay system, spell, reward, VFX, production UI, tuning change, prediction, rewind, lag compensation, Steam/session feature, loose physics replication, or match-flow redesign was added.

Bugs/stale-state risks found and fixed:

- `AWizardStaffArcanePinballProjectile` now has an explicit `bArcanePinballGameplayEnded` guard.
- Hit, bounce, and hit-application callbacks now return if projectile gameplay has already ended.
- Max-bounce cleanup and destroy-on-player-hit cleanup now deactivate projectile gameplay before calling `Destroy`.
- `EndPlay` now deactivates projectile gameplay, disables collision, removes hit/bounce delegates, stops projectile movement, deactivates the movement component, and clears recent-hit cooldown tracking.
- This prevents late callbacks during destruction, Trial cleanup, rematch cleanup, or actor end play from applying duplicate hit effects, duplicate telemetry, duplicate event-feed messages, or stale collision.

Checks performed where no code change was needed:

- Networked Use Reward already routes through the validated `ServerUseReward` path.
- Server validation already checks authority-side possession, input lock, Staff Clash lockout, and authoritative carried reward state.
- Repeated Use Reward input cannot spend the same carried reward twice because the server clears the carried reward immediately after a successful projectile spawn.
- Failed projectile spawn leaves the carried reward held and only logs a low-noise warning.
- The server still spawns one projectile through the existing `FireArcanePinball` path and uses the existing Arcane Pinball tuning values.
- Non-authority projectile collision is already disabled, so remote clients display projectile movement/trails without applying gameplay collision.
- Bounce count, speed-up, max bounces, lifetime, height lock, hit confirmation, self-hit rules, hit cooldown, repeat-hit prevention, caster tracking, Slosh, Stress, knockback, and telemetry all remain server-owned.
- `ArcanePinballCast` is only published after successful server projectile spawn.
- `ArcanePinballHit` is only published from a server-confirmed hit.
- Bounces, movement updates, overlap checks, and cooldown checks are still intentionally not routed to the event feed.
- Existing GameMode cleanup already destroys Arcane Pinball projectiles when leaving Mug Run and at the other established Trial/rematch/reset boundaries.

Authority after this pass:

- Authoritative: Use Reward validation, projectile spawn, projectile movement truth, bounce count, speed-up, max-bounce/lifetime/destroy-on-hit cleanup, hit confirmation, knockback, Mana Slosh, Staff Stress, telemetry, and projectile destruction.
- Visual/readability only: replicated projectile movement/trails and Arcane Pinball event-feed messages on clients.
- Remote clients still cannot spawn Arcane Pinball directly, use another wizard's reward, submit hit targets, fake bounces, force knockback, grant Mana Slosh or Staff Stress, force cleanup, or drive score/Favor/Candidate/winner/respawn/rematch from projectile visuals.

Updated listen-server Arcane Pinball stress test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Confirm host/client possession and movement still work.
4. Enter Mug Run through the normal prototype flow or an existing safe dev flow.
5. Have the host wizard collect mugs until Arcane Pinball is granted.
6. Use Reward on the host wizard and confirm one server-owned projectile spawns.
7. Bounce the projectile and confirm bounce count/speed-up are server-owned.
8. Hit the client-owned wizard and confirm server-owned hit confirmation, knockback, Slosh/Stress mirrors, and low-noise event-feed readability.
9. Repeat with the client-owned wizard using Arcane Pinball.
10. Try self-hit and repeat-hit timing if practical.
11. Let cleanup happen by hit, max bounces, lifetime, Mug Run end, Trial transition, and rematch/reset.
12. Confirm no stale projectile, duplicate hit effect, duplicate telemetry, duplicate event message, or stale reward readability remains.
13. Confirm a remote client cannot spawn, hit, bounce, clean up, or apply Arcane Pinball effects directly.

Standalone local preservation:

- Standalone local Arcane Pinball still uses the existing full gameplay path.
- Local bounces, hits, self-hits, Slosh/Stress/knockback/telemetry, cleanup, one-human-versus-bot play, couch multiplayer, and the current two-Trial loop were not intentionally changed.

Still local-only or intentionally deferred after this pass:

- Prediction, rewind, and lag compensation.
- Exact cross-client projectile visual correction beyond replicated movement readability.
- Production Arcane Pinball VFX/UI polish.
- Loose snapped segment physics replication.
- Steam sessions, lobbies, matchmaking, invites, lobby browser, and production online UI.

## 31. Listen-Server End-To-End Scaffolding Stability Pass Status

Completed as the twenty-third online spike:

- This was a focused code-level audit of the current listen-server scaffolding across Party Hall, Mug Run, Staffs at Dawn, Grand Wizard Final, Results, and rematch/reset boundaries.
- No new gameplay system, Trial, spell, reward, powerup, UI, VFX, tuning change, Steam/session feature, prediction, rewind, lag compensation, loose physics replication, scoring redesign, Final redesign, Trial-order change, or rematch-flow redesign was added.
- Codex did not claim human feel validation or a live two-window PIE playtest. The listed smoke test remains the manual validation path.

Bugs/stale-state risks found and fixed:

- `AWizardStaffStaffsAtDawnPowerupPickup` now has explicit `EndPlay` cleanup.
- Staffs powerup pickup end play now removes the overlap delegate, disables collision, disables overlap events, marks the pickup inactive, and applies hidden/inactive readability before the actor is destroyed.
- This protects Staffs at Dawn transition/rematch cleanup from stale overlap bindings or stale active pickup readability on destroyed powerup actors.

Checks performed where no code change was needed:

- Local-player creation and playtest bot filling remain gated to standalone local prototype sessions.
- In listen-server mode, desired player count falls back to connected PlayerControllers and does not create extra local players.
- GameState mirrors still carry party state, Trial state/type, active preset, timers, completed Trial count, Final Candidate/winner/steal readability, result text, match generation, and bounded gameplay events.
- PlayerState mirrors still carry stable display slot/color, round wins, Favor, current score, Staffs score, ready state, bot flag, and summary text.
- Match session generation still advances through `BumpMatchSessionGeneration`, and GameState clears the replicated event feed when generation changes.
- The replicated event feed remains bounded to 8 entries and uses match generation to avoid drawing stale messages on remote HUDs.
- Party Hall, countdown, Trial, Results, Final, and rematch state changes all call `SyncReplicatedObservableState` through the existing tick/end-of-scope path.
- Mug Run pickup collection remains server-owned; mugs hide/respawn through replicated active state, clear respawn timers on deactivation/end play, and grant staff/Slosh/rewards through existing authority paths.
- Use Reward remains ownership-aware, Arcane Pinball spawn/hit/effects remain server-owned, and projectile cleanup is hardened by the recent Arcane Pinball pass.
- Quick Bonk remains routed through the owning pawn/server request seam in networked play, with server-owned impact timing and hit resolution.
- Staff Clash remains server-started from eligible server-confirmed bonk flow, with server-owned participant validation, mash counts, resolve, and cleanup.
- Ring-out/respawn remains server-owned, clears pending respawn readability, clears recent bonk attribution, and resets Staffs staff state through existing authority paths.
- Staffs at Dawn powerup pickup active/hidden state remains replicated, collection remains server-owned, and GameMode-owned respawn timers reset with the Staffs powerup arrays.
- Mega Staff grant/expiration/temp-segment readability remains server-owned and clears through existing Staffs end, transition, respawn/reset, and rematch paths.
- Grand Wizard Final Candidate, SAFE/VULNERABLE, steal progress, timer, winner, and result readability remain mirrored through GameState while GameMode owns all Final authority.
- HUD Playtest/Minimal remote paths continue using GameState/PlayerState mirrors for core readable state instead of direct GameMode access.
- Replicated mirrors and event-feed data remain display-only and do not drive authoritative gameplay on clients.

Authority after this pass:

- Authoritative: match flow, Trial flow, timers, score, Favor, ready state, player identity assignment, mug pickup collection, reward ownership/use, Arcane Pinball gameplay projectile, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup collection, Mega Staff grant/expiration/temp-segment cleanup, Final Candidate/steal/winner, rematch/reset, and actor cleanup.
- Visual/readability only: GameState/PlayerState mirrors, HUD rows/timers/labels/meters, replicated staff count visuals, Slosh/Stress readability, carried reward display, pickup active state, Arcane Pinball movement/trails/event messages, Quick Bonk/Staff Clash readability, ring-out countdown readability, Mega Staff readability, Final Candidate/steal/winner readability, and replicated event-feed messages.
- Remote clients still cannot force match state, Trial state, score, Favor, staff segments, Slosh/Stress, pickup collection, reward use for another wizard, Arcane Pinball hits/effects, Quick Bonk hit targets, Staff Clash start/resolve/mash for another wizard, ring-out/respawn, Mega Staff, Final Candidate/steal/winner, rematch/reset, or event-feed authoring.

Updated listen-server end-to-end smoke test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Enable separate windows if practical.
4. Confirm host/client possession and movement still work.
5. Enter Party Hall and confirm basic mirrored state/HUD readability.
6. Progress to Mug Run.
7. Have host and client-owned wizards collect mugs and confirm pickup hide/respawn, staff count, Slosh, reward display, and event-feed readability.
8. Grant/use Arcane Pinball if practical and confirm server-owned projectile spawn, movement, hit/effects, cleanup, and low-noise cast/hit messages.
9. Progress to Staffs at Dawn.
10. Exercise Quick Bonk, Staff Clash if practical, ring-out/respawn if practical, Staffs powerup pickup, and Mega Staff if practical.
11. Confirm server authority and replicated readability for combat, respawn, powerup, and Mega Staff state.
12. Progress to Grand Wizard Final.
13. Confirm Candidate, SAFE/VULNERABLE, steal progress, timer, winner, and event-feed readability.
14. Let the Final choose a winner.
15. Trigger rematch/reset.
16. Confirm both windows return to clean Party Hall/intermission state.
17. Confirm each player has exactly one possessed wizard.
18. Confirm no stale mugs, rewards, Arcane Pinball projectiles, Staffs powerups, Mega Staff, Quick Bonk, Staff Clash, respawn state, Candidate, steal progress, winner, score, Favor, HUD labels, or event-feed messages carry into the next match.
19. Start the next loop and confirm the first Trial begins cleanly.
20. Confirm a remote client cannot force authority at any point.

Standalone local preservation:

- Standalone local PIE startup, one-human-versus-bot, local couch multiplayer, Mug Run, Use Reward/Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup/Mega Staff, Grand Wizard Final, and the current Party Hall -> Mug Run -> Staffs at Dawn -> Grand Wizard Final -> rematch loop were not intentionally changed.

Still local-only or intentionally deferred after this pass:

- Steam sessions, lobbies, matchmaking, friend invites, public lobby browser, and production online UI.
- Prediction, rewind, lag compensation, and exact cross-client physics fidelity.
- Loose snapped segment physics replication.
- Production notification system, chat, emotes, analytics pipeline, final VFX/UI, new Trials, new spells, new rewards, new powerups, Hammer Time, broad scoring/Favor redesign, or broad match-flow redesign.

## 32. Replicated Actor Lifecycle/Reset Cleanup Pass Status

Completed as the twenty-fourth online spike:

- This was a focused code-level audit of spawned/replicated actor lifecycle cleanup across Mug Run pickups, Staffs at Dawn powerup pickups, Arcane Pinball projectiles, wizard pawns/components, Mega Staff state, Grand Wizard Final readability state, loose segment placeholders, helper actors, and the replicated event feed.
- No new gameplay system, Trial, spell, reward, powerup, UI, VFX, tuning change, Steam/session feature, prediction, rewind, lag compensation, loose physics replication, scoring redesign, Final redesign, Trial-order change, or rematch-flow redesign was added.
- Codex did not claim human feel validation or a live two-window PIE playtest. The listed smoke test remains the manual validation path.

Bugs/stale-state risks found and fixed:

- `AWizardStaffManaMugPickup` now performs explicit `EndPlay` cleanup. It clears the respawn timer, removes the overlap delegate, disables collision, disables overlap events, marks the pickup inactive, and reapplies hidden/inactive readability before destruction completes.
- `AWizardStaffWizardCharacter` now removes its `UWizardStaffComponent::OnStaffSegmentSnapped` dynamic delegate binding during `EndPlay`. This prevents staff snap callbacks from targeting a pawn that is already being destroyed or torn down during rematch/reset/editor cleanup.

Checks performed where no code change was needed:

- Mug Run pickups already use server-owned collection, replicated active/hidden state, collision gated by active state, `SetPickupActive(false)` timer clearing, and GameMode reset/reposition logic.
- Staffs at Dawn powerup pickups already use server-owned collection, replicated active/hidden state, GameMode-owned respawn countdown arrays, reset-time actor destruction, and the earlier explicit `EndPlay` overlap/collision cleanup.
- Arcane Pinball projectiles already deactivate gameplay idempotently, remove hit/bounce delegates, disable collision, stop projectile movement, clear recent hit tracking, and guard hit/bounce callbacks with authority/readability/gameplay-ended checks.
- Wizard pawn reset already clears Quick Bonk, Staff Clash, respawn readability, carried reward readability, Mega Staff readability, movement state, staff visuals, Slosh, and Stress through existing reset paths.
- Runtime staff visuals already rebuild from replicated segment count without retaining stale visual components.
- Mega Staff state already remains server-owned, clears on expiration/reset/Staffs end, tracks remaining temporary segments, and mirrors active/remaining/temp-count readability without letting clients mutate gameplay.
- Grand Wizard Final readability remains GameMode/GameState-owned; the Final circle actor is hidden outside Final, collision stays disabled, Candidate/SAFE/VULNERABLE/steal/winner mirrors clear through Final/reset state, and clients cannot use readability state for authority.
- Replicated gameplay events remain bounded, generation-tagged, server-published only, and display-only. The feed clears on generation changes and at major state boundaries.
- Loose snapped segment placeholders remain local/server-side prototype cleanup only; this pass did not replicate loose physics or change loose segment behavior.
- Shared camera, arena, Party Hall, and debug/helper actors were not changed because no stale delegate/timer/authority hole was found in the audited paths.

Authority after this pass:

- Authoritative: match flow, Trial flow, timers, score, Favor, ready state, player identity assignment, mug pickup collection, reward ownership/use, Arcane Pinball gameplay projectile, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup collection, Mega Staff grant/expiration/temp-segment cleanup, Final Candidate/steal/winner, rematch/reset, actor spawning, actor deactivation, and actor destruction.
- Visual/readability only: GameState/PlayerState mirrors, HUD rows/timers/labels/meters, replicated staff count visuals, Slosh/Stress readability, carried reward display, pickup active state, Arcane Pinball movement/trails/event messages, Quick Bonk/Staff Clash readability, ring-out countdown readability, Mega Staff readability, Final Candidate/steal/winner readability, Final circle visibility, and replicated event-feed messages.
- Remote clients still cannot force pickup collection, pickup respawn, actor spawning/destruction, Arcane Pinball hits/effects, Mega Staff activation/expiration, Final state, score/Favor, rematch/reset, or event-feed authoring.

Two-client actor lifecycle/reset smoke test:

1. Start PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Enable separate windows if practical.
4. Confirm host/client possession and movement.
5. Enter Mug Run.
6. Collect multiple mugs with host and client-owned wizard.
7. Confirm pickup hide/respawn readability, no duplicate mugs, and no client-owned collection authority.
8. Use Arcane Pinball if practical.
9. Confirm one projectile spawns per accepted reward use, then confirm movement, hit/cleanup, and no duplicate projectile from one use.
10. Transition out of Mug Run with pickup/projectile state active if practical.
11. Confirm no stale mugs/projectiles survive into Staffs at Dawn.
12. Enter Staffs at Dawn.
13. Collect Staffs powerups with host and client-owned wizard if practical.
14. Confirm pickup hide/respawn readability, no duplicate powerups, and server-owned collection.
15. Trigger Mega Staff if practical, then transition to Final or rematch/reset and confirm Mega Staff readability/temp-segment state clears safely.
16. Enter Grand Wizard Final.
17. Confirm Final readability state appears once and clears after Final/rematch.
18. Rematch/reset.
19. Confirm no stale pickups, projectiles, powerups, Mega state, Final readability, timers, delegates, HUD labels, or event messages survive into Party Hall.
20. Start the next loop and confirm the first Trial begins cleanly.
21. Confirm remote clients cannot force actor authority at any point.

Standalone local preservation:

- Standalone local PIE startup, one-human-versus-bot, local couch multiplayer, Mug Run pickups, Staffs powerups, Use Reward/Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, Mega Staff, Grand Wizard Final, and the current Party Hall -> Mug Run -> Staffs at Dawn -> Grand Wizard Final -> rematch loop were not intentionally changed.

Still local-only or intentionally deferred after this pass:

- Steam sessions, lobbies, matchmaking, friend invites, public lobby browser, and production online UI.
- Prediction, rewind, lag compensation, exact cross-client physics fidelity, and loose snapped segment physics replication.
- Production notification system, chat, emotes, analytics pipeline, final VFX/UI, new Trials, new spells, new rewards, new powerups, Hammer Time, broad scoring/Favor redesign, or broad match-flow redesign.

## 33. Two-Client Listen-Server Manual Smoke-Test Support Status

Completed as the twenty-fifth online spike:

- This was requested as a focused two-client listen-server manual smoke-test and bugfix pass.
- Live two-window PIE was not run or directly observed in this Codex environment. This environment does not provide a controllable Unreal Editor PIE session where Codex can set Number of Players = `2`, set Net Mode = `Play As Listen Server`, start PIE, and inspect both host/client windows.
- No human feel validation or live PIE observation is claimed.
- Because live PIE was unavailable, this pass performed a code-level readiness sweep across the same systems the manual smoke test should exercise.
- No new gameplay system, Trial, spell, reward, powerup, UI, VFX, tuning change, Steam/session feature, prediction, rewind, lag compensation, loose physics replication, scoring redesign, Final redesign, Trial-order change, or rematch-flow redesign was added.
- No new diagnostic helper was added because the existing low-noise authority/client logs, replicated mirrors, HUD readability, and event feed are enough for the next manual run without adding more debug surface.

Concrete listen-server scaffold bugs found and fixed:

- None in this pass. The code-level sweep did not identify a concrete possession, authority, stale-state, actor lifecycle, reset-generation, HUD readability, event-feed, or replicated-readability bug that should be patched without live two-window observation.

Checks performed where no code change was needed:

- Party Hall/identity: `PostLogin`, `RestartPlayer`, `AssignOnlineScaffoldPlayerSlot`, PlayerState display slot/color mirrors, and standalone-gated local player/bot helpers remain set up so a listen-server host/client can each receive one controlled wizard without creating extra local-only players.
- Mug Run: mug collection remains authority-gated, pickup active/hidden state replicates, respawn is server-timer owned, staff count/Slosh/reward mirrors update through server paths, and mug `EndPlay` cleanup now covers timers/delegates/collision/readability.
- Use Reward/Arcane Pinball: Use Reward routes through the owning pawn/server seam in networked play, consumes reward only after successful server projectile spawn, keeps projectile movement/bounce/hit/effects server-owned, and uses hardened projectile cleanup.
- Staffs at Dawn combat: Quick Bonk, Staff Clash, ring-out/respawn, scoring/Favor attribution, and combat readability still use server-owned paths with client readability mirrors only.
- Staffs powerup/Mega Staff: powerup collection remains server-owned, active/hidden/respawn state replicates, EndPlay cleanup is explicit, Mega Staff grant/expiration/temp-segment state remains server-owned, and readability mirrors remain display-only.
- Grand Wizard Final: Candidate, SAFE/VULNERABLE, steal progress, timer, winner/result readability remain GameState mirrors while GameMode owns Candidate selection, steal progress, winner, score, Favor, and reset.
- Event feed: server is still the only publisher, messages remain bounded/generation-tagged/display-only, and no per-frame feed source was found in the audited routes.
- Rematch/reset: match generation bumping, event-feed clearing, actor cleanup, wizard reset, pickup/powerup/projectile cleanup, Final readability reset, and PlayerState/GameState mirror refresh paths remain in place.

Prepared manual two-client listen-server smoke-test path:

1. Start Unreal Editor PIE with Number of Players = `2`.
2. Set Net Mode = `Play As Listen Server`.
3. Enable separate windows if practical.
4. Confirm host and client windows appear.
5. Confirm each window has exactly one possessed wizard.
6. Confirm each player controls only their own wizard and movement/facing are visible.
7. In Party Hall, confirm display slot/color, ready/intermission/countdown, and HUD readability are stable.
8. Progress to Mug Run and collect mugs with host and client-owned wizards.
9. Confirm server-owned mug collection, pickup hide/respawn, staff count, Slosh, reward display, and event-feed readability on both windows.
10. Grant/use Arcane Pinball if practical and confirm one server projectile per accepted use, readable movement, server-owned bounce/hit/effects, and cleanup across hit/lifetime/transition/reset.
11. Progress to Staffs at Dawn and exercise Quick Bonk, Staff Clash if practical, ring-out/respawn if practical, Staffs powerup pickup, and Mega Staff if practical.
12. Confirm combat/powerup/Mega Staff authority remains server-owned and readability appears on both windows.
13. Progress to Grand Wizard Final and confirm Candidate, SAFE/VULNERABLE, steal progress, timer, winner/result, and event-feed readability.
14. Let Final choose a winner, then trigger rematch/reset.
15. Confirm both windows return to clean Party Hall/intermission state with exactly one possessed wizard per player.
16. Confirm no stale mugs, rewards, Arcane Pinball projectiles, Staffs powerups, Mega Staff, Quick Bonk, Staff Clash, respawn state, Candidate, steal progress, winner, score, Favor, HUD labels, or event-feed messages carry into the next loop.
17. Start the next loop and confirm the first Trial begins cleanly.

Authority after this pass:

- Authoritative: match flow, Trial flow, timers, score, Favor, ready state, player identity assignment, mug pickup collection, reward ownership/use, Arcane Pinball gameplay projectile, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup collection, Mega Staff grant/expiration/temp-segment cleanup, Final Candidate/steal/winner, rematch/reset, actor spawning, actor deactivation, and actor destruction.
- Visual/readability only: GameState/PlayerState mirrors, HUD rows/timers/labels/meters, replicated staff count visuals, Slosh/Stress readability, carried reward display, pickup active state, Arcane Pinball movement/trails/event messages, Quick Bonk/Staff Clash readability, ring-out countdown readability, Mega Staff readability, Final Candidate/steal/winner readability, Final circle visibility, and replicated event-feed messages.

Standalone local preservation:

- Standalone local PIE startup, one-human-versus-bot, local couch multiplayer, Mug Run pickups, Use Reward/Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup/Mega Staff, Grand Wizard Final, and the current Party Hall -> Mug Run -> Staffs at Dawn -> Grand Wizard Final -> rematch loop were not intentionally changed.

Still local-only or intentionally deferred after this pass:

- Manual two-window PIE validation remains required.
- Steam sessions, lobbies, matchmaking, friend invites, public lobby browser, and production online UI.
- Prediction, rewind, lag compensation, exact cross-client physics fidelity, and loose snapped segment physics replication.
- Production notification system, chat, emotes, analytics pipeline, final VFX/UI, new Trials, new spells, new rewards, new powerups, Hammer Time, broad scoring/Favor redesign, or broad match-flow redesign.

## 34. Two-Client Listen-Server Presentation Bugfix Status

Completed as the twenty-sixth online spike:

- This pass fixed concrete issues reported from an actual two-client listen-server PIE smoke attempt.
- Observed issue: with Number of Players = `2` and Net Mode = `Play As Listen Server`, the host/client windows were using a pawn/first-person-style view instead of the top-down shared camera used by standalone.
- Observed issue: at least one window could not see the runtime arena walls/placeholder meshes and saw mostly the open world map, even though gameplay interactions like bonk knockback and Arcane Pinball appeared to function.
- Codex did not run or directly observe the PIE windows. The symptoms above were human-observed and reported from the manual smoke test.

Fixes made:

- `AWizardStaffSharedCamera` now replicates and is always relevant so clients receive the camera actor, but camera movement is locally smoothed instead of replicated as actor movement. This avoids the secondary PIE client seeing packet-rate camera jitter while the server viewport looks smooth.
- In standalone/listen-server authority worlds, the shared camera keeps using PlayerController pawns for its bounds. On network clients, it gathers replicated wizard pawns directly because remote PlayerControllers are not available there.
- `AWizardStaffGameMode::SpawnSharedCamera`, `AssignSharedCameraToAllPlayers`, and `AssignSharedCameraToPlayer` are no longer standalone-only. The shared camera now exists for listen-server PIE and is assigned to host/client PlayerControllers.
- `RestartPlayer` now reassigns the shared camera after pawn possession so controller auto-camera management does not leave a newly possessed listen-server pawn in pawn view.
- `AssignSharedCameraToPlayer` disables auto camera management for that PlayerController, calls `SetViewTarget`, and sends `ClientSetViewTarget` to remote clients.
- Follow-up fix: the replicated shared camera now also applies itself to any local PlayerController in its own `BeginPlay`/`Tick` path. This covers the secondary PIE client window when the server-side `ClientSetViewTarget` arrives before the replicated camera actor is ready on that client.
- Follow-up fix: the shared camera now ticks in `TG_PostUpdateWork` so the client samples pawn positions after movement smoothing. This should reduce the remaining small camera jitter without changing gameplay authority.
- Runtime `AWizardStaffPrototypeArena`, `AWizardStaffStaffsAtDawnArena`, and `AWizardStaffPartyHall` actors now replicate as always-relevant presentation/gameplay scaffold actors. This lets remote clients receive the runtime-spawned placeholder walls, floors, signs, and arena meshes instead of seeing only the base map.
- Follow-up fix: client-owned broom boost input now routes through a small `ServerRequestBroomBoost` RPC. The server validates possession/input/clash/falling state and runs the existing boost path; clients receive a tiny replicated broom-visual flag for readability.
- Follow-up fix: networked Arcane Pinball clients no longer run local projectile movement/bounce simulation for the server-owned projectile. Clients display replicated movement only, the projectile net update rate is higher, and server bounce events force a net update so post-bounce direction/trail readability is cleaner.
- Follow-up fix: the Grand Wizard Final ritual circle now uses `AWizardStaffFinalRitualCircle`, a small replicated readability actor with constructor-owned mesh/material setup plus replicated center/radius/color/visibility. This replaces the previous server-only runtime-configured `AStaticMeshActor` that clients could miss entirely.
- Follow-up fix: the prototype input lock now mirrors to clients through `bReplicatedPrototypeInputLocked`, and local input also consults replicated GameState phase mirrors. This prevents owning clients from locally moving, jumping, broom boosting, bonking, or using rewards during Trial countdown/results/setup windows before Mug Run, Staffs at Dawn, or Final are actually active.

What still needs manual confirmation:

- Re-run PIE with Number of Players = `2` and Net Mode = `Play As Listen Server`.
- Confirm both windows start in the top-down shared-camera view.
- Confirm both windows can see the Party Hall and runtime prototype arena meshes.
- Confirm both windows can see Staffs at Dawn arena meshes after transition.
- Confirm the secondary client camera jitter is reduced enough for smoke testing.
- Confirm client-owned broom boost works after pressing `Space` while falling.
- Confirm Arcane Pinball bounces/trail readability are cleaner in the client window.
- Confirm the Grand Wizard Final ritual circle is visible in the client window.
- Confirm the client-owned wizard cannot move during Mug Run or Staffs at Dawn countdown, Trial results, or any non-active Final/Results state.
- Confirm the previous observed working pieces still work: bonk knockback, Arcane Pinball, movement/facing, HUD readability, pickups, Staff Clash, ring-out/respawn, Mega Staff, Final readability, event feed, and rematch/reset.

Authority after this pass:

- Authoritative gameplay remains unchanged. GameMode/server still owns match flow, Trial flow, timers, score, Favor, ready state, pickups, rewards, Arcane Pinball gameplay, Quick Bonk, Staff Clash, ring-out/respawn, Mega Staff, Final Candidate/steal/winner, rematch/reset, actor spawning, and actor cleanup.
- Visual/presentation fix: shared camera and runtime arena/hall actors now replicate for listen-server readability. Clients still do not use these visual actors to author gameplay.

Standalone local preservation:

- Standalone local PIE keeps the same top-down shared camera behavior, local one-human-versus-bot workflow, local couch multiplayer workflow, and current Party Hall -> Mug Run -> Staffs at Dawn -> Grand Wizard Final -> rematch loop.

## 35. Direct-Connect Listen-Server Mode Prep Status

Completed as the twenty-seventh online spike:

- This pass prepared a simple non-Steam direct-connect listen-server smoke path for separate host/client process testing.
- No Steam sessions, lobbies, matchmaking, public browser, friend invites, production online UI, new gameplay, new Trial, new spell, new reward, new powerup, Hammer Time, prediction, rewind, lag compensation, exact cross-client physics work, loose physics replication, gameplay tuning, match-flow redesign, scoring redesign, Final redesign, or rematch redesign was added.
- Direct-connect was prepared at the code/documentation level. Codex did not launch or directly observe separate Unreal host/client processes in this environment.

Session mode separation:

- Added `EWizardPrototypeSessionMode` with four lightweight diagnostics/setup modes: `LocalPrototype`, `LocalWithBots`, `OnlineListenServer`, and `OnlineClient`.
- `AWizardStaffGameMode` detects its authoritative setup mode from `UWorld::GetNetMode()` and local bot settings.
- `LocalPrototype` and `LocalWithBots` are the only modes that allow local prototype helpers such as `EnsureLocalPlayers`, keyboard fallback, and playtest bot fill.
- `OnlineListenServer` is used for listen-server authority worlds, including future `?listen` direct-connect hosting. In this mode, the host does not auto-create extra local prototype players or bots.
- `OnlineClient` is exposed as a client-side observed mode through `AWizardStaffGameState::GetObservedPrototypeSessionMode()`. Remote clients do not have GameMode authority and do not use session mode to drive gameplay.
- `AWizardStaffGameState` mirrors the authoritative prototype session mode for low-noise diagnostics/readability. This mirror is display-only and never drives score, possession, match flow, pickups, rewards, combat, Final state, or reset.

Concrete fixes made:

- `AWizardStaffGameMode` now keeps an explicit `PrototypeSessionMode` value and publishes it to GameState during observable-state sync.
- GameMode logs the detected prototype session mode once, and again only if it changes, to help distinguish standalone, listen-server host, and online client smoke paths.
- The existing local-only gates around `ApplyPlaytestBotPIEDefaults`, `EnsureLocalPlayers`, `IsPlayerIndexPlaytestBot`, `GetDesiredLocalPlayerCountForSession`, and keyboard fallback were left intact and now read through the explicit session-mode detector.
- `AWizardStaffGameState` now exposes both the replicated authoritative session mode and the locally observed mode so remote clients can identify themselves as `OnlineClient` without owning setup authority.

Checks performed where no code change was needed:

- `PostLogin`, `RestartPlayer`, shared-camera assignment, PlayerState display slot/color mirrors, and connected controller counting already support a listen-server host plus one direct client without adding extra local players.
- Runtime Party Hall, prototype arena, Staffs at Dawn arena, Final ritual circle, pickups, Arcane Pinball projectiles, powerups, event-feed messages, and replicated readability actors remain server-spawned/server-owned or visual-only according to the previous scaffolding.
- The current `GetDesiredLocalPlayerCountForSession()` behavior still sizes arrays for at least two players in online mode while avoiding local player creation outside standalone local prototype sessions.
- No new dev console command was added. The direct-connect host/client path uses standard Unreal map URL and `open <address>` commands only.

Direct-connect host steps:

1. Close PIE and any old game process.
2. Start a host game process with the prototype/default map as a listen server.
3. Current fallback command, because this repo has no project `.umap` assets under `Content` and no project `GameDefaultMap` in `Config/DefaultEngine.ini`:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Roger\Documents\Wizard's Staff game\WizardStaff.uproject" /Engine/Maps/Templates/OpenWorld?listen -game -log -windowed -ResX=1280 -ResY=720
```

4. If a real prototype map is later authored or selected, replace `/Engine/Maps/Templates/OpenWorld` with that project map path.
5. Expected host result: one possessed wizard, top-down shared camera, runtime Party Hall/prototype presentation actors, and server-authoritative match flow.

Direct-connect client steps:

1. Start a second game process:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Roger\Documents\Wizard's Staff game\WizardStaff.uproject" -game -log -windowed -ResX=1280 -ResY=720
```

2. In the client game window console, run:

```text
open 127.0.0.1
```

3. If needed, try:

```text
open 127.0.0.1:7777
```

4. Expected client result: exactly one possessed wizard, top-down shared camera, replicated GameState/PlayerState mirrors, runtime Party Hall/arena visibility, and no local-only bot/player setup.

PIE comparison path:

1. Use Number of Players = `2`.
2. Use Net Mode = `Play As Listen Server`.
3. Confirm both windows use the top-down shared camera, see runtime arenas/Final circle, and cannot move during countdown/setup/results input locks.
4. Use this path as the fallback if separate-process direct connect fails before gameplay begins.

What remains authoritative:

- GameMode/server still owns match flow, Trial flow, timers, possession/slot assignment, score, Favor, pickups, rewards, Use Reward, Arcane Pinball gameplay, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup collection, Mega Staff grant/expiration/temp segments, Grand Wizard Final Candidate/steal/winner, event publishing, actor cleanup, and rematch/reset.

What remains visual/readability-only:

- GameState/PlayerState mirrors, the session-mode mirror, shared camera, runtime presentation actors, HUD timers/rows/meters/labels, replicated pickup visibility, carried reward display, Arcane Pinball trails/movement readability, combat readability, Final circle/Candidate/steal readability, Mega Staff readability, and event-feed messages.

Standalone local preservation:

- Standalone local PIE, one-human-versus-bot, local couch multiplayer, local shared camera, keyboard fallback, playtest bot fill, local Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, Mega Staff, Grand Wizard Final, and the current Party Hall -> Mug Run -> Staffs at Dawn -> Grand Wizard Final -> rematch loop are intentionally preserved.

Known limitations:

- Separate-process direct-connect host/client testing was not run by Codex in this pass.
- The project currently has no authored project map asset. Direct-connect docs use the engine open-world template map because the prototype runtime actors can populate it.
- Steam sessions/lobbies, production online UI, disconnect/reconnect UX, host migration, dedicated server work, prediction/rewind/lag compensation, loose snapped segment physics replication, exact cross-client physics fidelity, production notification/chat/analytics systems, new Trials, new spells, new rewards, new powerups, and Hammer Time remain deferred.

## 36. Separate-Process Direct-Connect Host/Client Smoke-Test Status

Completed as the twenty-eighth online spike:

- Separate-process direct-connect host/client testing was actually launched by Codex for this pass.
- Codex could inspect process state and logs, but could not visually control or inspect the game windows. Treat this as a log-level direct-connect startup smoke test, not a full human gameplay/feel validation.
- No Steam sessions, lobbies, matchmaking, public browser, friend invites, lobby UI, production online UI, new gameplay, new Trial, new spell, new reward, new powerup, Hammer Time, prediction, rewind, lag compensation, exact cross-client physics work, loose physics replication, gameplay retuning, match-flow redesign, scoring redesign, Final redesign, or rematch redesign was added.

Exact commands used:

Host:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Roger\Documents\Wizard's Staff game\WizardStaff.uproject" /Engine/Maps/Templates/OpenWorld?listen -game -log -NoLiveCoding -windowed -ResX=1280 -ResY=720 -WinX=40 -WinY=40 -Abslog="C:\Users\Roger\Documents\Wizard's Staff game\Saved\Logs\WizardStaff_DirectConnect_Host_Rerun.log"
```

Client:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Roger\Documents\Wizard's Staff game\WizardStaff.uproject" 127.0.0.1 -game -log -NoLiveCoding -windowed -ResX=1280 -ResY=720 -WinX=1380 -WinY=40 -Abslog="C:\Users\Roger\Documents\Wizard's Staff game\Saved\Logs\WizardStaff_DirectConnect_Client_Rerun.log"
```

What was observed from the first direct-connect launch:

- Host launched as a listen server on `/Engine/Maps/Templates/OpenWorld?listen`.
- Client connected to `127.0.0.1`.
- Host accepted the client connection and logged `Join succeeded`.
- Host logged `Online Listen Server`.
- Client loaded the server map and logged `World NetMode = Client`.
- Client received GameState and PlayerState mirrors for P1 and P2.
- Concrete bug found: the host could auto-start Mug Run before the separate client process finished booting and connecting. In the first run, the client joined after Mug Run had already started.

Fix made:

- `AWizardStaffGameMode` now holds Party Hall/intermission while in `OnlineListenServer` mode until at least two PlayerControllers are connected.
- The hold is limited to the Party Hall/intermission waiting state. It does not affect standalone local mode, local bot/couch workflows, active Trial gameplay, countdown once enough players are present, results, Final, or rematch rules.
- The hold logs once: `WizardStaff listen server holding Party Hall until a second player connects.`
- `AWizardStaffGameState` now has a low-noise `OnRep_ReplicatedPrototypeSessionMode` diagnostic log so a remote client can report `authority=Online Listen Server observed=Online Client`.

What was observed from the rerun after the fix:

- Host started as `Online Listen Server`.
- Runtime prototype arena, Staffs at Dawn arena, and Party Hall fallback actors spawned on the host.
- Host logged that it was holding Party Hall until a second player connected.
- Client connected by direct address.
- Host accepted the connection, assigned `PlayerController_1` to display slot P2, and logged `Join succeeded`.
- Client logged `authority=Online Listen Server observed=Online Client`.
- Client received PlayerState mirrors: `slot=P1 color=0 bot=false` and `slot=P2 color=1 bot=false`.
- Mug Run started after the client joined, not before.
- The path reached direct-connect process launch, connection, map load, GameState/PlayerState mirror replication, session-mode diagnostics, and Mug Run start.

What Codex did not directly observe:

- Actual host/client window visuals.
- Top-down shared camera in the separate-process windows.
- Runtime arena/hall visibility in the separate-process windows.
- Direct input control or human feel.
- Mug collection, Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup, Mega Staff, Grand Wizard Final, event feed display, and rematch/reset through direct human play.

Follow-up human-observed validation:

- A human-observed separate-process direct-connect full-loop smoke test was run after the host/client pair launched.
- Both windows had the expected top-down camera.
- No extra bots or players appeared.
- The client could control only its own wizard.
- Countdown/input lock behavior felt correct.
- Mug Run started after both players were present.
- Mug Run passed in direct-connect.
- Staffs at Dawn passed in direct-connect.
- Grand Wizard Final passed in direct-connect.
- The observed Party Hall -> Mug Run -> Staffs at Dawn -> Grand Wizard Final loop completed without reported direct-connect gameplay bugs or issues.

Checks performed where no code change was needed:

- Direct URL client connection works with `127.0.0.1`.
- GameMode remains server-authoritative.
- Client receives replicated GameState/PlayerState mirrors without GameMode access.
- Session mode distinguishes the listen-server authority and the remote online client.
- Local-only player creation and playtest bot fill did not appear in the direct-connect logs; both mirrored PlayerStates reported `bot=false`.
- The human-observed direct-connect smoke pass did not reveal a possession, camera, local-vs-online gating, input-lock, Party Hall, Mug Run, Staffs at Dawn, or Grand Wizard Final bug after the Party Hall hold fix.

Warnings/blockers found:

- The project still has no authored project `.umap` asset under `Content` and no project `GameDefaultMap`, so direct-connect still uses `/Engine/Maps/Templates/OpenWorld` as a fallback.
- Startup logs include repeated engine warnings/errors about missing VisionOS editor icon files and `FBodyInstance::GetSimplePhysicalMaterial` during native CDO construction. These appeared in both host and client startup logs and were not fixed in this pass because they did not block direct connection and are outside the direct-connect scaffold symptom fixed here.
- Snapped staff segment authority/readability remains the last major gameplay scaffold that has been intentionally deferred. Before moving toward Steam/lobby work or map/default-map polish, the next pass should add the smallest server-owned snapped segment scaffold without attempting full loose physics replication.
- The project still needs a later cleanup to replace the engine OpenWorld fallback with a real project startup map/default-map configuration.

What remains authoritative:

- GameMode/server still owns match flow, Trial flow, timers, possession/slot assignment, score, Favor, pickups, rewards, Use Reward, Arcane Pinball gameplay, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup collection, Mega Staff grant/expiration/temp segments, Grand Wizard Final Candidate/steal/winner, event publishing, actor cleanup, and rematch/reset.

What remains visual/readability-only:

- GameState/PlayerState mirrors, the session-mode mirror, shared camera, runtime presentation actors, HUD timers/rows/meters/labels, replicated pickup visibility, carried reward display, Arcane Pinball trails/movement readability, combat readability, Final circle/Candidate/steal readability, Mega Staff readability, and event-feed messages.

Standalone local preservation:

- Standalone local PIE, one-human-versus-bot, local couch multiplayer, local shared camera, keyboard fallback, playtest bot fill, local Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, Mega Staff, Grand Wizard Final, and the current Party Hall -> Mug Run -> Staffs at Dawn -> Grand Wizard Final -> rematch loop were not intentionally changed.

How to rerun the direct-connect startup smoke test:

1. Build `WizardStaffEditor`.
2. Close old Unreal game/editor processes.
3. Launch the host command above.
4. Launch the client command above.
5. Confirm host logs `Online Listen Server`.
6. Confirm host logs `holding Party Hall until a second player connects` before the client joins.
7. Confirm client logs `World NetMode = Client`.
8. Confirm client logs `authority=Online Listen Server observed=Online Client`.
9. Confirm host logs P2 assignment and `Join succeeded`.
10. Confirm Mug Run starts only after the client joins.

How to rerun the visual/gameplay smoke test manually:

1. Use the same host/client commands above.
2. Confirm both windows show top-down shared camera and runtime presentation actors.
3. Play through Party Hall -> Mug Run -> Staffs at Dawn -> Grand Wizard Final -> Results/rematch.
4. Exercise mugs/rewards, Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, Staffs powerup/Mega Staff, Final Candidate/steal readability, event feed, and rematch/reset.
5. Record any host/client visual, input, authority, stale-state, actor lifecycle, HUD, event-feed, or reset issue with exact phase/window/repro notes.

## 37. Next Implementation Prompt Recommendation

Recommended next Codex prompt for the next online code spike:

```text
Project rules:
- This is Wizard's Staff, currently a locked local multiplayer prototype.
- Continue the online multiplayer spike with server-authoritative snapped staff segment replication/readability scaffolding only.
- Do not add Steam lobbies, matchmaking, public lobby browser, friend invites, a third Trial, new spells, new powerups, Hammer Time, final UI, cosmetics, progression, or gameplay retuning.
- Preserve local one-human-versus-bot and local couch multiplayer workflows.
- Keep the current online scaffolding for GameState/PlayerState, explicit prototype session mode, movement, staff count, Slosh/Stress, pickups, rewards, Use Reward, server-owned Arcane Pinball gameplay projectile, Quick Bonk, ring-out/respawn, Staff Clash, Staffs powerup/Mega Staff, Grand Wizard Final readable/steal state, rematch/reset cleanup, actor lifecycle cleanup, replicated event feed, and direct-connect preparation working.
- Do not add Steam sessions, lobby UI, matchmaking, production UI, prediction, rewind, lag compensation, exact loose physics replication, new Trials, new spells, new rewards, or new powerups.
- Keep changes small and build after changes.

Goal:
Create the smallest safe server-owned snapped staff segment scaffold so staff segment loss/snap decisions are authoritative, replicated staff count/readability stays correct, and clients can see readable snapped-segment results without client-side segment authority or loose physics replication.

Requirements:
1. Server owns all staff segment snap/loss decisions in networked play; clients must not declare snaps, grant/removal counts, score, Favor, knockback, or stress changes.
2. Reuse existing local snap/stress/segment-removal logic where safe, without retuning snap thresholds, stress relief, staff growth, knockback, scoring, or Favor.
3. Preserve standalone local snapping behavior, local one-human-versus-bot, local couch multiplayer, local shared camera, keyboard fallback, and playtest bot fill.
4. Replicate only the smallest readable snapped segment state needed, such as snap sequence, latest snapped owner/slot, segment count after snap, and short-lived visual/readability cues.
5. Keep `ReplicatedStaffSegmentCount` as the primary staff length/readability source and make sure it updates after server-owned snaps, ring-outs, Mega Staff expiration, and resets.
6. Do not replicate loose snapped segment physics, exact cross-client debris motion, prediction, rewind, lag compensation, or final VFX.
7. Clear snapped segment readability safely on Trial transition, ring-out/respawn, Mega Staff cleanup, Grand Wizard Final setup, rematch/reset, full party restart, and wizard destruction/end play.
8. Audit authority boundaries so remote clients cannot force snaps, segment loss, stress relief, staff growth, score, Favor, ring-out attribution, or stale snapped-segment events.
9. Keep direct-connect host/client commands simple and non-Steam.
10. Build after changes and run or document a focused two-client/direct-connect snap-readability smoke test if practical.

After implementation:
- Explain how server-owned snapped segment scaffolding works.
- Explain what snapped segment state replicates and what remains visual-only.
- Explain how staff count/readability updates after snaps.
- Explain how loose physics replication remains intentionally deferred.
- Explain how local snapping behavior was preserved.
- Explain what was tested in PIE or separate-process direct-connect.
- Be explicit about whether direct-connect host/client was actually rerun.
- Explain every concrete snapped segment authority/readability bug fixed.
- Explain what was audited and left unchanged.
- Explain how to rerun the snapped segment smoke test.
- List all files created or modified.
```

Follow-up recommendation after snapped segment scaffolding is stable:

```text
Do the pre-Steam direct-connect readiness/configuration pass: replace the engine OpenWorld fallback with the smallest safe project-owned startup map/default-map configuration, then rerun a quick direct-connect sanity check.
```

Fallback recommendation if direct-connect fails before gameplay begins:

```text
Do a focused session-mode/local-vs-online cleanup pass. Fix only local-player setup, possession, camera, runtime presentation, input-lock, and direct-connect launch/connection issues.
```

Fallback recommendation if event-feed issues appear during direct-connect testing:

```text
Do a focused replicated gameplay event-feed cleanup pass before adding another online gameplay system. Fix only duplicate, stale, spammy, or reset-generation event issues.
```

## 38. Previous First Implementation Prompt

This prompt was completed by the initial replication scaffolding pass and is kept for historical context:

```text
Project rules:
- This is Wizard's Staff, currently a locked local multiplayer prototype.
- Begin the first online multiplayer code spike.
- Do not add Steam lobbies, matchmaking, a third Trial, new spells, new powerups, Hammer Time, final UI, cosmetics, progression, or gameplay retuning.
- Preserve the current local one-human-versus-bot and local multiplayer workflows.
- Keep changes small and build after changes.

Goal:
Create the minimum replication scaffolding needed for a future listen-server online spike without converting all gameplay yet.

Requirements:
1. Add a WizardStaffGameState class and configure the game mode to use it.
2. Add a WizardStaffPlayerState class and configure the game mode to use it.
3. Add replicated observable state for:
   - party match state
   - active Trial state
   - active Trial type
   - remaining timer values or synchronized timer fields
   - active tuning preset
   - per-player display slot/color identity
   - per-player round wins
   - per-player Grand Wizard Favor
   - per-player Staffs at Dawn score
   - Party Hall ready state
   - Final Candidate / winner indices
4. Keep GameMode authoritative and keep existing local logic working.
5. Update HUD reads only where safe so local mode still works and future clients can read GameState/PlayerState instead of GameMode.
6. Do not replicate staff physics, bonk hit detection, Arcane Pinball, Mega Staff Brew, loose snapped segments, or Staff Clash yet.
7. Document any remaining local-only reads after the scaffolding pass.

After implementation:
- Explain what state was moved or mirrored into GameState/PlayerState.
- Explain what still reads from GameMode.
- Explain how local mode was preserved.
- List all files created or modified.
```

## Audit Summary

Wizard's Staff is in a better position for online migration now that GameState/PlayerState mirrors, a minimal listen-server pawn-control scaffold, explicit prototype session-mode separation, replicated staff segment count/readable visuals, replicated Slosh/Stress readability mirrors, server-owned Mug Run pickup active/respawn replication plus pickup end-play cleanup, replicated carried brew reward display, a network-safe Use Reward request seam, server-owned Arcane Pinball gameplay projectile scaffolding plus cleanup hardening, Quick Bonk request/basic hit scaffolding, server-owned ring-out/respawn scaffolding, Staff Clash request/state scaffolding, focused online combat stale-state hardening, Staffs at Dawn powerup pickup active/hidden/respawn replication plus actor end-play cleanup, Mega Staff Brew effect replication scaffolding, a focused Staffs/Mega Staff cleanup audit, Grand Wizard Final Candidate/timer/winner readable-state replication, Grand Wizard Final stealing progress readability scaffolding, a focused Final readable/steal cleanup hardening pass, focused rematch/reset cleanup hardening, replicated gameplay event/message feed scaffolding, a code-level listen-server end-to-end scaffold stability pass, a targeted replicated actor lifecycle/reset cleanup pass, code-level two-client listen-server smoke-test readiness, human-reported PIE presentation/input fixes, and direct-connect listen-server prep exist. Online multiplayer is still not production-ready yet, and separate-process direct-connect validation remains required. GameMode still owns the real local prototype loop and most gameplay remains local/server-authoritative. The next safest spike is a separate-process direct-connect host/client smoke-test and observed bugfix pass, unless direct-connect fails before gameplay and needs a narrower session-mode/local-vs-online cleanup pass. The riskiest systems remain the ones that make the prototype fun: loose segment physics, exact cross-client physics fidelity, production online flow, and eventual broader event/message polish. Those should be handled deliberately after direct-connect host/client possession, shared camera, runtime presentation, input locks, replicated gameplay scaffolding, and reset boundaries are proven outside PIE.
