# Wizard's Staff Prototype Audit

Scope: first playable audit before adding more gameplay content.

Current overview: see `Docs/CurrentPrototypeStatus.md` for the latest descriptive status of the prototype after later camera, slosh, bonk, hit reaction, staff heft, and out-of-arena respawn passes.

Verification: `WizardStaffEditor Win64 Development` builds successfully. Manual PIE playtest is still required for feel, controller routing, and physics stability.

## Systems In Place

- `AWizardStaffGameMode` creates 2-4 local players, uses a placed prototype arena or runtime fallback, runs the party/trial state flow, starts Mug Run as the first Trial, tracks the match timer, spawns mugs, declares the winner by staff segment count, handles trial results/auto-advance, and handles out-of-arena respawn.
- `AWizardStaffPrototypeArena` provides the editable placeholder arena layout, player spawn markers, mug spawn markers, and arena bounds for respawn safety.
- `AWizardStaffWizardCharacter` handles placeholder wizard visuals, local movement, hop, Mana Slosh, contact-driven quick bonk, hit reactions, staff heft, debug commands, and player color.
- `UWizardStaffComponent` handles base staff visuals, stacked staff segments, fallback socket spacing, one hybrid staff collision box, stress, snapping, and snapped physics segment spawning.
- `AWizardStaffManaMugPickup` handles placeholder mug visuals, overlap collection, drink calls, and timed respawn.
- `AWizardStaffSharedCamera` follows all local player pawns with a shared angled camera.
- `AWizardStaffHUD` draws one shared debug HUD with timer, staff count, Mana Slosh, stress, and winner text.
- Tuning is exposed through small Blueprint-friendly structs on the character, staff component, game mode, and pickup actor.

## Playtest Checklist

1. Open `WizardStaff.uproject`.
2. Start Play-In-Editor with the default game mode.
3. Confirm 2 local wizard pawns spawn in the authored arena, or in the runtime fallback if no arena actor is placed, and appear in different colors.
4. Move player 1 with keyboard: `WASD`, `Q/E` turn, `Space` hop.
5. Use gamepads for extra local players and confirm each pawn can move independently.
6. Collect mugs and confirm Mana Slosh increases, staff segment count increases, possible brew rewards appear, and the HUD updates.
7. Press `F` or left mouse to quick bonk and confirm other players receive knockback.
8. Grow a staff with mugs or debug key `Z`, then move through the doorway and around blocks to confirm staff obstruction is noticeable but does not launch the player.
9. Build stress by bonking and rubbing the staff into walls. Confirm stress appears on the HUD/debug text.
10. Force edge cases with debug keys: `X` remove segment, `C` reset Mana Slosh, `B` max staff stress, `N` force snap.
11. Let the timer expire and confirm the winner message matches the tallest staff.
12. Confirm Trial Results counts down, then starts a new Mug Run Trial countdown without restarting PIE. Press `M` to restart the party loop early.

## Known Risks

- The staff collision is one attached kinematic box, not a true rigid-body staff. This is stable for the prototype but can feel approximate.
- Obstruction stress uses overlap and movement speed of the attached collision box, not real contact impulse. It may need tuning after hands-on play.
- The authored arena workflow is now a Blueprintable actor with placeholder components. A committed hand-authored `.umap` may still be useful once level layout needs to stabilize.
- Staff segment sockets are only a convention right now. Placeholder meshes use calculated height fallback, so real mug meshes need top/bottom socket validation.
- Snapped segments spawn as physics actors and are now tracked by a lifetime/max-count chaos budget, but the values still need long-session tuning.
- Input uses legacy action/axis bindings while the project config points at Enhanced Input classes. This is acceptable for the prototype, but input should be standardized before many more actions are added.
- Keyboard testing now includes a player-2 fallback through `AWizardStaffGameMode::KeyboardFallbackControls`. Reliable 3-4 player testing still needs gamepads.
- Prototype tuning presets now apply grouped feel profiles from `AWizardStaffGameMode`; individual manual tuning values still exist but can be overridden when a preset is applied.
- HUD is Canvas debug UI drawn from the first local player only. It is functional but not layout-safe for final UI.
- Party/trial flow is prototype-only and auto-advances after results instead of using Party Hall UI.

## Tune Next

- Staff feel: `CollisionTuning.CollisionLengthPerSegment`, `CollisionThickness`, `ObstructedControlMultiplier`, and `ObstructionRecoverySpeed`.
- Stress feel: `StressGainedPerBonk`, `StressGainedPerWallImpact`, `CaughtStressPerSecond`, `StressMultiplierPerSegment`, `MaxStaffStress`, and `SnapImpulseForce`.
- Mana Slosh feel: `SloshGainedPerStaffSegment`, legacy `SloshGainedPerDrink` compatibility mirroring, `MovementPenaltyPerSlosh`, `TurnPenaltyPerSlosh`, and `SloshDecayPerSecond`.
- Bonk feel: `StaffContactPadding`, `VisualDuration`, `StrikeStartPitchDegrees`, `StrikeEndPitchDegrees`, `StrikeSideWobbleDegrees`, `KnockbackStrength`, `KnockbackPerStaffSegment`, `Cooldown`, and staff heft bonk timing bonuses.
- Out-of-arena safety: `RespawnDelay`, `HorizontalOutOfBoundsPadding`, `FallZThreshold`, and `bCancelRespawnIfPlayerReturns`.
- Mug Run pacing: `MatchDuration`, `MugSpawnCount`, pickup `RespawnDelay`, and authored `MugSpawn_*` positions.
- Arena layout: doorway width, table/block placement, `PlayerSpawn_*` positions, and `ArenaHalfSize`.
- Party/trial flow: `TrialCountdownDuration`, `TrialResultsDisplayDuration`, `TrialsBeforeFinalRound`, `bResetStaffsBetweenMatches`, and `bResetSloshBetweenMatches`.
- Loose segment budget: `LooseSegmentLifetime`, `MaxLooseSegments`, `bCleanupLooseSegmentsOnRematch`, and `FadeOutDuration`.
- Camera readability: shared camera arm length, padding, and zoom speed with 2, 3, and 4 players.
- Preset balance: compare `StableTuningPreset`, `ChaoticTuningPreset`, and `AbsurdTuningPreset` (displayed as Arcane Catastrophe) without expanding the game loop yet.

## Do Not Expand Yet

- Do not add online multiplayer or replication.
- Do not add spells, charged bonks, teams, progression, inventory, or menus.
- Do not replace the hybrid staff with a full physics chain until the current stress/snap loop is tuned.
- Do not add final art assets before the staff length, collision, Mana Slosh, and stress numbers feel playable.
- Do not add more pickup types until plain mugs create a reliable match arc.

## Small Audit Fixes Made

- Staff obstruction now filters overlaps to components that actually block a `WorldDynamic` staff, so non-blocking query actors such as mug pickups should not count as wall impacts.
- `ObstructionRecoverySpeed` no longer has an editor max clamp of `1.0`; its default is `8.0` and it is intended to work as interpolation speed.
