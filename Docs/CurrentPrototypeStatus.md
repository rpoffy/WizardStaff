# Wizard's Staff Current Prototype Status

Last updated: July 2, 2026

Scope: this document describes the current local multiplayer prototype as it exists in code. It is not a final design spec. The game is still placeholder-heavy, code-first, and built for fast playtesting.

## High-Level State

Wizard's Staff is currently a playable local party-game prototype with a small round structure, two Trials, a Final Round, and a rematch loop. The current target flow is implemented:

1. Party Hall intermission.
2. Mug Run Trial.
3. Trial Results.
4. Party Hall intermission.
5. Staffs at Dawn Trial.
6. Trial Results.
7. Grand Wizard Final Round.
8. Final winner.
9. Automatic rematch back to Party Hall.

The core physical comedy loop is also in place:

1. Two to four local wizards spawn in a simple authored or runtime fallback arena.
2. Players move, hop, collect mugs when the Trial uses them, and bonk each other.
3. Drinking a mug adds one visible segment to the wizard's staff, which also adds Mana Slosh and can grant a one-use brew reward in Mug Run.
4. Longer staffs are more powerful, heavier, harder to swing, easier to wedge on props, and more fragile.
5. Staffs collide with walls and arena props using a simple hybrid collision setup.
6. Staff bonks and wall impacts build Staff Stress.
7. When stress reaches the threshold, the top staff segment snaps off into a loose physics object and vents some Mana Slosh.
8. Loose snapped segments are intentionally chaotic but are protected by cleanup and out-of-arena respawn failsafes.

The prototype intentionally does not include online multiplayer, final art, a full spell system, menus, progression, shops, cosmetics, or a polished UI. The focus is the physical comedy loop: grow a staff, become powerful, become awkward, bonk friends, get wedged, snap pieces off, recover, and keep the local party session moving.

## Completed Game Target

The long-term content target for the completed version of Wizard's Staff is a roster of 12 total mini-games/Trials built around the same physical comedy identity: awkward staffs, Slosh, bonking, snapping, ring-outs, loose segment chaos, and short readable party-game objectives.

The intended default completed-match structure is three Trials, then the Grand Wizard Final, then winner/rematch. The current vertical slice uses two Trials plus the Grand Wizard Final and takes roughly 3:30 for a full loop. The completed game should target about 10 minutes for a full default party session. That pacing target matters: future Trials should be designed and tuned around a compact party-session rhythm rather than simply adding another long round each time.

The 12-Trial goal is a full roster/content target, not a promise that every match plays all 12 Trials. A possible Steam Early Access target is at least 6 total polished Trials before publishing to Early Access, if Early Access becomes the release path. Do not solve final Trial selection rules yet; preserve this direction so future content decisions do not accidentally stretch the game beyond the intended couch-party length.

## Important Entry Points

- `WizardStaff.uproject` is the Unreal project file.
- `Source/WizardStaff/Public` contains the gameplay class declarations and Blueprint-exposed tuning structs.
- `Source/WizardStaff/Private` contains the gameplay implementation.
- `Config/DefaultEngine.ini` sets `AWizardStaffGameMode` as the global default game mode, disables splitscreen with `bUseSplitscreen=False`, and offsets gamepad assignment so keyboard player 1 plus one gamepad can become player 1/player 2.
- `Config/DefaultInput.ini` contains the current action and axis bindings.
- `Docs/PlayerGuide.md` is the player-facing guide for the current prototype loop, goals, mechanics, and normal controls.
- `Docs/PrototypeArenaWorkflow.md` explains the editable arena actor workflow.
- `Docs/Phase1PolishBacklog.md` is the current Phase 1 polish and tester-prep backlog for the locked vertical-slice baseline.
- `Docs/PrototypeAudit.md` is an earlier audit note. This file is the more current status overview.

## Current Playable Loop

The game starts directly into the prototype loop through `AWizardStaffGameMode`. On BeginPlay, the game mode looks for a placed `AWizardStaffPrototypeArena`, falls back to a spawned prototype arena if needed, ensures the desired number of local players exists, creates a shared camera if enabled, spawns or finds the Party Hall, and starts the party match flow.

The default session begins in the Party Hall. Players can run around, bonk lightly, read standings, see the active tuning preset, and wait for the next Trial countdown. A physical Ready Bell now sits in the Party Hall: bonking it marks that player ready for the next Trial. If all active players ring the bell, the remaining intermission is shortened to a small ready countdown; the normal intermission timer remains the fallback. The first Trial is Mug Run. Mugs spawn around the Mug Run/prototype arena, and drinking a mug adds one staff segment, adds Mana Slosh through that staff growth, and can grant a one-use brew reward. Staff segment count is the Mug Run score.

After Mug Run Results, players return to the Party Hall. The second Trial is Staffs at Dawn, a pure bonk-combat Trial with mugs disabled for now. Staffs at Dawn now uses a purpose-built placeholder combat arena when one is available, with safer fallback to the Mug Run/prototype arena. Players score by landing staff bonks and by causing recent bonk victims to respawn out of the arena. After Staffs at Dawn Results, the match enters the Grand Wizard Final Round.

Physical staff segments are intentionally volatile. They can snap off, launch players, and disappear during the chaos, especially in Staffs at Dawn and Arcane Catastrophe. To keep earlier Trials meaningful, players also earn persistent Grand Wizard Favor during the party match. Favor resets only on full rematch.

Normal Trials now start with fresh physical staff segments by default. This keeps Mug Run and Staffs at Dawn from accidentally inheriting a weird snapped or giant staff state from the previous round. The Grand Wizard Final is the exception: it deliberately rebuilds staffs from Final Round advantage tuning, and the starting Candidate receives the configured max Candidate staff.

The Final Round crowns a Grand Wizard Candidate based on Grand Wizard Favor first, then round wins, then current staff segment count. The current Candidate controls the title while inside or near the center ritual circle. Challengers can bonk the Candidate out of the circle, hold the circle, and steal the title. Whoever is Candidate when the final timer expires wins the whole match. The final winner message is shown, playtest telemetry summary remains visible, and the game automatically rematches back to Party Hall.

## Local Playtest Notes

Recent playtest: five local Mug Run rounds with two players.

Result: the game was fun. The core loop of collecting mugs, growing awkward staffs, bonking, getting wedged, snapping segments, and recovering into another round is already producing the intended couch-party energy.

Recent input feel note: keyboard and gamepad sensitivity were increased after local playtesting because maneuvering and turning felt too slow. The adjustment improved general readability and made it easier to recover, aim bonks, and steer around doors/props without removing the comedy from Slosh, Staff Heft, and staff obstruction.

Latest pacing playtest note: the game still has fun bones, Favor readability worked, and the Grand Wizard Final felt great because the Candidate felt like a real grand opponent. Full rounds felt too long at roughly 130 seconds of active Trial time, so the next playtest target is 60-second Mug Run and 60-second Staffs at Dawn Trials. The Final Round remains a snappy 60-second default.

Latest family/local playtest note: a few full local party cycles with a younger player were a lot of fun. The two-Trial party structure is working as a family couch-play prototype, and Arcane Pinball feels good as the first proven brew reward spell.

Latest Staffs at Dawn playtest note: repeated local loops were fun, but staff growth in Staffs at Dawn felt too difficult because hitting players and whiffing bonks generated too much Staff Stress. Bonk hit and whiff stress multipliers were lowered so combat can still threaten snapping without immediately erasing the staff-segment rewards from fighting. Staff snapping should still threaten greedy play and reckless bonking, but it should not erase combat staff-growth rewards so quickly that players feel like growth is pointless.

Latest Staffs at Dawn arena note: after a focused arena tuning pass, Staffs at Dawn is starting to feel pretty good with its own purpose-built placeholder combat arena. The raised island, exposed edges, wider bridge lanes, larger duel pads, safer central spawns, and bounce surfaces give the Trial more identity than reusing the Mug Run/prototype arena. The next playtest should watch ring-out frequency, whether players can recover from a normal bonk, whether risky bridge/pad fights produce exciting ring-outs, camera readability with tall staffs, and whether players can build staff segments through combat before snapping them away.

Latest Staffs at Dawn scoring note: scoring and combat staff-growth defaults were retuned for the larger arena. Landed bonks still matter at `+1`, credited ring-outs are now more valuable at `+3`, the ring-out credit window is slightly more forgiving, every two landed bonks grants a staff segment, and credited ring-outs grant two staff segments. Bonk stress, stress decay, and snap thresholds were left alone for this pass so snapping still threatens reckless play without immediately erasing every combat reward.

Latest Staffs at Dawn movement note: the updated Staffs at Dawn arena and scoring changes are feeling fun, and the one-use airborne broom boost adds a clutch recovery option during ring-out situations. The boost should feel like a risky last-second save, not full flight. Ring-outs should still matter, and clean, well-earned ring-outs should not be erased too often. The first tuning passes shortened the boost, stopped it from instantly overriding strong horizontal knockback, added Slosh/Staff Heft control penalties, and added a short landing cooldown. After playtesting, the high-Slosh disorder behavior was softened because players are often fully sloshed and the old random launch behavior made broom boost frustrating too often. Mana Slosh still makes broom boost less reliable, but disorder now starts later, happens only occasionally, and uses smaller yaw/side/vertical impulses. Staff Heft still makes giant-staff recoveries harder. Broom boost tuning should keep focusing on duration, forward speed, upward safety, Slosh interaction, Staff Heft interaction, and whether other players can clearly read that a recovery attempt is happening.

Latest Staffs at Dawn ring-out integrity note: after broom boost was added, lightweight telemetry now tracks Staffs at Dawn broom boosts used, broom boost ring-out saves, and broom boosts that still resulted in a ring-out. This is meant to answer whether the boost creates occasional clutch recoveries or erases too many clean edge bonks. Ring-out score, ring-out staff growth, respawn delay, and arena layout were not changed in this pass.

Latest 10-15 minute local loop validation note: the current two-Trial loop is still the right short-session target and should keep being tuned before any larger content expansion. The loop of Party Hall, Mug Run, Staffs at Dawn, Grand Wizard Final, match summary, and automatic rematch is fun enough to keep iterating in repeated short sessions. The main new friction was Mega Staff Brew visibility: in one Staffs at Dawn match the powerup did not appear at all, so the pickup was not contributing enough to the 60-second round. Spawn tuning has been adjusted to make Mega Staff Brew more visible with two spawn slots, a shorter initial delay, shorter respawn timing, and marker rerolling. The next validation pass should confirm that Mega Staff Brew appears often enough to matter, still feels powerful/risky/funny instead of dominant, and keeps Staffs at Dawn feeling like a staff-combat Trial rather than a powerup race. Marker rerolling should improve variety by preventing the pickup from feeling glued to one arena spot, but this still needs playtest confirmation. HUD/debug messages did not have a newly reported spam problem, but Mega Staff pickup/expire/scoring/Favor messages should keep being watched because the powerup now appears more often. No stale state has been newly reported after automatic rematch, but repeated short sessions should continue checking carried rewards, temporary Mega Staff state, broom tracking, Favor, Trial scores, and loose segment cleanup.

Latest HUD/readability playtest note: the Canvas HUD readability pass worked well. `Playtest` mode is now much more usable for normal PIE sessions, and debug messages are no longer taking over the screen. The small HUD message feed is easier to read than unrestricted engine debug messages because routine gameplay events are grouped in one predictable area without blocking the game. HUD display modes are working as intended: `Playtest` should remain the default for normal local playtesting, while `FullDebug` should be used only when tuning or diagnosing systems. The next validation pass can focus more directly on the game loop now that visibility is improved: Mug Run readability, Staffs at Dawn readability, Mega Staff Brew visibility, broom recovery readability, Final Round clarity, and match summary readability. Keep watching whether the message feed becomes spammy now that Staffs at Dawn powerups appear more often.

Latest player readability marker note: each wizard now uses a single flat circle-with-point ground marker. The point is integrated into the marker shape, rotates with the character, and shows the wizard's facing direction without using a separate arrow mesh. This is the preferred placeholder readability solution for the current slice because it is cheap, clear from the shared camera, and avoids cluttering the character with extra directional geometry.

Latest Party Hall readability note: the intermission standings display has been moved off the camera-side wall and onto a larger far/top-wall board from the shared camera's perspective. The board and text are tilted upward toward the top-down camera so they read better during play. Standings now use compact columns for ready state, wins, Favor, and staff count. Countdown and leader signs remain as smaller supporting plaques around the standings display, while the active preset and Ready Bell text are flat floor labels with thin non-colliding backing plates. This keeps the Party Hall playable while making the main intermission information easier to read without opening a menu.

Latest broom recovery forgiveness note: after increasing broom boost vertical forgiveness, the change feels great. Broom boost now gives players a real clutch save chance instead of requiring an instant reaction, but it still does not feel like free flight or guaranteed recovery. Being Mana Sloshed makes broom recovery harder, which supports the core power-versus-control tradeoff: greedy staff growth gives players reach and power, but makes late recovery less reliable. Do not keep tuning broom boost unless repeated playtests show it is too strong or too weak. Future tuning should protect this feel: saveable near-miss ring-outs, difficult recovery under Slosh, clean strong ring-outs still mattering, and giant-staff players remaining harder to save.

Latest one-human-versus-bot playtest note: the playtest bot now feels great for solo loop validation and should be treated as a valid normal iteration path when a second human is not available. It creates enough pressure to validate Mug Run, Staffs at Dawn, the Grand Wizard Final, rematch cleanup, and overall loop readability. The bot is challenging enough to expose systems, but still makes goofy mistakes, which is desirable because awkward bot behavior supports Wizard's Staff instead of turning the test into a serious AI duel. The Grand Wizard Final feels amazing with the bot. After shrinking the visible ritual circle, reducing Candidate safety padding, and slightly shortening the steal hold, the Final feels more intense: the Candidate becomes vulnerable sooner after a good disruption, and challengers have a better chance to steal without needing a perfect launch plus perfect timing.

Latest Grand Wizard Final pressure/readability note: the improved Final pressure values were preserved. The focused pass did not add mechanics or change the Candidate/circle/steal rules. HUD wording was tightened so players can more quickly read `SAFE`, `VULNERABLE`, and active `Stealing` states without extra explanation. The next Final playtest should watch whether the smaller safety zone makes steals exciting without becoming too swingy, and whether the steal bar is readable during bot and human Candidate rounds.

Latest preset validation note: each preset has now been played through a few times: `Stable`, `Chaotic`, and `Arcane Catastrophe`. All three currently feel great. The earlier `Stable` tuning pass fixed the weak-bonk/flat-tie problem enough that Stable no longer needs to be called out as unvalidated. `Chaotic` remains the safer default preset for normal iteration. `Arcane Catastrophe` is still the favorite high-energy chaos preset because it amplifies the slapstick without breaking the loop. The bot remains useful enough for full-match solo validation across Mug Run, Staffs at Dawn, the Grand Wizard Final, match summary, rematch cleanup, and general loop readability.

Latest Grand Wizard Final Slosh note: Mana Slosh now locks at Final Round start by default after Favor-based staff setup has rebuilt each player's starting staff. This keeps the Grand Wizard Candidate fully compromised by their huge earned staff while defending the ritual circle, and it keeps challengers at their Final-start Slosh level instead of letting everyone passively sober up during the Final timer. Broken staff segments still vent locked Slosh, so snapping pieces off the Candidate's huge staff can reduce their Slosh burden and keep the starting advantage in check. This is an intentional prototype rule for the current Final and can be revisited later if the Final is reworked.

Latest Mana Slosh bonk note: Mana Slosh now contributes modestly to quick bonk knockback before staff segment scaling is added. The intent is simple: a drunken wizard hits harder, but not wildly harder. `KnockbackPerManaSlosh` defaults to `1.0`, so 50 Slosh adds roughly 50 knockback and full Slosh adds roughly 100 before the normal bonk max cap. This gives Slosh more gameplay presence without replacing staff size, Staff Heft, stress, or snapping as the main power-versus-control tradeoffs.

Latest Staff Clash tuning note: Staff Clash already feels fun because it formalizes the same-time, same-angle bonk moments that naturally happen during play. The active clash duration was reduced from `1.5` seconds to `1.0` second so the duel reads as a quick slapstick beat instead of freezing the fight too long. `ClashDuration` is only how long an already-started clash lasts; it is not a cooldown, timer, or trigger frequency. Staff Clash should remain occasional, funny, and readable, not a replacement for normal staff bonking.

Latest Mana Slosh tuning note: staff snapping now acts as a small Slosh release valve. Stable was recently adjusted because it was still too hard to become meaningfully sloshed when actively chasing mugs or combat staff growth: Stable now gains `27` Mana Slosh per staff segment, decays at `4` Slosh per second, and snapped segments vent only `4` Slosh. Chaotic and Arcane Catastrophe were left unchanged because they already feel good. Current snap relief defaults are `4` Slosh removed in `Stable`, `11` in `Chaotic`, and `19` in `Arcane Catastrophe`. This gives fully sloshed players a comeback path without removing the core power-versus-control tradeoff: staff growth still makes wizards stronger and more compromised, but snapping can help them climb back from total slosh.

Latest vertical-slice stability pass note: a code-level lifecycle audit and `WizardStaffEditor Win64 Development` build passed after the main stale-state fixes. Mug Run winner/Favor, Final Candidate selection, Party Hall standings, leader highlights, and Canvas HUD player rows now consistently use the game mode's local-player index helper instead of relying directly on `PlayerState` IDs, which is safer for one-human-versus-bot PIE sessions. Active Arcane Pinball projectiles are now cleaned up when Mug Run ends, when Staffs at Dawn starts, when the Grand Wizard Final starts, and when a full party match starts, so already-fired Mug Run spells should not leak into later states. A headless editor validation run on `Stable` completed three full party cycles in one process: Mug Run, Staffs at Dawn, Grand Wizard Final winner, automatic rematch, then repeat. A separate headless `Chaotic` run completed one full cycle and rematched. This is strong loop evidence, but it is not the same as a human-driven interactive PIE pass across all three presets in one run. That literal gate still needs confirmation, especially for HUD readability and feel. The pass also found warning noise from runtime staff component name reuse and loose segment physics cleanup order; component naming was rebuilt successfully, and the loose segment cleanup-order patch should be verified on the next build/run. One `FRotator` NaN warning appeared during heavy Chaotic play and should be watched if it repeats.

Latest loop lock-in note: the current gameplay loop now feels great on `Stable`, `Chaotic`, and `Arcane Catastrophe`. The odd flow issues observed in recent playtests have been resolved, including the mini-game false-start feel during countdown and the Staffs at Dawn case where ringed-out players could keep moving around on the lower floor and delay respawn. Countdown now stages players at the Trial spawns and locks prototype input until the Trial becomes active. Staffs at Dawn now uses a tighter lower-floor ring-out line so falling beneath the raised arena resolves promptly while still allowing real broom recovery attempts. The current two-Trial loop can be treated as locked in for this phase: future work should protect it, keep regression-testing rematches, and avoid expanding content until the next phase is deliberately chosen.

What worked:

- The tuning presets made it quick to compare feel profiles without stopping play.
- `Arcane Catastrophe` was the favorite preset because it amplified the slapstick without needing extra rules or content.
- Loose snapped staff segments created memorable physical comedy.
- The out-of-arena respawn and loose segment cleanup systems let that chaos remain funny without completely breaking the session.
- The current Mug Run objective is simple enough that the physical comedy stays central.
- Arcane Pinball feels good as the first brew reward spell because it adds readable magical chaos without replacing staff bonking.
- Grand Wizard Favor was obvious enough to understand as the persistent match-performance score.
- The Grand Wizard Final felt strong because the starting Candidate had a readable earned advantage.
- One-human-versus-bot playtesting is now useful enough to validate the full loop when a second human is not available.
- The Grand Wizard Final ring/safe-zone adjustment improved intensity by giving challengers a clearer steal chance after a good disruption.
- Full one-human-versus-bot matches across `Stable`, `Chaotic`, and `Arcane Catastrophe` all stayed fun, which is a strong sign that Mug Run, Staffs at Dawn, and the Grand Wizard Final can work as a short party session.
- A headless editor stability run completed three consecutive `Stable` party cycles without a fatal crash, stale Final state, or rematch failure.
- Latest local validation reports that the current gameplay loop feels great on all three presets.
- The recent countdown false-start and Staffs at Dawn lower-floor delayed-respawn issues have been resolved.
- Staffs at Dawn now feels more distinct with its own arena instead of borrowing the Mug Run/prototype layout.
- Broom boost adds a strong clutch recovery beat for Staffs at Dawn ring-out fights.
- Broom boost vertical forgiveness now feels much better: near-miss ring-outs are saveable, but recovery is still difficult.
- Broom boost under high Mana Slosh is now less frustrating: it can still go wrong, but the disorder is softer and not guaranteed every time.
- Staff snapping now vents some Mana Slosh, giving fully sloshed players a comeback path without removing the staff-growth tradeoff.
- Mega Staff Brew is a promising Staffs at Dawn chaos beat, but it needs to appear reliably enough during 60-second rounds to be evaluated.
- The HUD readability pass made normal PIE sessions much clearer. `Playtest` mode and the controlled message feed are easier to read than scattered engine debug messages.
- The new flat circle-with-point player ground marker makes facing direction readable without adding a separate arrow mesh.

What should be protected:

- Loose snapped segment chaos should remain part of the game's identity.
- Staffs should keep feeling awkward, physical, and capable of getting caught.
- Snapping should stay readable and useful as a release valve when a staff becomes too large or wedged.
- Snapping should also remain a small Mana Slosh release valve so being fully sloshed has a comeback path.
- Staff snapping should threaten greedy play without making earned staff growth feel pointless.
- Arcane Pinball should remain the first proven brew reward spell and should be tuned before another spell is added.
- Playtest bots should remain prototype playtest helpers, not final single-player AI. Their goofy mistakes are part of why they are useful for this project right now.
- Ring-outs should remain meaningful even with broom boost recovery in the game.
- Failsafes should protect the session without sanding away the comedy.
- `Arcane Catastrophe` should remain available as a named chaos preset even if it is not the default.
- `Arcane Catastrophe` should be protected as the favorite high-energy chaos preset as long as it keeps feeling funny and playable rather than broken.

What should be tuned carefully:

- Snap impulse and bonk knockback should be tuned around comedy plus recovery, not pure power.
- Slosh should make players laugh and overcorrect, but should not make normal steering or broom recovery feel hopeless.
- Out-of-arena respawn bounds and delay should catch extreme launches while still allowing big moments to breathe.
- Loose segment lifetime and max count should prevent long-session clutter without removing the initial physics surprise.
- Staffs at Dawn bonk stress should be watched closely so combat staff-growth rewards have time to matter before snapping.
- Staffs at Dawn scoring should keep ring-outs exciting without making normal bonk score feel irrelevant.
- Staffs at Dawn arena tuning should focus on ring-out frequency, bridge widths, duel pad sizes, spawn safety, camera readability, and whether players can keep earned combat staff growth long enough for it to matter.
- Broom boost should mostly be left alone unless repeated playtests show it is too strong or too weak. If tuning resumes, protect the current feel: saveable near-miss ring-outs, recoverable but sketchy control under Slosh, clean strong ring-outs still matter, and giant-staff players remain harder to save.
- Mega Staff Brew spawn frequency and marker variety should be tuned so the pickup matters in short rounds without becoming the whole Trial.
- Future Grand Wizard Final tuning should focus on circle size, Candidate safety padding, steal hold duration, and readability before adding new Final mechanics.
- HUD and message-feed feedback should be watched for spam now that Staffs at Dawn powerups appear more often. Routine gameplay feedback should stay in the feed, while `FullDebug` remains the place for tuning and diagnostic noise.
- `Stable`, `Chaotic`, and `Arcane Catastrophe` all currently feel fun. `Chaotic` can remain the safer default for general iteration, while `Arcane Catastrophe` should be preserved for high-energy playtests and stress testing.

That milestone has now been reached at prototype level. The current party-game wrapper supports Party Hall intermissions, Mug Run, Staffs at Dawn, Results, Grand Wizard Final Round, final winner, rematch, tuning presets, local playtest bots, and local playtest telemetry.

Next recommended milestone: treat the current two-Trial loop as the locked vertical-slice baseline for this phase. Mug Run, Staffs at Dawn, Party Hall readability, scoring clarity, Arcane Pinball, Mega Staff Brew, Staff Clash, broom boost, and Final Round feel should keep being protected as one complete session. One-human-versus-bot sessions are useful for frequent iteration, but real local multiplayer should continue whenever available because human timing, spite, teamwork, and bad decisions are still the real target. The next phase should focus on small polish, bug fixing, repeated rematch stability, and preparing for real local multiplayer feedback before adding a third Trial. Use `Docs/Phase1PolishBacklog.md` as the working checklist for this phase. Staffs at Dawn has a minimal powerup pickup framework using the arena's `FuturePowerupSpawn_*` markers, and `Mega Staff Brew` is the first real Staffs at Dawn powerup. It should be tuned and protected before adding Hammer Time, another powerup, a third Trial, or a broader spell/powerup roster.

## Next Trial Gate

Do not add a third Trial until the current two-Trial party session remains stable in repeated local playtests. The current loop can now be treated as the locked vertical-slice baseline for this phase, but the gate should stay in the document as a regression checklist before expanding the roster.

Appears satisfied based on latest playtests:

- Mug Run is fun in `Stable`, `Chaotic`, and `Arcane Catastrophe`.
- Staffs at Dawn is fun in `Stable`, `Chaotic`, and `Arcane Catastrophe`.
- The current two-Trial session has completed `Stable` validation; Stable no longer needs to be called out as unvalidated.
- The two-Trial session has been playtested with Mug Run brew rewards and Arcane Pinball enabled.
- Arcane Pinball adds readable magical chaos without overshadowing staff bonking, staff growth, staff snapping, or Grand Wizard Favor readability.
- The Grand Wizard Final is readable and exciting.
- Players understand Trial results, round wins, Final Candidate, and final winner well enough for the current Canvas HUD phase.
- Input sensitivity feels acceptable on keyboard and gamepad.
- Broom boost gives players a fair clutch recovery chance without turning Staffs at Dawn ring-outs into unreliable outcomes.
- A headless editor run completed three full `Stable` party cycles in one process and automatically rematched each time.
- High Mana Slosh still makes broom boost sketchy, but it no longer feels hopeless or frustrating.
- Staff snapping works as both a punishment and a small Mana Slosh release valve.
- Loose segment chaos remains funny without cluttering the arena or breaking the session.
- The telemetry summary gives useful information after each match.
- Latest reported playtesting says all three presets feel great and the observed odd flow issues have been resolved.

Ongoing regression checks before expanding content:

- Continue completing full party sessions without restarting PIE.
- Cover `Stable`, `Chaotic`, and `Arcane Catastrophe` during regression passes, either by cycling presets between rematches or by doing separate back-to-back sessions if preset switching mid-run is not practical.
- Watch the single historical Chaotic `FRotator` NaN warning; if it repeats, trace the visual/rotation source before expanding content.
- Keep checking for stale state, restart issues, stuck rematch state, leaked rewards, leaked powerups, broken bot state, unreadable HUD/message spam, countdown false starts, and delayed Staffs at Dawn ring-out respawns.

Until the next phase is deliberately chosen, the best use of development time is small polish, bug fixing, repeated rematch stability checks, controller/readability tuning, arena touch-ups, telemetry clarity, and preparing for more real local multiplayer feedback.

## Party Match And Trial Flow

`AWizardStaffGameMode` owns the outer party match flow, inner trial flow, implemented Trials, and prototype Final Round.

Party match states:

- `PartyHall`: playable tavern-room intermission where players can move, bonk lightly, and read standings.
- `Intermission`: compatibility state for lightweight between-trial logic.
- `Trial`: a trial is counting down or active.
- `Results`: a trial has ended and winner/results text is visible.
- `FinalRound`: Grand Wizard Final Round reached after the configured trial count.

Trial states:

- `WaitingToStart`: short internal reset state used while preparing a new match.
- `Countdown`: players are reset, staged at the active Trial spawns, and briefly input-locked so the arena does not have a false playable start before the timer begins.
- `Active`: the trial timer is active and the active Trial owns its scoring rules.
- `Results`: timer is stopped, Trial-specific actors are inactive, and the winner message is shown.
- `Finished`: the trial has completed and the framework is deciding whether to start another trial or enter Final Round.

Implemented Trial types:

- `MugRun`: collect mugs, grow the tallest staff, and win by staff segment count.
- `StaffsAtDawn`: bonk-combat Trial, currently scored by landed bonks and credited out-of-arena respawns.

`StartPartyMatch()` starts the party loop from the beginning, clears round wins, performs the existing rematch reset, and sends players to the Party Hall. `StartNextTrial()` selects the Trial by completed trial index. With the current default `TrialsBeforeFinalRound = 2`, the party flow is Mug Run first, Staffs at Dawn second, then Grand Wizard Final. If the configured trial count is increased later, the current selector alternates Mug Run and Staffs at Dawn as a simple placeholder schedule.

`RestartMugRunMatch()` still exists for existing debug controls, but now restarts the whole party loop into a fresh Party Hall intermission.

The Party Hall is represented by `AWizardStaffPartyHall`, a placeholder C++ actor built from cube components and spawn arrows. The game mode searches for a placed authored Party Hall first and falls back to spawning one at `PartyHallSpawnLocation`. Players return there between Trials. The out-of-arena failsafe uses Party Hall bounds while the Party Hall is active, then switches to the active Trial arena bounds during countdown, active play, and results. Countdown now explicitly stages players at Trial spawns and locks prototype input until the Trial becomes active, so players do not get a loose false start from the respawn failsafe.

The Party Hall Ready Bell is a tagged placeholder mesh on the Party Hall actor. Staff bonk impact checks can detect that tag, and `AWizardStaffGameMode` then marks the bonking player ready. Ready state resets every time a new Party Hall intermission begins. The Ready Bell does not grant score, Favor, Staff Stress, or Trial results; it only gives a physical way to skip waiting when the group is ready.

The shared HUD now shows Party Hall/intermission time, each player's Ready Bell state, staff segment count, Grand Wizard Favor, round wins, active tuning preset, current leader labels, Trial-specific score text, Final Round candidate text, and post-match telemetry summary. Round wins and Favor are awarded at Trial Results for Mug Run and Staffs at Dawn. Ties currently grant a round win plus reduced Favor to each tied winner, which is acceptable for the prototype but should be revisited once the larger party scoring rules are designed.

## Grand Wizard Favor And Final Advantage

Grand Wizard Favor is the persistent match-performance score for the current party session. It is separate from current physical staff height.

Favor must stay visibly communicated because it is the score that survives volatile staff physics. The Canvas HUD currently shows Favor in Party Hall standings, Trial Results, active Staffs at Dawn scoring, Final setup, player rows, and the match summary. Favor gains also show short feedback messages such as `P1 gained +3 Favor: Mug Run Winner`.

Current Favor rewards:

- Mug Run winner gains `MugRunWinnerFavor`.
- Staffs at Dawn winner gains `StaffsAtDawnWinnerFavor`.
- Tied Trial winners gain `TiedTrialWinnerFavor`.
- Staffs at Dawn credited ring-outs can optionally grant `FavorPerStaffsAtDawnRingOut`.

Favor persists across Mug Run, Staffs at Dawn, Results, Party Hall, and the Grand Wizard Final. It resets only when the full party match restarts/rematches through `StartPartyMatch()`.

Final Candidate selection now uses:

1. Highest Grand Wizard Favor.
2. Highest round wins.
3. Highest current staff segment count.
4. Lower player index as the prototype fallback.

At Final start, `bUseFavorBasedFinalStaffSetup` can rebuild or adjust each player's physical staff from Favor so earned performance becomes visible. The starting Candidate receives `CandidateMaxStartingSegments` so they feel like the Grand Wizard opponent. Challengers use `ChallengerStartingBaseSegments`, `ChallengerSegmentsPerFavor`, and `ChallengerMaxStartingSegments`. Rebuilt Final staff segments also apply Mana Slosh from their segment count, so the Grand Wizard's huge staff comes with the same control cost as Trial staff growth. By default, `bLockManaSloshAtFinalStart` then locks each wizard's Mana Slosh at that Final-start value for the rest of the Final, preventing passive decay from making the showdown sober. Staff segment snaps can still reduce the locked Slosh value. Final setup feedback explains why the Candidate was chosen and briefly shows the rebuilt starting staff counts.

Design note: every Trial should now contribute either to physical staff growth, Grand Wizard Favor, or both. Any future Trial must visibly communicate which of those rewards it grants. This keeps the Final from depending only on the current snap-prone staff height.

## Grand Wizard Final Round

After `TrialsBeforeFinalRound` completed Trials, the party match enters the Grand Wizard Final Round. The default target is two Trials before the final: Mug Run, then Staffs at Dawn. The current implementation reuses the prototype arena and spawns a flat placeholder ritual circle at the arena center.

At final-round start:

- The player with the highest Grand Wizard Favor becomes the Grand Wizard Candidate.
- Ties are resolved by round wins, then current staff segment count, then lower player index as a prototype tie-breaker.
- If Favor-based Final setup is enabled, player staff sizes are rebuilt or adjusted from Favor before the Final begins.
- The Candidate is placed in the center ritual circle.
- Other players are respawned at normal arena spawn points.
- Mugs are inactive.

During the final:

- The Candidate keeps control of the title while inside or near the ritual circle.
- By default, challengers can only steal after the Candidate has been knocked outside the circle control area.
- A challenger standing in the circle fills a short hold timer.
- When that hold completes, the challenger becomes the new Candidate.
- Existing bonk, staff collision, staff stress, loose segment chaos, and out-of-arena respawn systems remain active.
- Whoever is Candidate when the final timer expires wins the whole match.
- After the winner is shown for `PostMatchDuration`, the game automatically rematches back to Party Hall.

Important tuning lives in `FWizardFinalRoundTuning`:

- `FinalRoundDuration`, default `60.0`
- `CircleRadius`
- `CandidateNearCirclePadding`
- `CandidateSwapHoldDuration`
- `bRequireCandidateOutsideCircleToSteal`
- `bLeaderStartsWithBonus`
- `LeaderStartBonusDuration`
- `bUseFavorBasedFinalStaffSetup`
- `CandidateStartingBaseSegments`
- `CandidateSegmentsPerFavor`
- `CandidateMaxStartingSegments`
- `ChallengerStartingBaseSegments`
- `ChallengerSegmentsPerFavor`
- `ChallengerMaxStartingSegments`
- `bShowDebug`

Important tuning lives in `FWizardPartyMatchTuning`:

- `IntermissionDuration`
- `TrialCountdownDuration`
- `TrialResultsDisplayDuration`
- `TrialsBeforeFinalRound`
- `bResetStaffsAtTrialStart`
- `bShowDebug`

Important tuning lives in `FWizardGrandWizardFavorTuning`:

- `MugRunWinnerFavor`
- `StaffsAtDawnWinnerFavor`
- `TiedTrialWinnerFavor`
- `bGrantFavorForStaffsAtDawnRingOuts`
- `FavorPerStaffsAtDawnRingOut`

Mug Run trial reset currently:

- Clears winner text and resets the match timer.
- Resets pending out-of-arena respawn timers.
- Moves each wizard back to their spawn point.
- Optionally resets Mana Slosh.
- Clears hit reaction and bonk state.
- Resets staff stress and stuck/collision relief state.
- Optionally clears staff segments.
- Destroys and respawns the Mug Run pickups.
- Optionally cleans up loose snapped staff segment physics actors.

Mug Run now has a minimal brew reward framework. A mug adds one staff segment and adds Mana Slosh through that staff growth. While Mug Run is active, each mug can optionally roll for a carried one-use brew reward using `bEnableBrewRewardsInMugRun`, `BrewRewardChance`, and `bReplaceExistingRewardOnPickup` on `FWizardMugRunTuning`. Each wizard has one carried reward slot, currently supporting `None` and `ArcanePinball`. Default input is right mouse on keyboard/mouse and left bumper on gamepad; the player-2 keyboard fallback uses semicolon. Carried brew rewards are cleared when Mug Run ends so they do not enable rewards in Staffs at Dawn or the Grand Wizard Final.

Arcane Pinball is the first real brew reward spell. Using it fires a placeholder sphere projectile in the wizard's facing direction and adds significant Staff Stress to the caster through `StressOnCast`. It uses `UProjectileMovementComponent` to bounce off blocking walls, props, players, and other arena objects. The projectile is locked to the height it was fired from by default so ricochets stay inside the playable arena instead of climbing upward. Each blocking bounce, including bounces off players, speeds the projectile up exponentially using `SpeedMultiplierPerWallBounce`, capped by `MaxProjectileSpeed`. On player hit it applies a light bonk-style reaction, optional Mana Slosh, and Staff Stress through `StressOnHit`, then either continues bouncing or destroys depending on tuning. Self-hit is allowed by default, so careless casters can stress their own staff by getting hit by their own spell. Spells do not cost mana because the standalone mana resource has been removed; the risk comes from Staff Stress, Mana Slosh, self-hit potential, and physics chaos. Main tuning lives on each wizard in `FWizardArcanePinballTuning`: `ProjectileSpeed`, `SpeedMultiplierPerWallBounce`, `MaxProjectileSpeed`, `bLockHeightToLaunchHeight`, `MaxBounces`, `Lifetime`, `HitKnockback`, `SloshOnHit`, `StressOnCast`, `StressOnHit`, `bAllowSelfHit`, and `bDestroyOnPlayerHit`.

Prototype tuning presets now also control brew reward chance and Arcane Pinball intensity. Brew reward chances are `Stable` 0.35, `Chaotic` 0.50, and `Arcane Catastrophe` 0.65. `Stable` lowers spell frequency and keeps Pinball slower, shorter-lived, lower knockback, and less stressful. `Chaotic` is the normal spell baseline: useful enough to create magical interruptions without replacing staff bonking. `Arcane Catastrophe` has a higher mug reward chance, faster Pinball launch speed, stronger bounce speed ramp, more bounces, and higher cast/hit stress than `Chaotic`.

Current Arcane Pinball preset intent:

- `Stable`: readable chaos, 1050 launch speed, 1.10 bounce speed multiplier, 2600 max speed, 5 bounces, 4.25 second lifetime, 360 hit knockback, 4 slosh on hit, 10 cast stress, and 4 hit stress.
- `Chaotic`: default playtest chaos, 1350 launch speed, 1.22 bounce speed multiplier, 3900 max speed, 7 bounces, 5 second lifetime, 500 hit knockback, 8 slosh on hit, 22 cast stress, and 8 hit stress.
- `Arcane Catastrophe`: wild chaos, 1750 launch speed, 1.35 bounce speed multiplier, 6500 max speed, 12 bounces, 6.2 second lifetime, 700 hit knockback, 16 slosh on hit, 42 cast stress, and 18 hit stress.

Self-hit remains enabled in all presets because it supports the comedy loop. Arcane Pinball does not destroy on player hit by default, so it can continue ricocheting after hitting a wizard. Player-hit cooldown prevents one projectile from machine-gunning the same wizard every frame.

Each wizard now carries a placeholder spellbook mesh near the left-hand area. The spellbook remains visible as a simple inactive book, then enables a colored glow shell when the wizard has a carried brew reward ready. Spell glow colors should be unique per reward for readability. Arcane Pinball currently uses a fuchsia glow.

Staffs at Dawn trial reset currently:

- Clears winner text and resets the Staffs at Dawn timer.
- Clears Staffs at Dawn scores.
- Clears recent bonk attacker tracking used for ring-out credit.
- Resets pending out-of-arena respawn timers.
- Moves each wizard back to arena spawn points when the Trial starts.
- Keeps Mug Run pickups inactive.

Important tuning lives in `FWizardMugRunRematchTuning`:

- `PostMatchDuration`
- `bResetStaffsBetweenMatches`
- `bResetSloshBetweenMatches`

`PostMatchDuration` is kept for compatibility with earlier rematch tuning, but the new trial framework uses `TrialResultsDisplayDuration` for the visible results state.

## Local Multiplayer

Local multiplayer is handled by `AWizardStaffGameMode`.

- `DesiredLocalPlayerCount` supports two to four local players.
- The default is currently two local players.
- The game mode uses local player creation only. There is no replication or online session work.
- Splitscreen is disabled. All players use the same shared camera view.
- Player colors are assigned by player index so each wizard is readable in the arena.

Keyboard and mouse mostly drive player 1. Gamepads should be used for reliable two to four player testing. For controller trouble, the game mode also has a simple player-2 keyboard fallback.

## Input And Movement Sensitivity Tuning

Input currently uses Unreal's legacy action/axis mappings while the project config points at Enhanced Input player/input component classes. The important tuning values are split between `Config/DefaultInput.ini`, `AWizardStaffWizardCharacter`, and the player-2 keyboard fallback on `AWizardStaffGameMode`.

Keyboard turning:

- `Config/DefaultInput.ini` maps `Q` and `E` to the `Turn` axis.
- The current `Q`/`E` axis scales are `-2.0` and `2.0`, which makes key turning more responsive than the earlier slower feel.
- Those scaled axis values feed into `AWizardStaffWizardCharacter::Turn()`.
- `AWizardStaffWizardCharacter::TurnRateDegreesPerSecond` is the shared base turn rate after input axis scaling.

Mouse turning:

- `Config/DefaultInput.ini` maps `MouseX` to the `Turn` axis with scale `1.0`.
- `AxisConfig` also sets `MouseX` sensitivity, currently `0.07`.
- Mouse feel is therefore a combination of `MouseX` axis sensitivity, the `Turn` axis scale, and `TurnRateDegreesPerSecond`.

Gamepad stick turning:

- `Config/DefaultInput.ini` maps `Gamepad_RightX` to the `Turn` axis.
- The current right-stick turn axis scale is `2.0`, matching the recent sensitivity improvement for controller turning.
- `AxisConfig` for `Gamepad_RightX` still controls dead zone, sensitivity, exponent, and inversion before the value reaches the wizard.
- Final turn speed still passes through `TurnRateDegreesPerSecond`.

Movement response:

- `MoveForward` and `MoveRight` are mapped to `WASD`, arrow keys, and the left stick in `Config/DefaultInput.ini`.
- `AWizardStaffWizardCharacter::MoveInputScale` is the global input multiplier.
- `WalkSpeed` controls top movement speed.
- `MaxAcceleration` controls how quickly movement starts/responds.
- `BrakingDecelerationWalking` controls how quickly the wizard stops.
- Slosh, Staff Heft, staff obstruction, and hit reactions multiply or bend these values at runtime. The target feel is silly and compromised, not unreadable or helpless.

Jump and broom boost:

- `Jump` is mapped to `SpaceBar` and gamepad bottom face button in `Config/DefaultInput.ini`.
- Pressing Jump while grounded performs the normal wizard hop.
- Pressing Jump again while airborne triggers a one-use broom boost for that airtime.
- The broom boost shows a simple placeholder broom under the wizard and pushes them forward briefly, giving players a clutch way to fight a ring-out.
- The boost resets after the wizard lands and the short landing cooldown completes.
- The current default is intentionally short and mostly horizontal with a little vertical forgiveness: `BoostDuration` is `0.55`, `ForwardSpeed` is `1050`, `InitialForwardBoost` is `720`, `InitialUpwardBoost` is `145`, and `MinVerticalVelocityDuringBoost` is `-80`.
- Strong bonk knockback is no longer instantly erased by the boost; the broom pushes against current velocity instead of fully replacing it.
- Current control defaults keep Slosh meaningful without making the broom feel unusable: `SloshControlPenaltyAtMax` is `0.22`, `StaffHeftControlPenaltyPerHeavySegment` is `0.075`, `MaxStaffHeftControlPenalty` is `0.60`, and `MinControlMultiplier` is `0.25`.
- When `bEnableHighSloshDisorder` is enabled, `HighSloshDisorderThreshold` starts disorder at `75` Slosh, `MinDisorderAlphaAtThreshold` starts the effect gently at `0.10`, and `FullDisorderSlosh` is `120` so normal max Slosh does not reach full disorder. `HighSloshDisorderChanceAtFullSlosh` is `0.45`, so disorder is occasional rather than guaranteed. Current disorder caps are `MaxDisorderYawDegrees = 60`, `MaxDisorderSideImpulse = 120`, and `MaxDisorderVerticalImpulse = 50`.
- Important tuning lives on `AWizardStaffWizardCharacter::BroomBoostTuning`: `BoostDuration`, `ForwardSpeed`, `InitialForwardBoost`, `InitialUpwardBoost`, `VelocityLerpSpeed`, `MinVerticalVelocityDuringBoost`, `SloshControlPenaltyAtMax`, `StaffHeftControlPenaltyPerHeavySegment`, `MaxStaffHeftControlPenalty`, `MinControlMultiplier`, `LandingCooldown`, high-Slosh disorder values, and placeholder broom visual offsets/scales. The broom visual is intentionally a little oversized so recovery attempts read clearly during Staffs at Dawn ring-out moments.

Player 2 keyboard fallback:

- `AWizardStaffGameMode::KeyboardFallbackControls` drives player 2 directly from keys like `IJKL` and `U`/`O`.
- The fallback sends raw movement and turn values into `AWizardStaffWizardCharacter::ApplyPrototypeLocalInput()`.
- Because it bypasses `DefaultInput.ini` axis scales, fallback turn feel is tuned mainly with `TurnRateDegreesPerSecond`.
- There is no separate fallback sensitivity scalar yet. Add one only if playtesting shows player 2 keyboard needs a different feel than normal keyboard/controller turning.

## Wizard Character

`AWizardStaffWizardCharacter` is the main player pawn class. It is a `ACharacter`, not a full physics body.

Current responsibilities:

- Basic movement, turning, hop/jump, and one-use airborne broom boost.
- Placeholder wizard visuals built from simple meshes.
- Per-player color assignment.
- Player ground marker for readability.
- Leader marker for the current tallest staff holder.
- Mana Slosh value.
- Quick Bonk input and bonk timing.
- Hit reactions from bonks.
- Staff heft penalties for very large staffs.
- Debug commands for tuning and testing.

The wizard remains intentionally controllable. Slosh, heft, obstruction, and hit reactions all reduce or bend control, but the character is still driven through the character movement system.

## Staff Segment System

`UWizardStaffComponent` owns the visible staff system.

Each wizard starts with a base staff. Mugs add visible placeholder mug/drink segments. The component tracks `SegmentCount` and exposes `AddStaffSegment()`, `RemoveTopStaffSegment()`, `ClearStaffSegments()`, and `SnapTopStaffSegment()`.

Segments are attached in a chain. If meshes provide sockets, the component can use `TopSocketName` and `BottomSocketName`. The current placeholder setup also has a calculated fallback based on segment height and spacing, so the stack works without custom mug meshes.

Important tuning lives in `FWizardStaffVisualTuning`:

- `BaseStaffFallbackHeight`
- `BaseStaffVisualScale`
- `SegmentFallbackHeight`
- `SegmentVisualScale`
- `SegmentSpacing`
- `MaxTestSegments`, default `16`; this should stay at or above `CandidateMaxStartingSegments` so the Grand Wizard Candidate can receive their full Final staff.
- staff and segment colors

The current visuals are not final. The useful part is the reusable attachment logic and segment count tracking.

## Staff Collision

The staff collision is a hybrid setup, not a full physics chain.

`UWizardStaffComponent` creates one attached `UBoxComponent` that roughly matches the full staff length. The box grows as `SegmentCount` increases. It collides with world props and walls, but it is not simulating each mug as its own rigid body.

This keeps the wizard controllable while still allowing the staff to catch on walls, doorways, and blocks. The collision box can obstruct movement and feed stress into the staff, but the character remains character-movement driven.

Important tuning lives in `FWizardStaffCollisionTuning`:

- `bStaffCollisionEnabled`
- `BaseCollisionLength`
- `CollisionLengthPerSegment`
- `CollisionThickness`
- `CollisionLocalOffset`
- `ObstructedControlMultiplier`
- `ObstructionRecoverySpeed`

This is one of the most important feel areas. If the staff is too forgiving, large staffs feel overpowered. If the collision is too strong, players can get launched or hard-stuck.

## Stuck Behavior

The prototype keeps the comedy of getting wedged, but tries to prevent true soft-locks.

`UWizardStaffComponent` watches for cases where the player is providing movement input, the owner is barely moving, and the staff has a blocking obstruction. When that persists, the stuck failsafe starts adding stress faster. If the player stays stuck long enough, the component can briefly reduce collision pressure and gently nudge the owner.

The preferred escape is still staff stress and snapping. Collision relief and nudging are backup safety valves.

Important tuning lives in `FWizardStaffStuckTuning`:

- `bEnableStuckFailsafe`
- `MovementInputThreshold`
- `MovementInputGraceTime`
- `OwnerMoveSpeedThreshold`
- `StuckTimeBeforeStressBoost`
- `StuckStressPerSecond`
- `StuckTimeBeforeCollisionRelief`
- `CollisionReliefDuration`
- `CollisionReliefCooldown`
- `CollisionReliefControlMultiplier`
- `GentleNudgeDistance`

## Staff Stress And Snapping

`UWizardStaffComponent` tracks `StaffStress`. Stress is generated by bonking, wall impacts, and being caught against obstacles. Longer staffs multiply stress gain, making huge staffs more fragile.

When `StaffStress` reaches `MaxStaffStress`, the top staff segment snaps off:

- The top visual segment is removed from the attached staff.
- `SegmentCount` decreases.
- A small physics segment actor is spawned at the removed segment transform.
- The segment receives a mass-aware impulse and light damping so it pops loose without usually flying far from the break point.
- Staff stress is reduced to a post-snap ratio.
- The wizard reduces Mana Slosh through the snap callback. Current tuning removes `4` in `Stable`, `11` in `Chaotic`, and `19` in `Arcane Catastrophe`; Stable intentionally vents less because that preset was too hard to get sloshed in.

The loose physics segment can currently create funny player interactions. That behavior is intentionally preserved for now. Because it can sometimes launch players out of the arena, the game mode has an out-of-arena respawn failsafe. The snapped segment launch was tuned to stay more local to the break by default; use `bSnapImpulseIgnoresSegmentMass` only when deliberately testing older, more explosive launches.

Important tuning lives in `FWizardStaffStressTuning`:

- `MaxStaffStress`
- `StressGainedPerBonk`
- `StressGainedPerWallImpact`
- `StressMultiplierPerSegment`
- `StressDecayRate`
- `WallImpactSpeedForFullStress`
- `CaughtStressPerSecond`
- `StressAfterSnapRatio`
- `SnapImpulseForce`
- `SnapUpwardImpulse`
- `SnappedSegmentLinearDamping`, default `1.25`
- `SnappedSegmentAngularDamping`, default `0.7`
- `bSnapImpulseIgnoresSegmentMass`, default `false`

Recommended starting intent: keep snapped segments obvious and funny, but have them tumble around the nearby fight instead of leaving the arena immediately. If snapped pieces feel too dead, lower `SnappedSegmentLinearDamping` slightly before increasing `SnapImpulseForce`. If they still fly too far, raise damping or keep `bSnapImpulseIgnoresSegmentMass` disabled.

## Loose Snapped Segment Chaos Effects And Budget

Loose snapped staff segments are now tracked by `AWizardStaffGameMode` after they spawn. The staff component still creates the physics actor, applies the same impulse, and lets it behave as a loose physics object. After that initial comedy moment, the game mode registers the actor as a loose snapped segment.

Tracked loose segments use weak references, so cleanup is safe if a segment was already destroyed by some other path. The game mode also tags loose segment actors with `WizardStaffLooseSegment`, which lets rematch cleanup catch any tagged loose pieces even if they were not in the tracked array.

Freshly snapped segments now have a short unstable magic window. During that window, sufficiently hard impacts can trigger modest prototype chaos effects in both Mug Run and Staffs at Dawn:

- `ManaSplash`: adds a small amount of Mana Slosh to a hit wizard.
- `TripBonk`: applies a small stumble/knockback to a hit wizard.
- `ArcanePop`: creates a small radial knockback burst on hard impacts.

The chaos window is intentionally short, cooldown-limited, and non-scoring. It is meant to make snapped pieces feel magically unstable without adding a new Trial, broad powerup system, permanent hazard, damage model, puddles, or chain-reaction mechanic. Loose segment effects should stay funny and occasional, not oppressive.

The chaos budget has two cleanup paths:

- Active-window cleanup: when loose segment chaos effects are enabled, the segment starts fading after `ChaosActiveDuration`.
- Lifetime cleanup: when chaos effects are disabled or the active window is not being used, after `LooseSegmentLifetime`, the segment starts a simple scale-down fade if `FadeOutDuration` is greater than zero, then destroys itself.
- Max-count cleanup: if more than `MaxLooseSegments` exist, the oldest tracked segment is destroyed immediately.

Important tuning lives in `FWizardLooseSegmentChaosTuning`:

- `bEnableLooseSegmentChaosEffects`, default `true`.
- `ChaosActiveDuration`, recommended starting value `4.0` seconds.
- `ChaosTriggerCooldown`, recommended starting value `0.85` seconds.
- `MinImpactSpeedForChaosEffect`, recommended starting value `340`.
- `bAffectOwner`, default `true` so reckless snapped pieces can still punish their source.
- `bAffectOtherPlayers`, default `true`.
- `bShowLooseSegmentChaosDebug`, default `false`.
- `ManaSplashSloshAmount`, recommended starting value `5`.
- `TripBonkKnockback`, recommended starting value `260`.
- `TripBonkUpwardBoost`, recommended starting value `30`.
- `ArcanePopRadius`, recommended starting value `160`.
- `ArcanePopKnockback`, recommended starting value `220`.
- `ArcanePopUpwardBoost`, recommended starting value `40`.
- `LooseSegmentLifetime`
- `MaxLooseSegments`
- `bCleanupLooseSegmentsOnRematch`
- `FadeOutDuration`

Current tuning intent: ManaSplash and TripBonk should be the common loose segment effects, while ArcanePop should be rare and reserved for very hard bounces. If loose segments start dominating Mug Run or Staffs at Dawn, first raise `MinImpactSpeedForChaosEffect`, raise `ChaosTriggerCooldown`, or lower `TripBonkKnockback`. If they are unreadable or never matter, lower `MinImpactSpeedForChaosEffect` slightly before increasing effect strength. Keep Playtest HUD feedback minimal; use `bShowLooseSegmentChaosDebug` and `FullDebug` HUD mode for tuning.

## Out-Of-Arena Respawn

`AWizardStaffGameMode` handles a simple safety respawn when a wizard leaves the playable arena. This was added to protect the prototype from extreme physics launches without removing the funny loose-segment behavior.

If a wizard is horizontally outside the active arena center plus arena half-size and padding, or falls below the Z threshold, the game mode schedules a respawn. After the delay, the wizard is returned to an arena spawn transform, movement is stopped, and walking movement is restored. If the wizard naturally comes back into bounds before the delay, the pending respawn can be canceled.

Staffs at Dawn also uses the arena actor's `RingOutFallDistanceBelowArena` line. Its default is intentionally close under the raised platform so landing on or hopping around the lower floor counts as out-of-play quickly, while a real broom recovery back toward the playable arena can still cancel the pending respawn.

Important tuning lives in `FWizardOutOfArenaRespawnTuning`:

- `bEnableOutOfArenaRespawn`
- `RespawnDelay`
- `HorizontalOutOfBoundsPadding`
- `FallZThreshold`
- `bCancelRespawnIfPlayerReturns`
- `bShowDebug`

The current default respawn delay is 1.5 seconds.

## Mana Slosh

The standalone mana resource has been removed from active gameplay. Spells and brew rewards do not cost mana. Mana Slosh remains on `AWizardStaffWizardCharacter` as the readable comedy pressure that makes greedy staff growth harder to control. In playtest language, wizards can still be called mana sloshed or sloshed.

`DrinkMug()` currently does three things:

- Adds one staff segment through the staff component.
- Adds Mana Slosh only because the staff segment was successfully gained.
- Optionally rolls for a one-use Mug Run brew reward when that system is enabled.

The older `DrinkManaMug()` function is intentionally kept as a deprecated compatibility wrapper for any prototype Blueprint or input references that still call it. It no longer restores a standalone mana resource.

Mana Slosh is tied to successful staff growth rather than to a specific pickup or Trial. This keeps the central loop consistent: growing your staff makes you more powerful, more awkward, and more compromised. Arcane Pinball can also add Mana Slosh on hit as a spell-effect tuning value.

Current gameplay sources of staff-growth Mana Slosh:

- Mugs in Mug Run, because they grant one staff segment.
- Staffs at Dawn combat staff growth from landed bonks.
- Staffs at Dawn ring-out staff growth rewards.
- Mega Staff Brew temporary staff growth.
- Favor-based Grand Wizard Final staff setup.

Mana Slosh effects:

- Reduces movement speed.
- Reduces turn responsiveness.
- Reduces acceleration and braking.
- Adds slight oversteer.
- Can create occasional stumble kicks at high slosh.
- Adds visual wobble to the wizard and staff.
- Is reduced when staff segments snap off, giving fully sloshed players a slow comeback path.
- Locks during the Grand Wizard Final by default after Final staff setup, so the earned Final staff advantage keeps its control cost unless staff segments snap off and vent Slosh.
- Avoids strong camera wobble.

Important tuning lives in `FWizardManaSloshTuning`:

- `SloshGainedPerStaffSegment`
- `SloshGainedPerDrink`, legacy compatibility value mirrored from segment-growth tuning for old Blueprint references
- `MaxSlosh`
- `SloshDecayPerSecond`
- `SloshReducedOnStaffSnap`, tuned per preset as the Slosh relief valve when a segment breaks
- movement, turn, acceleration, and braking penalties
- oversteer values
- stumble values
- visual wobble values
- debug Mana Slosh levels

## Staff Heft

Large staffs now become harder to manage even before they snap.

`FWizardStaffHeftTuning` adds penalties after a configurable segment count. Heavy staffs reduce movement and turning, and they also slow down bonk timing by increasing hit delay, cooldown, and visual duration.

This keeps huge staffs powerful but less abusable. The player still wants a tall staff, but a very tall staff becomes slower, more awkward, and easier to punish.

Important tuning:

- `SegmentCountBeforeHeftPenalty`
- `MovementPenaltyPerHeavySegment`
- `TurnPenaltyPerHeavySegment`
- `MaxMovementPenalty`
- `MaxTurnPenalty`
- `BonkCooldownPerHeavySegment`
- `BonkVisualDurationPerHeavySegment`
- max bonk timing bonuses

## Bonk Combat

Bonk combat is currently contact-driven by the staff's collision box.

The wizard performs a 90-degree downward staff strike from above the player toward the floor. When the staff reaches the completed strike pose, the character queries the current staff collision box against pawns once. A wizard is hit only if the staff collision overlaps them at that final impact moment.

This replaced the earlier broad sweeping arc. The current behavior is more physical and less abusable: the staff actually needs to make contact.

Bonk behavior:

- Basic quick bonk is free.
- There is a cooldown to prevent spam.
- The hit check is a single end-of-strike impact sample, not a full-animation damage sweep.
- Each target can only be hit once per swing.
- Staff length increases contact reach because the staff collision gets longer as segments are added.
- Knockback now scales from base bonk strength, then current Mana Slosh, then staff segment count, up to a cap.
- Staff size still increases knockback, but sloshed wizards hit a little harder before segment scaling is added.
- Large staffs have slower bonk timing through Staff Heft.
- Bonking adds Staff Stress even on whiffs, with stronger stress on hits.
- Hit feedback uses simple placeholder visual/audio hooks and debug output.

Staff Clash is a small prototype counter to frequent same-angle simultaneous bonks. It is an emergent mechanic formalized from natural simultaneous bonk behavior, not a timed event. If two wizards are both actively swinging toward each other within a short timing window, their normal bonks are suppressed and both wizards lock into the final strike pose for the clash duration, default `1.0` second. Jump and active broom boost movement are canceled, and each wizard's position is locked at the clash-start location until the clash resolves. During the lock, pressing Quick Bonk adds mash count. When the timer ends, the wizard with more mash presses wins the clash, their bonk goes through as a stronger knockback hit, and the loser's bonk fizzles. If the mash count ties, both bonks fizzle. While wizards are clashing, third-party bonks and Arcane Pinball hits ignore them so the clash stays readable.

`ClashDuration` is how long an active clash lasts after it has already triggered. It does not control how often clashes happen. If clashes happen too often, first narrow `ClashTimingWindow`; if clashes rarely happen, slightly widen `ClashTimingWindow` before loosening facing thresholds. Clash winner knockback now uses the winner's current staff size twice: normal bonk strength already scales from staff segments, and the special clash winner multiplier also gains a small capped per-segment bonus. If clash wins feel weak, slightly raise `WinnerKnockbackMultiplier` or `WinnerKnockbackMultiplierPerStaffSegment`; if they create too many cheap ring-outs, lower `WinnerMaxKnockbackMultiplier`, `WinnerKnockbackMultiplier`, or `WinnerUpwardBoostMultiplier`.

Important tuning lives in `FWizardBonkTuning`:

- `BaseRange`
- `RangePerStaffSegment`
- `KnockbackStrength`
- `KnockbackPerManaSlosh`
- `KnockbackPerStaffSegment`
- `MaxKnockbackStrength`
- `UpwardBoost`
- `Cooldown`
- `StaffContactPadding`
- `StrikeStartPitchDegrees`
- `StrikeEndPitchDegrees`
- `StrikeSideWobbleDegrees`
- `VisualDuration`
- `HitStressMultiplier`
- `WhiffStressMultiplier`
- feedback values and optional sounds

`BaseRange` and `RangePerStaffSegment` still exist as useful reporting/fallback values, but the main hit detection path is now the staff collision box.

Staff Clash tuning lives in `FWizardStaffClashTuning`:

- `bEnableStaffClash`
- `ClashDuration`
- `ClashTimingWindow`
- `FacingDotThreshold`
- `OpposingForwardDotThreshold`
- `WinnerKnockbackMultiplier`
- `WinnerKnockbackMultiplierPerStaffSegment`
- `WinnerMaxKnockbackMultiplier`
- `WinnerUpwardBoostMultiplier`
- `InputLockMultiplier`
- `bShowDebug`

## Hit Reactions

Bonked players use a light reaction system on `AWizardStaffWizardCharacter`. This is not full ragdoll.

On hit, the victim receives character-movement knockback and temporary control reduction. Stronger hits can produce a short stronger control loss or a simple faux knockdown lean. High Mana Slosh increases stumble severity, so drunker wizards are easier to disrupt.

Important tuning lives in `FWizardHitReactionTuning`:

- `StumbleDuration`
- `KnockbackScale`
- `SloshToStumbleMultiplier`
- `RecoveryTime`
- `StrongHitThreshold`
- strong-hit control values
- stumble/recovery control multipliers
- optional knockdown values
- visual lean values

The goal is fast recovery and readable comedy, not long loss-of-control punishments.

## Mug Run Trial

Mug Run is the first implemented Trial and remains the main staff-growth mode.

`AWizardStaffGameMode` owns the match timer, mug spawning, score comparison, leader highlighting, and winner message. `AWizardStaffManaMugPickup` owns the individual pickup behavior.

Mugs are placeholder actors with overlap collision and simple mug-like meshes. On overlap with a wizard, the pickup calls `DrinkMug()`, becomes inactive, and respawns after its cooldown if respawn is enabled.

Important tuning:

- `FWizardMugRunTuning.MatchDuration`, default `60.0`
- `FWizardMugRunTuning.MugSpawnCount`
- `FWizardMugRunTuning.MugSpawnZ`
- `FWizardManaMugPickupTuning.RespawnDelay`
- `FWizardManaMugPickupTuning.PickupRadius`
- `FWizardManaMugPickupTuning.bRespawnAfterPickup`

Score is currently staff segment count at the end of the Trial. Ties are reported as ties. Mug Run winners also earn Grand Wizard Favor, so a good Mug Run can matter in the Final even if that staff later snaps down during Staffs at Dawn.

## Staffs At Dawn Trial

Staffs at Dawn is the second implemented Trial. It is a pure bonk-combat Trial meant to showcase staff length, staff heft, slosh, snapping, knockback, out-of-arena recovery, and Arcane Catastrophe chaos without adding spells or new weapons.

The current implementation uses `AWizardStaffStaffsAtDawnArena` when a placed or runtime arena is available, and falls back to the Mug Run/prototype arena if not. Mug Run pickups stay disabled during this Trial. Players keep using the same wizard movement, staff collision, bonk, stress, snapping, and out-of-arena respawn systems.

The Staffs at Dawn arena is intentionally less cluttered than Mug Run. It is now a larger raised island arena, roughly twice the original footprint, so normal bonks do not always become automatic ring-outs. The current tuning uses a larger central combat platform for recovery, wider bridge lanes for risky but playable crossings, larger outer duel pads near exposed edges, safer central player spawns, a few pillars/low walls for bounce surfaces, and a small number of staff-catching props near bridge approaches. Runtime fallback spawns it away from the Mug Run arena at `RuntimeStaffsAtDawnArenaLocation`, so the two layouts do not overlap.

Scoring currently works like this:

- Landing a staff bonk scores `PointsPerBonk`, default `+1`.
- If a recently bonked player is launched or falls out of the arena and triggers the respawn failsafe, the last credited bonker scores `PointsPerOutOfArena`, default `+3`.
- The ring-out credit uses `OutOfArenaCreditWindow`, default `4.5` seconds.
- Every `LandedBonksPerStaffSegment` landed bonks grants +1 staff segment, default every `2` landed bonks. Setting the value to `0` disables this combat staff growth.
- Credited ring-outs grant `StaffSegmentsPerOutOfArena` staff segments, default `+2`. Setting the value to `0` disables ring-out staff growth.
- Staff growth during Staffs at Dawn adds Mana Slosh through `SloshGainedPerStaffSegment`, so combat growth now creates the same power-versus-control tradeoff as Mug Run growth.
- A player who is ringed out during Staffs at Dawn respawns with physical staff segments reset. Their score, Favor, and Trial performance remain, but the immediate big-staff threat is cleared so knocking out a huge wizard has a real payoff.
- Broom boost recovery telemetry uses `BroomBoostRecoveryTelemetryWindow`, default `2.0` seconds. If a player uses broom boost shortly before or during an out-of-arena respawn threat, returning to bounds counts as a broom boost save; respawning counts as a failed broom boost recovery.
- Credited ring-outs can optionally grant persistent Grand Wizard Favor.
- Staff-growth rewards show short feedback such as `P1 staff grew +1: Landed Bonks` so players understand why their staff changed without a mug pickup.
- Staffs at Dawn powerup pickups can spawn from `FuturePowerupSpawn_*` markers when `bEnableStaffsAtDawnPowerups` is enabled. The current default is more visible for 60-second rounds with `PowerupSpawnCount = 2`, `InitialPowerupSpawnDelay = 5`, `PowerupRespawnDelay = 18`, and `MegaStaffPickupRespawnDelay = 18`. Each spawn rerolls from the available markers and prefers a different unreserved marker, so Mega Staff Brew should not keep appearing in the exact same arena spot. The only current type is `MegaStaffBrew`.
- Mega Staff Brew temporarily grants a large number of real staff segments, default `+5` for `8` seconds. These segments immediately interact with existing staff heft, collision length, stress, snapping, bonk cooldown, knockback, and broom boost penalties. It is meant to feel powerful and scary, but not safe.
- The active Mega Staff player gets a larger pulsing green ground marker and tinted hat, plus short debug messages when the effect is picked up, about to expire, expires, or loses temporary segments to snapping. This is placeholder readability for the shared camera, not final VFX.
- Current tuning makes Mega Staff slightly more explosive but riskier: stress is multiplied during the effect, knockback is mildly boosted, and the shorter duration should create a quick panic window instead of a safe super mode.
- When Mega Staff Brew expires, the wizard rebuilds back down by the number of temporary segments still tracked as active. Snapped temporary segments decrement that count, so the effect should not remove extra permanent Staffs at Dawn combat-growth segments when it ends.
- Staffs at Dawn also clears active Mega Staff Brew effects and broom recovery tracking when the Trial ends, so temporary powerup state should not leak into Results, the Final, or the next party cycle.
- Staff segment snapping is still tracked in telemetry, but snapping another player's segment is not yet directly scored because opponent attribution is not reliable enough yet.
- Ties are allowed and currently grant a round win plus reduced Favor to each tied winner at Trial Results.

Important tuning lives in `FWizardStaffsAtDawnTuning`:

- `TrialDuration`, default `60.0`
- `PointsPerBonk`
- `PointsPerOutOfArena`
- `OutOfArenaCreditWindow`
- `LandedBonksPerStaffSegment`
- `StaffSegmentsPerOutOfArena`
- `BroomBoostRecoveryTelemetryWindow`
- `bEnableStaffsAtDawnPowerups`
- `PowerupSpawnCount`
- `PowerupRespawnDelay`
- `InitialPowerupSpawnDelay`
- `bRespawnPowerupsAfterPickup`
- `DefaultPowerupType`
- `MegaStaffBonusSegments`
- `MegaStaffDuration`
- `MegaStaffPickupRespawnDelay`
- `MegaStaffStressMultiplierDuringEffect`
- `MegaStaffKnockbackMultiplierDuringEffect`
- `bRemoveTemporarySegmentsOnExpire`
- `bShowMegaStaffDebug`
- Wizard readability values: `MegaStaffMarkerScaleMultiplier`, `MegaStaffMarkerPulseScale`, `MegaStaffExpireWarningTime`
- `bShowDebug`
- `bShowArenaDebug`
- `bDrawArenaBoundsDebug`
- `bDrawPlayerSpawnDebug`
- `bDrawRingOutBoundsDebug`
- `bDrawFuturePowerupSpawnDebug`

Current design note: Staffs at Dawn should stay simple for now. Its purpose is to test whether the existing staff physics, bonk timing, slosh, snapping, combat staff growth, broom recovery, ring-outs, and presets can carry a combat round. The powerup framework is intentionally tiny: one pickup type, one short temporary effect, and no multiple-powerup system.

## Prototype Arena

Mug Run uses the hand-authored workflow through `AWizardStaffPrototypeArena`.

`AWizardStaffGameMode` searches for a placed `AWizardStaffPrototypeArena` at BeginPlay when `bUseAuthoredPrototypeArena` is enabled. If one is found, that actor supplies player spawn transforms, mug spawn locations, arena center, and arena half-size for the out-of-arena respawn failsafe.

If no authored arena is present and `bSpawnPrototypeArena` is enabled, the game mode spawns `RuntimePrototypeArenaClass` as a fallback. By default, that class is also `AWizardStaffPrototypeArena`, so empty test maps still get the same placeholder layout. If the class is unset, the old legacy cube-spawned blocks remain as a final fallback.

It contains:

- A simple floor.
- Outer wall blocks.
- A doorway or narrow gap.
- Tables/blocks/obstacles for staff collision comedy.
- Mug spawn markers named `MugSpawn_*`.
- Player spawn markers named `PlayerSpawn_*`.
- `ArenaHalfSize` for respawn bounds.

The intended editor workflow is to create a Blueprint subclass of `WizardStaffPrototypeArena`, place it in a test level, then move/scale the placeholder components and arrow markers by hand. Spawn markers are discovered by component name prefix, so additional Blueprint arrow components named with `PlayerSpawn` or `MugSpawn` prefixes are picked up by the game mode.

## Staffs At Dawn Arena

Staffs at Dawn has a separate placeholder arena workflow through `AWizardStaffStaffsAtDawnArena`.

`AWizardStaffGameMode` searches for a placed `AWizardStaffStaffsAtDawnArena` at BeginPlay when `bUseAuthoredStaffsAtDawnArena` is enabled. If one is found, Staffs at Dawn uses that actor for player spawn transforms, arena center, and arena half-size. If no authored arena is present and `bSpawnStaffsAtDawnArena` is enabled, the game mode spawns `RuntimeStaffsAtDawnArenaClass` at `RuntimeStaffsAtDawnArenaLocation`. If no Staffs at Dawn arena exists, the Trial falls back to the Mug Run/prototype arena.

It contains:

- A larger open central combat platform.
- Four exposed but wider risky bridge lanes.
- Larger outer duel pads near ring-out edges.
- A few low walls and pillars for staff collision and Arcane Pinball bounce surfaces.
- A small number of staff-catching props.
- Safer central player spawn markers named `PlayerSpawn_*`.
- Placeholder powerup markers named `FuturePowerupSpawn_*`, used by the Staffs at Dawn powerup framework and placed on real platform/pad surfaces.
- `ArenaHalfSize` for Staffs at Dawn out-of-arena respawn bounds.
- `RingOutFallDistanceBelowArena` for treating lower ground or landscape under the arena as out-of-play.

The intended editor workflow is to create a Blueprint subclass of `WizardStaffStaffsAtDawnArena`, place it in a test level, then move/scale the placeholder components and arrow markers by hand. Spawn markers are discovered by component name prefix, so additional Blueprint arrow components named with `PlayerSpawn` or `FuturePowerupSpawn` prefixes are picked up by the arena actor.

Ring-outs use the existing out-of-arena failsafe. During Staffs at Dawn countdown, active play, and results, the active bounds center and half-size come from the Staffs at Dawn arena when available. A wizard knocked beyond that horizontal bounds plus `OutOfArenaRespawnTuning.HorizontalOutOfBoundsPadding`, below `OutOfArenaRespawnTuning.FallZThreshold`, or below the Staffs at Dawn arena's relative ring-out fall line is scheduled for the normal delayed respawn. The current default `RingOutFallDistanceBelowArena` is `120`, which makes lower landscape or ground under the raised arena behave like non-playable void without allowing lower-floor jumping to repeatedly delay respawns.

For arena tuning, enable `FWizardStaffsAtDawnTuning.bShowArenaDebug`. The debug message printed at Staffs at Dawn start reports the arena source used, player spawn count, arena half-size, respawn/ring-out half-size, effective fall-out Z, future powerup marker count, and active powerup spawn slots. Optional debug draws show cyan arena bounds, orange respawn/ring-out bounds, green player spawn arrows, and purple powerup markers. These helpers are intended for tuning ring-out risk, fair spawns, camera framing, powerup placement, and staff-combat readability.

See `Docs/PrototypeArenaWorkflow.md` for the exact editing and fallback rules.

## Shared Camera

`AWizardStaffSharedCamera` is a shared angled camera for local multiplayer.

It tracks all active local player pawns, computes bounds, follows the center smoothly, and zooms out as players spread apart. It can include staff height in zoom calculations so very tall staffs remain more readable. It avoids camera shake and strong wobble.

Important tuning:

- `TargetOffset`
- `CameraPitchDegrees`
- `MinArmLength`
- `MaxArmLength`, default `3200.0` for the wider Staffs at Dawn arena and tall staffs
- `PlayerBoundsPadding`
- `RadiusToArmLengthScale`
- `MinimumTrackedRadius`
- `bIncludeStaffHeightInZoom`
- `StaffHeightToArmLengthScale`
- `FollowLerpSpeed`
- `ZoomLerpSpeed`

The current camera is prototype-friendly and functional. It is not yet a cinematic or final party-game camera.

## HUD And Debug Output

`AWizardStaffHUD` draws a simple shared Canvas HUD. It is intentionally ugly and functional. It is not UMG yet. It is drawn as one shared HUD, which matches the no-splitscreen setup.

The HUD now has display modes:

- `Playtest`: default mode for normal local playtests. It keeps the middle of the screen mostly clear and shows party/trial state, Trial name, timer, active tuning preset, compact player rows, Staffs at Dawn score when relevant, staff count, compact Mana Slosh and Staff Stress meters, Grand Wizard Favor, round wins, carried brew reward only when one is held, Party Hall ready status, Final Candidate status, and small event messages.
- `FullDebug`: tuning/diagnostic mode. It preserves the detailed Canvas display with Mana Slosh bars, Staff Stress bars, larger standings/results panels, telemetry summary detail, and developer-facing feedback.
- `Minimal`: focus mode. It shows only state, Trial/timer, and the most important leader/Candidate/winner text.
- `Hidden`: hides the custom Canvas HUD. Engine-level debug messages can still appear.

Use `H` or the `CycleWizardHudMode` exec/debug function during PIE to cycle `Playtest -> FullDebug -> Minimal -> Hidden -> Playtest`.

`Playtest` mode intentionally hides movement percentages, turn/accel/brake/react/heft diagnostic text, large diagnostic stress/slosh bars, duplicate state lines, and the large telemetry panel. It still shows compact Mana Slosh and Staff Stress meters so players can see when they are sloshed or close to snapping. Switch to `FullDebug` when tuning systems or reading detailed post-match telemetry.

Gameplay feedback now routes through a small HUD message feed instead of unrestricted engine debug messages. The feed appears near the lower-left edge, keeps the middle of the screen clear, merges repeated duplicate messages, and shows only the most recent 3 messages in `Playtest` mode or 5 in `FullDebug`.

The feed is used for routine playtest events such as Favor gains, Ready Bell state, Staffs at Dawn scoring, ring-out scoring, Staffs at Dawn staff growth, Staff Clash start/result messages, Arcane Pinball reward/cast/hit/self-hit messages, Mega Staff Brew pickup/fade/expire/snap messages, Candidate changes, Final winner messages, tuning preset changes, and staff snaps.

Direct `GEngine->AddOnScreenDebugMessage` output should be reserved for `FullDebug` diagnostics, explicit debug booleans, or critical warning/error cases. Normal gameplay feedback should use the HUD feed so PIE sessions stay readable.

## Local Playtest Bots

`UWizardStaffPlaytestBotComponent` is a prototype-only local playtest helper. It is not final AI, not a single-player mode, and not online support. Its job is to create enough movement, targets, bonk pressure, pickups, ring-out attempts, and Final Round contesting to test the party loop when a second human is not available.

Bots occupy normal local player slots so existing camera, scoring, Favor, telemetry, Ready Bell, respawn, and rematch systems keep working. `AWizardStaffGameMode` owns the main toggles:

- `bEnablePlaytestBots`
- `bFillMissingPlayersWithBots`
- `DesiredHumanPlayers`
- `DesiredTotalPlayers`
- `bShowPlaytestBotDebug`
- `FWizardPlaytestBotTuning`

Typical solo test setup: set `bEnablePlaytestBots = true`, `bFillMissingPlayersWithBots = true`, `DesiredHumanPlayers = 1`, and `DesiredTotalPlayers = 2`. The second local player slot becomes a bot-driven wizard. HUD player rows show a small `BOT` label. `SetPlaytestBotsEnabled` and `TogglePlaytestBots` are exec-friendly helpers for PIE.

Current default solo setup has bots enabled with one human and one filled bot, so pressing Play in PIE should create P1 as the human and P2 as a `BOT`. `bAutoEnableSoloPlaytestBotInPIE` forces the solo bot defaults at PIE startup before local players are created, which protects against stale map or Blueprint GameMode defaults that still have bots disabled. Turn that flag off only when you intentionally want PIE to respect the manual bot settings.

Bot assignment now uses local player-controller order first, with `PlayerState` IDs only as a fallback. This matters in PIE because `PlayerState->GetPlayerId()` is not always the same as the local player slot. When the assignment succeeds, the log prints `WizardStaff assigned P2 as a playtest bot.`

Bot behavior is intentionally simple:

- Party Hall: wander, bonk lightly, and ring the Ready Bell after a randomized delay.
- Mug Run: prefer active mugs so pickup, staff growth, Slosh, and Arcane Pinball reward grants get exercised; still occasionally bonk nearby players and sometimes cast Arcane Pinball if holding it.
- Staffs at Dawn: move toward opponents, contest nearby Mega Staff Brew pickups, avoid edges when possible, bonk in range, and try broom boost while falling or near ring-out danger.
- Grand Wizard Final: Candidates try to stay near the ritual circle; challengers chase the Candidate unless the Candidate is vulnerable, then they move toward the circle to steal.

The one-human-versus-one-bot loop is now a valid normal iteration path for quick solo playtests. It is useful for checking that the party flow can complete, that systems reset, and that there is always another wizard moving around to collide with, bonk, steal from, or get launched by. The bot is useful enough for pickup coverage, Ready Bell pacing, ring-out targets, Mega Staff chaos, broom recovery attempts, rematch cleanup, and Grand Wizard Final pressure.

The bot is still bad at long-term strategy, precise aiming, smart edge positioning, deliberate Favor planning, and real combat reads. It does not use Behavior Trees, navmesh, or polished pathfinding. If it stalls, it retargets or reverses briefly. Some bad decisions are acceptable and desirable because goofy incompetence is useful for stress testing the physical comedy loop.

Important tuning:

- `BotThinkInterval`
- `BotMoveAggression`
- `BotTurnAggression`
- `BotBonkChance`
- `BotStaffClashMashChance`
- `BotBonkRangePadding`
- `BotJumpChance`
- `BotBroomBoostRecoveryChance`
- `BotRewardUseChance`
- `BotReadyBellDelayMin`
- `BotReadyBellDelayMax`
- `BotTargetRefreshTime`
- `BotStuckTimeBeforeRetarget`

Use `FullDebug` plus `bShowPlaytestBotDebug` to see simple bot target lines and state labels. Keep normal Playtest HUD mode clean; bot routine decisions should not spam the message feed.

## Playtest Telemetry And Match Summary

`AWizardStaffGameMode` tracks lightweight local-only playtest stats for the current party match. Stats reset when a full rematch starts.

Tracked per player:

- Mugs collected.
- Staff segments gained.
- Staff segments snapped off.
- Bonks attempted.
- Bonks landed.
- Times bonked by another player.
- Staff Clash starts.
- Staff Clash wins.
- Staff Clash ties.
- Staff Clash ring-outs caused.
- Out-of-arena respawns.
- Arcane Pinball rewards received.
- Arcane Pinballs cast.
- Arcane Pinball hits on other players.
- Arcane Pinball self-hits.
- Arcane Pinball total bounces.
- Staff stress gained from casting Arcane Pinball.
- Staff stress gained from being hit by Arcane Pinball.
- Staffs at Dawn score.
- Staffs at Dawn combat staff segments gained.
- Staffs at Dawn credited ring-outs caused.
- Mega Staff pickups collected.
- Mega Staff segments granted.
- Mega Staff temporary segments snapped during the effect.
- Mega Staff ring-outs caused.
- Loose snapped segment chaos hits.
- Mana Slosh added by loose snapped segment effects.
- Loose snapped segment TripBonk triggers.
- Loose snapped segment ArcanePop triggers.
- Broom boosts used during Staffs at Dawn.
- Broom boost ring-out saves during Staffs at Dawn.
- Broom boost attempts that still ended in an out-of-arena respawn during Staffs at Dawn.
- Grand Wizard Favor earned.
- Time spent as Grand Wizard Candidate.
- Final staff segment count.
- Round wins.
- Whether that player won the final match.

The Canvas HUD shows a simple match summary after the Grand Wizard Final winner is declared, including total Staffs at Dawn credited ring-outs, Staffs at Dawn rounds completed, average Staffs at Dawn respawns per round, compact Staff Clash `S/W/T/RO` stats, a `Broom U/S/F` line for Staffs at Dawn broom boosts used/saves/failed recoveries, a compact Mega Staff pickup/segment/snap/ring-out line, a concise Arcane Pinball line for reward/cast counts, hit/self-hit counts, bounces, and cast/hit stress, plus a compact loose segment chaos line in FullDebug mode. Debug log output is controlled by the telemetry debug flag. The prototype does not write telemetry files to disk yet.

## Prototype Tuning Presets

`AWizardStaffGameMode` owns three editable prototype tuning presets:

- `StableTuningPreset`
- `ChaoticTuningPreset`
- `AbsurdTuningPreset`

The active preset is `ActivePrototypeTuningPreset`. All three presets currently feel fun after the latest validation, with `Chaotic` still serving as the safer default for normal iteration.

The internal enum and tuning variable still use `Absurd`/`AbsurdTuningPreset`, but player-facing HUD/debug text displays that preset as `Arcane Catastrophe`.

When a preset is applied, it pushes values into the existing manual tuning structs instead of replacing them. It affects:

- Staff collision length, thickness, obstruction control, and recovery.
- Staff stuck stress, collision relief, and gentle nudge values.
- Staff stress, snap threshold, snap impulse, stress gain, and decay.
- Out-of-arena respawn delay, padding, and fall threshold.
- Mana Slosh gain, decay, penalties, oversteer, stumble chance, and visual wobble.
- Staff Heft movement, turning, and bonk timing penalties.
- Bonk knockback, cooldown, strike duration, contact padding, and stress multipliers.
- Mug Run match duration, mug count, mug respawn delay, brew reward chance, and loose snapped segment budget.
- Arcane Pinball projectile speed, bounce speed ramp, max speed, bounce count, lifetime, knockback, slosh, stress, self-hit, and destroy-on-player-hit behavior.

`Stable` is forgiving and easier to control, with fewer calmer brew spells, while still having meaningful staff bonks, enough stress pressure to avoid flat ties, and enough Mana Slosh gain to make greedy staff growth noticeable. `Chaotic` is the main playtest profile. `Arcane Catastrophe` is intentionally silly, remains the favorite high-energy chaos preset, and is useful for stress testing big slosh, stronger launches, higher loose-segment chaos, and more intense spell chaos.

Press `T` during PIE to cycle `Stable -> Chaotic -> Arcane Catastrophe`. The game mode also exposes console-friendly functions:

- `CyclePrototypeTuningPreset`
- `SetPrototypeTuningPresetByName Stable`
- `SetPrototypeTuningPresetByName Chaotic`
- `SetPrototypeTuningPresetByName Arcane Catastrophe`
- `SetPrototypeTuningPresetByName Absurd`

The active preset is printed on screen when changed and appears in the green prototype debug state line.

## Current Controls

Keyboard and mouse are mainly for player 1. Gamepads are the intended way to test multiple local players. `bOffsetPlayerGamepadIds=True` is enabled so one keyboard plus one gamepad should map to player 1 and player 2 instead of both fighting over player 1.

`AWizardStaffGameMode` also has `KeyboardFallbackControls` for controller-free two-player PIE testing. By default it drives player 2 from the primary keyboard and can be disabled or rebound in the game mode defaults.

| Action | Keyboard / Mouse | Gamepad |
| --- | --- | --- |
| Move | `WASD` or arrow keys | Left stick |
| Turn | `Q` / `E` or mouse X | Right stick X |
| Hop | `Space` | Face button bottom |
| Drink debug mug | `R` | Face button right |
| Quick Bonk | `F` or left mouse | Right shoulder |
| Add staff segment | `Z` | none |
| Remove staff segment | `X` | none |
| Reset slosh | `C` | none |
| Max staff stress | `B` | none |
| Force snap top segment | `N` | none |
| Restart Mug Run match | `M` | none |
| Cycle tuning preset | `T` | none |
| Cycle HUD mode | `H` | none |
| Set low/medium/high/absurd slosh | `Shift+1` / `Shift+2` / `Shift+3` / `Shift+4` | none |

Player 2 keyboard fallback:

| Action | Keyboard |
| --- | --- |
| Move | `I` / `K` / `J` / `L` |
| Turn | `U` / `O` |
| Hop | Right Shift |
| Quick Bonk | Right Control |
| Drink debug mug | `P` |

If a PS4 controller still sends no input at all on Windows, use Steam Input or DS4Windows to expose it as an XInput-style controller for the current prototype input mappings.

## How To Playtest The Current Loop

1. Open `WizardStaff.uproject`.
2. Start Play-In-Editor with the default game mode.
3. Confirm players start in the Party Hall, using a placed `WizardStaffPartyHall` if present or the runtime fallback if not.
4. Use keyboard/mouse for player 1 and gamepads for extra local players.
5. Run around and bonk lightly in the Party Hall while the intermission timer counts down.
6. Confirm the game mode uses the placed `WizardStaffPrototypeArena`, or spawns the runtime fallback if the map has no arena actor.
7. During countdown, confirm players move from the Party Hall into the Mug Run arena.
8. Collect mugs and confirm Mana Slosh, staff segment count, possible brew reward, and HUD values update.
9. Grow a large staff through mugs or the `Z` debug key.
10. Move through the doorway and around blocks to test obstruction and stuck behavior.
11. Bonk another player with `F`, left mouse, or right shoulder.
12. Confirm bonks require staff contact during the downward strike.
13. Watch staff stress rise from bonks and wall impacts.
14. Force snapping with `B` or `N`, or build stress naturally.
15. Let snapped physics segments hit players and confirm the comedy is still present.
16. If a player is launched out of the play area, confirm they respawn after roughly 1.5 seconds.
17. Let the Mug Run timer expire and confirm the winner message matches the tallest staff.
18. Confirm Results counts down, then players return to the Party Hall with standings visible.
19. Confirm the next Party Hall countdown names Staffs at Dawn as the next Trial.
20. During Staffs at Dawn, confirm players spawn in the Staffs at Dawn combat arena when one is present, or safely fall back to the prototype arena if not.
21. Confirm Mug Run pickups are inactive during Staffs at Dawn.
22. Land staff bonks and confirm Staffs at Dawn score increases.
23. Bonk a player off an exposed edge and confirm the recent attacker receives the ring-out score bonus after the victim respawns.
24. Let the Staffs at Dawn timer expire and confirm Results shows the Staffs at Dawn winner or tie.
25. Confirm Results counts down, then the Grand Wizard Final begins.
26. Confirm the Grand Wizard Final starts, the current highest-staff player is placed in the ritual circle, and the HUD labels them as Candidate.
27. Bonk the Candidate out of or away from the circle, then have another player stand in the circle until the steal progress completes.
28. Let the final timer expire and confirm the current Candidate becomes the Grand Wizard winner.
29. Confirm the winner and playtest match summary are shown, then the rematch returns to Party Hall without restarting PIE.
30. Press `M` during a match, countdown, results, or Party Hall to restart the party loop without restarting PIE.
31. Press `T` to cycle Stable, Chaotic, and Arcane Catastrophe tuning presets and confirm the on-screen preset message appears.

## One Human Versus One Bot Checklist

Use this when a second local player is not available.

1. Enable `bEnablePlaytestBots`, keep `bFillMissingPlayersWithBots` enabled, set `DesiredHumanPlayers = 1`, and set `DesiredTotalPlayers = 2`.
2. Start PIE and confirm P2 appears with a `BOT` label in the HUD.
3. In Party Hall, confirm the bot moves around, bonks occasionally, and rings the Ready Bell after a short delay instead of instantly.
4. In Mug Run, confirm the bot chases mugs, sometimes bonks, and occasionally fires Arcane Pinball if it receives the reward.
5. In Staffs at Dawn, confirm the bot approaches the fight, contests nearby Mega Staff Brew pickups, can be ringed out, and sometimes attempts broom recovery.
6. In the Grand Wizard Final, confirm a bot Candidate tries to stay near the circle, while a bot challenger pressures the Candidate or tries to steal when the Candidate is vulnerable.
7. Run through automatic rematch and confirm the bot remains labeled, active, and reset with the rest of the party loop.
8. If diagnosing target choices, switch to `FullDebug` and enable `bShowPlaytestBotDebug`; otherwise keep normal playtests in `Playtest` HUD mode.

## 10-15 Minute Local Loop Validation

Use this shorter pass before adding more content. The goal is to prove the current two-Trial party loop stays fun, readable, and stable across repeated back-to-back cycles.

1. Play at least three complete party cycles in one PIE session: Party Hall, Mug Run, Results, Party Hall, Staffs at Dawn, Results, Grand Wizard Final, match summary, automatic rematch.
2. Use the Ready Bell in each Party Hall and confirm it shortens waiting without feeling like a menu.
3. In Mug Run, collect enough mugs to see staff growth, Mana Slosh, Arcane Pinball rewards, staff stress, and at least one staff snap.
4. Confirm carried Arcane Pinball rewards and already-fired Arcane Pinball projectiles are cleared after Mug Run and do not appear in Staffs at Dawn or the Final.
5. In Staffs at Dawn, confirm mugs stay inactive, bonks score, ring-outs score, combat staff growth happens, and ringed-out players respawn with physical staff segments reset.
6. Pick up Mega Staff Brew when it appears and confirm it is obvious, risky, temporary, and cleaned up when Staffs at Dawn ends.
7. Use broom boost during at least one ring-out attempt and check the match summary's `Broom U/S/F` line for boosts, saves, and failed recoveries.
8. Confirm normal Trials start with fresh physical staff state, while the Grand Wizard Final rebuilds staffs from Favor and applies Mana Slosh from the rebuilt segment count.
9. Watch HUD feedback for spam: Favor gains, Staffs at Dawn scoring, Mega Staff messages, Candidate swap text, and telemetry summary should be readable without covering play.
10. After automatic rematch, confirm Favor, round wins, Trial scores, carried rewards, powerups, temporary Mega Staff state, broom tracking, and loose-segment cleanup behave like a fresh party match.

The current two-Trial vertical slice takes about 3:30 for a full Party Hall, Mug Run, Results, Party Hall, Staffs at Dawn, Results, Grand Wizard Final, match summary, and automatic rematch loop. Keep using repeated short sessions to regression-test resets, HUD readability, and the two-Trial rhythm. The completed-game target is a 12-total-Trial roster, but the intended default match is about a 10-minute session built around three Trials plus the Grand Wizard Final. Future content should be paced carefully instead of assuming every new Trial can add another long round. Do not judge 20-30 minute retention yet; longer-session retention should wait until the current loop survives several shorter sessions without stale state, confusing messages, or repeated annoyances.

## Known Risks And Rough Edges

- The staff uses one attached box collision, not a true multi-body staff.
- Staff obstruction is approximate and depends on overlap/contact behavior rather than real joint physics.
- Snapped segment physics are intentionally chaotic and can create extreme launches.
- Out-of-arena respawn is a safety net, not a full arena boundary solution.
- The authored arena workflow is still placeholder components and Blueprint placement, not final level art.
- Placeholder mesh sockets are not validated against real future mug meshes.
- Loose snapped physics actors are budgeted by lifetime and max count, but the exact values still need long-session playtesting.
- Input is still legacy action/axis binding even though the project uses Enhanced Input classes in config.
- Keyboard mostly controls player 1; reliable 2-4 player testing needs gamepads.
- HUD is Canvas debug UI, not final UMG.
- Party Hall is a placeholder playable room, not a designed social hub yet.
- Staffs at Dawn now has a purpose-built placeholder combat arena, but its bridge widths, exposed edges, spawn positions, and prop spacing need playtesting.
- Staffs at Dawn ring-out credit depends on a recent-bonk time window, so weird physics sequences may occasionally credit or miss an attacker imperfectly.
- Staffs at Dawn temporary powerup state is explicitly cleaned up when the Trial ends, but Mega Staff timing and readability still need repeated short-session playtests.
- Staffs at Dawn does not yet score snapping another player's staff segment because snap attribution is not reliable.
- Grand Wizard Final uses simple distance/hold checks rather than authored objectives or bespoke final-round animation.
- There is no automated gameplay test coverage yet.

## What Should Be Tuned Next

The next work should be hands-on tuning rather than new systems.

Highest value tuning areas:

- Staff collision size, obstruction strength, and recovery.
- Staff stuck stress timing and collision relief.
- Staff stress gain from bonks, wall impacts, and being caught.
- Snap impulse, especially with the accepted funny physics behavior.
- Out-of-arena respawn bounds and delay.
- Authored arena prop placement, doorway width, and spawn marker positions.
- Staffs at Dawn bridge widths, duel pad sizes, exposed edge risk, pillar/low-wall placement, and spawn spacing.
- Mana Slosh gained per staff segment, plus Slosh penalties, oversteer, and visual wobble.
- Staff Heft thresholds and bonk slowdown for huge staffs.
- Staffs at Dawn ring-out staff reset: watch whether clearing the victim's physical staff makes ring-outs feel rewarding without making comebacks too punishing.
- Broom boost duration, forward speed, upward safety, Slosh interaction, Staff Heft interaction, high-Slosh disorder, and recovery readability so it helps ring-out recovery without becoming full flight.
- Broom boost save/failure telemetry in Staffs at Dawn: if `BroomBoostRingOutSaves` climbs close to total ring-outs, recovery is probably too strong; if `BroomBoostFailedRingOutRecoveries` dominates, recovery is probably too weak or unreadable.
- Bonk end-impact timing, contact padding, strike timing, and knockback caps.
- Shared camera min/max distance and staff-height zoom.
- Mug count, respawn delay, and match duration.
- Arcane Pinball brew reward chance, projectile speed ramp, hit knockback, self-hit frustration, and whether it overshadows staff bonking.
- Staffs at Dawn duration, bonk points, ring-out points, and ring-out credit window.
- Staffs at Dawn landed-bonks-per-staff-segment and ring-out staff segment rewards.
- Whether Staffs at Dawn should keep mugs fully disabled or allow rare chaos mugs later.
- Trial countdown/results duration and whether staffs/slosh reset between trials.
- Final Round duration, circle radius, candidate near-circle padding, and steal hold duration.
- Grand Wizard Favor rewards for Trial wins, tied wins, and Staffs at Dawn ring-outs.
- Favor-based Final staff setup values for Candidate and Challenger starting segments.
- Staffs at Dawn powerup spawn count, initial spawn delay, respawn delay, and whether pickups respawn after collection.
- Loose segment lifetime, max loose segment count, rematch cleanup, and fade duration.
- Stable, Chaotic, and Arcane Catastrophe preset values after repeated playtests.
- Long-term session pacing toward the completed target of a 12-total-Trial roster and roughly 10-minute default matches built around three Trials plus the Grand Wizard Final.

## What Should Not Be Expanded Yet

Avoid expanding into larger features until the physical comedy loop is tuned.

Do not prioritize yet:

- Online multiplayer.
- Replication.
- A full spell system, spell trees, mana-cost combat rotations, inventories, loadouts, or many new spells.
- One-off brew reward spells are allowed only when they directly support the physical comedy loop.
- Mug Run brew rewards should remain simple, readable, and easy to understand while players are running, bonking, and managing awkward staffs.
- Arcane Pinball is currently the only real brew reward spell and should be tuned in repeated local playtests before another brew spell is added.
- Staffs at Dawn powerups beyond Mega Staff Brew. Do not add Hammer Time, multiple combat powerups, or a powerup loadout system yet.
- Additional Trials or mini-games, especially a third Trial before the `Next Trial Gate` is passed.
- Permanent progression.
- Full-body ragdoll movement.
- Full physics-chain staff simulation.
- Final art, animation, sound, or UI.
- Large map/content production.

The prototype is currently in the right place for repeated local playtests, tuning, and small safety fixes.
