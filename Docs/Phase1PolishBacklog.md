# Wizard's Staff Phase 1 Polish Backlog

Last updated: July 2, 2026

Scope: this backlog is for protecting the locked vertical-slice baseline before adding a third Trial, another spell, another Staffs at Dawn powerup, Hammer Time, final art, progression, shops, inventory, online multiplayer, or broad new systems.

This is a planning document. It should guide small bug fixes, code-level validation, tester preparation, and local multiplayer polish. It should not be used as permission to expand gameplay content.

## Locked Baseline

The current locked slice is:

1. Party Hall.
2. Mug Run.
3. Staffs at Dawn.
4. Grand Wizard Final.
5. Winner/rematch loop.
6. Stable, Chaotic, and Arcane Catastrophe presets.
7. Arcane Pinball.
8. Mega Staff Brew.
9. Staff Clash.
10. Broom boost.
11. Loose snapped segment chaos.
12. Mana Slosh and staff snapping Slosh relief.
13. Playtest bots.
14. Canvas HUD `Playtest`, `FullDebug`, `Minimal`, and `Hidden` modes.

Current status: the gameplay loop feels great in this small slice and should be treated as the locked vertical-slice baseline for this phase. The goal of Phase 1 polish is to protect the fun, reduce rough edges, fix bugs, and prepare for more real local multiplayer feedback before moving toward the eventual 3-Trial default match format.

## Long-Term Trial Structure Direction

The completed game target is a roster of 12 total Trials. This does not mean every match should play all 12 Trials.

The intended default match structure is:

1. Trial.
2. Trial.
3. Trial.
4. Grand Wizard Final.
5. Winner/rematch.

The current vertical slice has:

1. Mug Run.
2. Staffs at Dawn.
3. Grand Wizard Final.
4. Winner/rematch.

Possible Steam Early Access target: at least 6 total polished Trials before publishing to Early Access, if Early Access becomes the release path.

Do not solve final Trial selection rules yet. For now, document this direction so future planning does not assume every match plays every Trial. Future expansion should move from the current 2-Trial slice toward a 3-Trial default match, while the 12-Trial goal remains a full roster/content target.

## Locked Baseline Protection Rules

- Do not add new Trials yet.
- Do not add new spells yet.
- Do not add new Staffs at Dawn powerups yet.
- Do not add Hammer Time yet.
- Do not redesign the Grand Wizard Final yet.
- Do not over-tune Arcane Catastrophe unless it becomes broken rather than funny.
- Do not spend time on final art, progression, shop, inventory, or online multiplayer yet.
- Do not design the game around all 12 Trials being played in one match.
- Future expansion should move from the current 2-Trial slice toward a 3-Trial default match.
- The 12-Trial goal is a full roster/content target, not the default match length.
- The possible Early Access milestone is at least 6 polished Trials, not the full 12-Trial roster.
- Codex should not be asked to judge feel, fun, pacing, or readability through fake human validation. Those notes should come from manual playtests.

## Codex Vs Human Validation

Human validation is for:

- Fun.
- Feel.
- Preset comparison.
- HUD readability while actually playing.
- Whether the Grand Wizard Final feels exciting.
- Whether the game makes players want another match.
- Whether Arcane Catastrophe is funny chaos or too much.
- Whether Staffs at Dawn ring-outs, broom saves, Staff Clash, Mega Staff Brew, and Arcane Pinball feel fair enough in real play.

Codex validation is for:

- Build health.
- UHT/compiler errors.
- Log warnings.
- Crash risks.
- Stale state.
- Cleanup leaks.
- Invalid transforms and NaN guards.
- Rematch reset safety.
- Projectile cleanup.
- Powerup cleanup.
- Bot cleanup.
- Staff Clash stuck-state safety.
- Code-level audits.

Future Codex prompts should use concrete human-observed issues instead of asking Codex to perform subjective feel passes. Good prompts are things like "players can still move during countdown," "Arcane Pinball leaked into Staffs at Dawn," "Mega Staff stayed active into Results," or "PIE log shows this warning."

## 1. Must Fix Before Sharing With Testers

These are tester-blocking issues. Fix them before sending the build to outside players.

- Build errors: any UHT, compiler, linker, or editor startup failure.
- Fatal PIE crashes, especially during BeginPlay, match transition, Trial start, Final start, rematch, or PIE shutdown.
- Serious log warnings that point to broken actor/component state, such as invalid attachment, mobility mismatch, repeated actor-name failure, invalid mesh assignment, or failed spawn loops.
- NaN or invalid transform warnings from wizard movement, staff visuals, Staff Clash, broom boost, loose segment physics, projectile velocity, or camera tracking.
- Rematch/reset stale state: Favor, round wins, Trial scores, candidate state, ready state, HUD winner text, countdown lock, carried rewards, Mega Staff state, Staff Clash state, broom state, and bot state must reset on the intended boundary.
- Trial transition stale state: Mug Run rewards and Arcane Pinball projectiles must not leak into Staffs at Dawn or the Final.
- Staffs at Dawn powerup cleanup: Mega Staff Brew pickup actors, timers, temporary segments, active effect state, and telemetry should not leak into Results, Final, Party Hall, or rematch.
- Loose snapped segment cleanup: loose segment actors should respect lifetime/max-count/rematch cleanup and should not accumulate indefinitely, crash when already destroyed, or dominate the arena after their active chaos window.
- Staff Clash stuck-state safety: players should never remain permanently input-locked, hit-immune, position-locked, or unable to bonk after a clash or Trial transition.
- Countdown false starts: players should remain staged and input-locked during Trial countdown, then unlock cleanly when the Trial becomes active.
- Staffs at Dawn ring-out bounds: falling to lower ground below the raised arena should schedule respawn promptly and should not be delayed indefinitely by running, jumping, or broom boosting on non-playable floor.
- Out-of-arena respawn: extreme launches must reliably return players without corrupting scores, staff state, Slosh, bot state, or camera tracking.
- Ready Bell reset: Party Hall ready states must clear every intermission and must not auto-ready players from previous halls.
- HUD obstruction: `Playtest` HUD mode should not cover the center play area, especially during Staffs at Dawn and Grand Wizard Final.
- Gamepad usability blocker: one keyboard player plus at least one controller should work reliably enough for local tester sessions.
- Bot-only support blocker: one human versus one playtest bot should complete the full loop when a second human is not available.
- Outside tester instructions: before sharing, prepare a short tester note explaining controls, presets, how to start PIE/build, what to report, and known rough edges.

## 2. Should Polish Soon

These are not necessarily blockers, but they are high-value before broader local multiplayer feedback.

- Rematch smoke test: repeatedly run Party Hall, Mug Run, Results, Party Hall, Staffs at Dawn, Results, Grand Wizard Final, winner, and automatic rematch without restarting PIE.
- Preset regression: confirm Stable, Chaotic, and Arcane Catastrophe all load the intended tuning values and preserve their identities.
- HUD message feed: keep routine gameplay feedback in the controlled feed and keep direct engine messages for `FullDebug` or critical warnings.
- HUD mode cycling: verify `Playtest`, `FullDebug`, `Minimal`, and `Hidden` can still be cycled and do not leave stale panels behind.
- Ready Bell clarity: make sure the bell is physically readable in Party Hall and the HUD ready labels are understandable.
- Party Hall clarity: keep standings, current leader, next Trial, active preset, and ready status readable without turning the hall into a menu.
- Mega Staff Brew readability: make pickup, active state, about-to-expire, expiration, and temporary segment cleanup obvious enough for testers.
- Arcane Pinball cleanup/readability: ensure receive/cast/hit/self-hit messages stay concise and projectiles are destroyed at state boundaries.
- Staff Clash readability: keep the message/pose/input moment clear, but do not let it overwhelm normal bonk combat.
- Staffs at Dawn ring-out telemetry: keep ring-out count, broom saves/failures, and respawn stats useful for judging arena danger.
- Loose segment chaos readability: keep Mana Splash and Trip Bonk occasional and readable; keep ArcanePop rare/disabled unless intentionally tested.
- Controller/gamepad usability: verify right-stick/key turning, Jump/broom, Quick Bonk, Use Reward, Ready Bell bonking, and HUD mode cycling do not fight each other.
- Bot labels: keep bot identity visible in player rows without clutter.
- Bot cleanup/reset: make sure bots reset target state, ready timing, reward use, broom state, Staff Clash state, and Trial behavior on rematch.
- Code-level log pass: run PIE and review logs after quitting for warnings introduced by runtime arenas, pickups, staff components, loose segments, and final circle visuals.
- Tester build preparation: decide the default preset for testers, likely Chaotic, and document how to switch to Stable or Arcane Catastrophe.

## 3. Nice-To-Have Polish

These are useful if they are small and do not risk destabilizing the baseline.

- Add or improve placeholder visual markers only where they improve gameplay readability. Keep the locked player facing marker as a single flat circle-with-point ground marker; do not bring back a separate arrow mesh unless repeated playtests prove the current marker fails.
- Add small debug labels for Trial state or arena source in `FullDebug` only.
- Add concise documentation for controller layout and player-2 keyboard fallback.
- Add a short "known funny bugs" note so testers understand which chaotic physics behaviors are intentional.
- Add a lightweight outside-tester feedback form/checklist.
- Add optional log categories or clearer UE_LOG prefixes for GameMode, Staff, HUD, Bot, Powerup, and Projectile systems.
- Add comments around fragile reset boundaries where future changes are likely to cause stale state.
- Improve authored arena editing notes for moving spawn points, powerup markers, and ring-out bounds.
- Improve placeholder sign placement in Party Hall if testers miss Ready Bell/standings.
- Add simple local debug commands for forcing Trial transitions only if they help code-level validation.
- Improve playtest bot target debug in `FullDebug`, as long as it does not affect normal play.

## 4. Do Not Touch Unless Repeated Playtests Prove A Problem

These systems are currently part of the fun. Avoid sanding them down after one odd moment.

- Arcane Catastrophe chaos level.
- Loose snapped segment physics comedy.
- Loose segment chaos effects, unless they become oppressive or invisible.
- Broom boost core feel, especially the balance of saveable near-misses and failed clean ring-outs.
- Staff snapping as a Slosh release valve.
- Mana Slosh's relationship to bonk power.
- Staff Clash trigger conditions and duration, unless clashes become constant or get players stuck.
- Grand Wizard Final circle/steal rules, unless human playtests repeatedly show the Final is too safe, too swingy, or unclear.
- Arcane Pinball self-hit risk.
- Mega Staff Brew's scary temporary-staff identity.
- Staffs at Dawn combat staff growth and ring-out reward values.
- Playtest bot mistakes, as long as they create useful pressure and do not break the loop.
- Canvas HUD architecture. Keep it until a real UI phase is deliberately chosen.

## 5. Explicitly Out Of Scope For This Phase

- Third Trial implementation.
- Additional brew reward spells.
- Additional Staffs at Dawn powerups.
- Hammer Time.
- Full spell system.
- Spell trees.
- Loadouts.
- Inventory.
- Permanent progression.
- Shops.
- Cosmetics.
- Final art.
- Full UMG conversion.
- Online multiplayer.
- Replication.
- Full-body ragdoll locomotion.
- New Grand Wizard Final mechanics.
- Final Trial selection rules.
- Designing around all 12 Trials in one default match.
- Steam Early Access production work beyond documenting the possible 6-Trial content target.

## Real Local Multiplayer Feedback Checklist

Use this after code-level smoke checks pass. Human playtest notes should drive feel changes.

1. Did players understand Party Hall, Ready Bell, Trial Results, Favor, Final Candidate, and final winner without explanation?
2. Did players ask for another match?
3. Which preset was most fun, and why?
4. Did Arcane Catastrophe feel funny or broken?
5. Did Staffs at Dawn ring-outs feel exciting without feeling automatic?
6. Did broom boost create clutch saves without erasing clean ring-outs?
7. Did Staff Clash feel occasional and funny rather than constant?
8. Did Mega Staff Brew feel powerful, risky, and readable?
9. Did Arcane Pinball add chaos without replacing staff bonking?
10. Did the HUD/message feed stay readable while playing?
11. Did controller players feel they had enough turning and input responsiveness?
12. Did rematch make players feel like the session could keep going?
13. Did any stale state, stuck state, missing input, or weird reset appear after rematch?

## Tester Sharing Readiness Checklist

Use this before handing the slice to outside testers.

1. Build succeeds.
2. PIE starts without fatal errors.
3. Quitting PIE does not produce alarming warnings.
4. One human plus one bot can complete the loop.
5. Two local humans can complete the loop.
6. At least one gamepad is recognized and usable.
7. Stable, Chaotic, and Arcane Catastrophe can be selected.
8. Playtest HUD mode is default.
9. FullDebug remains available for diagnosing issues.
10. Arcane Pinball, Mega Staff Brew, Staff Clash, broom boost, loose segment chaos, and rematch cleanup all survive a smoke pass.
11. Known rough edges are documented.
12. Testers know what feedback is useful and what is currently out of scope.
