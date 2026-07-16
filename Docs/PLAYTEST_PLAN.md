# Playtest Plan

**Last Updated:** 2026-07-16

This is a repeatable validation plan, not proof that every item has already passed. Record date, build, mode, players, observations, and untested items after a session.

## Status Labels for Test Notes

- **Passed:** directly observed in this session.
- **Built only:** compiled or inspected, but not observed.
- **Blocked:** test cannot run in the available environment.
- **Regression:** behavior differs from established expectation.

## 1. Local Normal-loop Regression

1. Launch standalone PIE on `/Game/Maps/WizardStaff_Prototype`.
2. Confirm top-down shared camera, local player setup, keyboard fallback where applicable, and local bot fill where configured.
3. Play Party Hall -> Mug Run -> Results -> Party Hall -> Staffs at Dawn -> Results -> Party Hall -> Cauldron Catastrophe -> Results -> Party Hall -> Grand Wizard Final -> rematch.
4. After every Results phase, verify every wizard is physically respawned in Party Hall, the hall sign names the coming Trial (or `Grand Wizard Final`), and the countdown waits there before moving players to the next phase. Verify the Mug arena is visible/collidable before players leave Party Hall, players are placed and camera-snapped exactly once for the Mug countdown, Cauldron does not repeat after its Final sign, no unrelated arena is swept through, and Cauldron exit shows neither a vertical fall nor empty space while entering Party Hall. The old Cauldron arena may remain off-camera during intermission and must disappear when the next Trial/Final begins. For the Final, confirm the reused prototype arena floor and walls are visible before the ritual circle and players appear; the ritual circle must never appear alone over empty space.
5. During the loop, verify Mug Run pickup/respawn, staff growth, Slosh, reward use, Arcane Pinball, Quick Bonk, Staff Clash, ring-out/respawn, powerup/Mega Staff, Candidate/steal/winner, and reset.
6. Verify local snapping still creates loose physics behavior and no stale state remains after rematch.
7. Repeat the first Party Hall -> Mug Run handoff and one full rematch while watching wizard position and camera framing. Confirm each destination produces one authoritative placement/camera snap, with no duplicate teleport, empty-arena flash, or changed staff/Slosh reset behavior.

## 2. Current Cauldron Regression

Use standalone PIE first. Cauldron remains development-only unless explicitly promoted.

1. Start Cauldron through the existing dev/safe flow.
2. Confirm the open octagonal floor exists before player control unlocks; players must not fall during staging.
3. Confirm the arena is visibly separated from Party Hall, Mug Run, and Staffs at Dawn.
4. Collect multiple vials and verify only the current top-stack effect is active.
5. Test banking initiation from its intended access point with a short and a very long staff. Confirm a confirmed bonk starts banking reliably from reasonable nearby angles/ranges, while passive proximity, wrong-side attempts, and leaving the 360-unit hold range do not bank.
6. Deposit enough Speed and Burdening Power vials to observe the server-owned 25% hazard roll. Confirm Speed only creates slippery puddles, Burdening Power only creates sticky sludge, no timed random hazards appear, and sticky remains mild/readable with broom-boost escape.
7. Walk through a slippery puddle at low and high Mana Slosh. Confirm its entry shove follows travel direction, high Slosh skids farther, low friction carries out for about 1.5 seconds after leaving, and Trial cleanup restores normal movement.
8. Use `DebugAssignCauldronCurse 0`, then have the cursed wizard Quick Bonk the active gold intake. Confirm the orb clears, the cauldron boils violently, eight hollow red landing circles appear before eight staggered arcs, and each blast affects only its marked radius at a noticeably weaker-than-normal curse force. Confirm it awards no score, removes no vial, and cannot be triggered by passive contact or an inactive intake.
9. Exercise cursed-orb attachment, transfer/Clash behavior, snap/ground behavior, detonation, cleanup, and end-of-match cleanup.
10. Ring out a player and verify respawn/camera behavior without camera pull beneath the floor.
11. Confirm score-driven staff growth follows current tuning.
12. End/rematch and verify hazards, vials, curses, curse bombs, deposit arcs, and readout state are gone.

## 3. Two-client Listen-server / Direct-connect Regression

**Established baseline:** separate-process direct-connect and 2026-07-15 two-player listen-server PIE full-loop sessions have been human-observed as passing, including transition reset/presentation/staging, dirty-checked mirrors, and stable PlayerState slot-first identity. Re-run after changes to shared systems.

1. Host `/Game/Maps/WizardStaff_Prototype?listen`.
2. Start a second process; client uses `open 127.0.0.1`.
3. Confirm each process has exactly one possessed wizard, correct top-down camera, stable display slot, and no extra local bot/player.
4. Confirm countdown/input lock prevents premature move, jump, boost, bonk, and reward use.
5. Run one pass through Mug Run, Staffs at Dawn, Cauldron Catastrophe, and Final. Exercise one example of each replicated system where practical.
6. In both windows, confirm the shared prototype arena is hidden during Party Hall/Staffs/Cauldron, visible before Mug staging, and restored before Final presentation and placement.
7. Snap a staff segment and confirm authoritative staff count/readability updates in both windows. Loose snapped physics debris is intentionally standalone-only and is not expected online.
8. Judge movement/facing smoothness only while the relevant PIE viewport is focused. Unreal may throttle an unfocused embedded viewport to save resources; background-only choppiness is an editor artifact, not evidence of a gameplay replication defect.
9. Rematch; verify no stale pickups, projectiles, powerups, Mega Staff, combat, respawn, Final, HUD, or event-feed state.
10. In both windows, watch each countdown and rematch handoff for duplicate pawn placement or a second camera snap. The server should stage each wizard once after the destination presentation/floor is ready.
11. Throughout the loop, compare host and client HUD mirrors: player slot/color, ready state, staff/current Trial score, Favor/wins, all countdown/result/intermission timers, Cauldron curse/banking values, Final Candidate/steal/winner state, and event-feed clearing. Values should remain current and readable; no gameplay may depend on the client mirrors.
12. For the slot-identity regression, record which window owns P1 and P2. Confirm those slots and colors remain attached to the same PlayerStates through Trial transitions, ring-out/respawn, and rematch; verify spawn sides, score/Favor awards, Arcane Pinball/event labels, and HUD rows never swap because controller iteration order changes. Reconnect/rejoin is not expected to restore a departed identity in this pass.

## 4. Steam Host and Join Validation

**Current status:** Host session creation was observed. Valid client search/join/travel needs two machines and two Steam accounts; same-machine multi-process testing previously hit Steam API limitations.

1. On host machine/account, run a private Steam-capable development build and use `WizardSteamHost`.
2. Confirm logs show Steam subsystem, session creation, project-map listen travel, `OnlineListenServer`, only P1, and Party Hall wait state.
3. On a second machine/account, run the same compatible private build and use `WizardSteamJoinFirstSession`.
4. Confirm compatible metadata search, join success, resolved connect string, client travel, `OnlineClient`, host accepted connection, P2 assignment, and Mug Run start.
5. Run only a small gameplay sanity path first: one host mug and one client mug. Preserve logs.

Do not claim Steam join is passed until this environment exists and both sides are observed.

## 5. Private Steam Build and Leaderboard Validation

1. Install the current `private_test` build through Steam as an authorized user.
2. Confirm startup map, runtime lighting, and normal local loop.
3. Complete a Final in a Steam-hosted session, not direct-connect/local.
4. Record logs for leaderboard queue and flush completion.
5. Verify the configured `WizardStaff_BestGrandWizardFavor` leaderboard in Steamworks after sufficient propagation time.
6. Repeat one lower-score result to verify KeepBest behavior rather than assuming overwrite behavior.

**Current status:** BuildID `24238419` on `private_test` passed a Steam-installed human full-loop playthrough on 2026-07-16, including the in-game Party Hall standings board. Steamworks leaderboard submission/read-back remains unverified.

## Reporting Template

```text
Date/build:
Mode and players:
Passed:
Regression(s):
Logs/screenshots:
Not tested / blocked:
Follow-up owner and priority:
```
