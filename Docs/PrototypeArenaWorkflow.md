# Wizard's Staff Prototype Arena Workflow

Last updated: June 29, 2026

This workflow replaces the old runtime-only arena with editable prototype arena actors while keeping runtime spawning as a fallback.

## Mug Run Arena Actor

`AWizardStaffPrototypeArena` is a Blueprintable actor built from placeholder cube components and arrow spawn markers.

It currently includes:

- A flat open combat floor.
- Outer wall sections.
- One doorway or narrow gap with a lintel.
- Two table-like blocks.
- A few block props for staff obstruction.
- Four player spawn markers named `PlayerSpawn_01` through `PlayerSpawn_04`.
- Twelve mug spawn markers named `MugSpawn_01` through `MugSpawn_12`.
- `ArenaHalfSize`, used by the out-of-arena respawn failsafe.

The visuals are intentionally plain. The point is to make staff collision, doorway wedging, table clutter, and Mug Run pacing easy to tune by hand.

## Staffs At Dawn Arena Actor

`AWizardStaffStaffsAtDawnArena` is a separate Blueprintable actor for the Staffs at Dawn combat Trial. The default runtime layout is a larger raised island arena, roughly twice the original Staffs at Dawn footprint, so a single bonk is less likely to guarantee a ring-out.

It currently includes:

- A larger open central combat platform for safe-ish recovery.
- Four exposed but wider risky bridge lanes.
- Larger outer duel pads near ring-out edges.
- Low walls and pillars for staff collision and Arcane Pinball bounces.
- A small number of staff-catching props near bridge approaches.
- Four central player spawn markers named `PlayerSpawn_01` through `PlayerSpawn_04`.
- Staffs at Dawn powerup markers named `FuturePowerupSpawn_*`, currently used by the Mega Staff Brew pickup framework and placed on real platform/pad surfaces.
- `ArenaHalfSize`, used by the out-of-arena respawn failsafe during Staffs at Dawn.
- `RingOutFallDistanceBelowArena`, used to treat terrain or landscape below the raised arena as out-of-play.

The visuals are intentionally plain. The point is to make ring-out risk, staff-fighting space, bridge width, and spawn spacing easy to tune by hand without adding new Trial rules.

## How The Game Mode Uses It

`AWizardStaffGameMode` looks for a placed `AWizardStaffPrototypeArena` at BeginPlay when `bUseAuthoredPrototypeArena` is enabled.

If it finds one:

- Player spawn transforms come from `PlayerSpawn_*` arrow components.
- Mug spawn locations come from `MugSpawn_*` arrow components.
- Out-of-arena horizontal bounds are centered on the arena actor and use its `ArenaHalfSize`.
- The shared camera starts near the arena center before tracking players.

If it does not find one, and `bSpawnPrototypeArena` is enabled, the game mode spawns `RuntimePrototypeArenaClass`. By default this is the same `AWizardStaffPrototypeArena` class, so the fallback arena matches the authored layout.

If `RuntimePrototypeArenaClass` is unset, the game mode falls back to the older legacy cube-spawned arena blocks.

For Staffs at Dawn, `AWizardStaffGameMode` also looks for a placed `AWizardStaffStaffsAtDawnArena` when `bUseAuthoredStaffsAtDawnArena` is enabled.

If it finds one:

- Staffs at Dawn player spawn transforms come from that arena's `PlayerSpawn_*` arrow components.
- Staffs at Dawn out-of-arena horizontal bounds are centered on that arena actor and use its `ArenaHalfSize`.
- Mug Run still uses `AWizardStaffPrototypeArena`.

If it does not find one, and `bSpawnStaffsAtDawnArena` is enabled, the game mode spawns `RuntimeStaffsAtDawnArenaClass` at `RuntimeStaffsAtDawnArenaLocation`. By default this is offset away from the Mug Run arena so the two placeholder layouts do not overlap.

If no Staffs at Dawn arena exists, Staffs at Dawn falls back to the Mug Run/prototype arena.

## Editing In The Unreal Editor

Recommended workflow:

1. Create a Blueprint subclass of `WizardStaffPrototypeArena`, for example `BP_PrototypeArena`.
2. Place that Blueprint actor in the test level.
3. Move, scale, duplicate, or hide the cube components to tune the arena.
4. Move `PlayerSpawn_*` arrow components to adjust player starts.
5. Move `MugSpawn_*` arrow components to adjust pickup placement.
6. Adjust `ArenaHalfSize` if the playable area is larger or smaller than the default.

For Staffs at Dawn:

1. Create a Blueprint subclass of `WizardStaffStaffsAtDawnArena`, for example `BP_StaffsAtDawnArena`.
2. Place that Blueprint actor in the test level.
3. Move, scale, duplicate, or hide the platform, bridge, pillar, low-wall, and prop components to tune combat flow.
4. Move `PlayerSpawn_*` arrow components so players start with room to orient before the first bonk.
5. Move `FuturePowerupSpawn_*` arrow components to tune where Staffs at Dawn powerups such as Mega Staff Brew can appear.
6. Adjust `ArenaHalfSize` to match the outer safe bounds for ring-out respawn timing.
7. Adjust `RingOutFallDistanceBelowArena` if players are landing on lower terrain and waiting too long to count as out-of-play, or if they are being flagged before they clearly leave the raised arena.

The game mode discovers spawn markers by component name prefix:

- Player spawns must start with `PlayerSpawn`.
- Mug spawns must start with `MugSpawn`.
- Staffs at Dawn powerup spawn markers must start with `FuturePowerupSpawn`.

This means added Blueprint arrow components such as `MugSpawn_13` or `PlayerSpawn_CornerTest` will be picked up as long as the prefix is correct. Names are sorted before use, so numbered suffixes are recommended for predictable order.

Keep player spawn arrows above the floor and inside the arena bounds. Mug spawns can sit at floor level; `FWizardMugRunTuning.MugSpawnZ` still adds the pickup height offset.

## Switching Between Authored And Runtime Arena

On `AWizardStaffGameMode`:

- `bUseAuthoredPrototypeArena = true`: use a placed `WizardStaffPrototypeArena` actor if one exists.
- `bUseAuthoredPrototypeArena = false`: ignore placed arena actors.
- `bSpawnPrototypeArena = true`: spawn `RuntimePrototypeArenaClass` if no authored arena is used.
- `bSpawnPrototypeArena = false`: do not spawn a fallback arena.
- `RuntimePrototypeArenaClass`: class used for the runtime fallback.
- `bUseAuthoredStaffsAtDawnArena = true`: use a placed `WizardStaffStaffsAtDawnArena` actor for Staffs at Dawn if one exists.
- `bUseAuthoredStaffsAtDawnArena = false`: ignore placed Staffs at Dawn arena actors.
- `bSpawnStaffsAtDawnArena = true`: spawn `RuntimeStaffsAtDawnArenaClass` if no authored Staffs at Dawn arena is used.
- `bSpawnStaffsAtDawnArena = false`: do not spawn a Staffs at Dawn fallback arena.
- `RuntimeStaffsAtDawnArenaClass`: class used for the Staffs at Dawn runtime fallback.
- `RuntimeStaffsAtDawnArenaLocation`: spawn location for the runtime Staffs at Dawn fallback arena.

For normal playtests, leave the authored and fallback booleans enabled. This uses hand-authored arenas when present and still keeps older empty test maps playable.

For pure runtime fallback testing, disable `bUseAuthoredPrototypeArena` and/or `bUseAuthoredStaffsAtDawnArena`.

For testing a completely hand-built map with no fallback props, enable `bUseAuthoredPrototypeArena` and disable `bSpawnPrototypeArena`.

## Staffs At Dawn Debug Helpers

On `AWizardStaffGameMode`, enable `StaffsAtDawnTuning.bShowArenaDebug` to turn on Staffs at Dawn arena tuning helpers during countdown, active play, and results.

Optional marker toggles:

- `bDrawArenaBoundsDebug`: cyan box showing the active Staffs at Dawn arena half-size.
- `bDrawRingOutBoundsDebug`: orange box showing the respawn/ring-out horizontal bounds after `HorizontalOutOfBoundsPadding`.
- `bDrawPlayerSpawnDebug`: green spheres/arrows showing player spawn positions and facing.
- `bDrawFuturePowerupSpawnDebug`: purple markers for Staffs at Dawn powerup spawn positions.

When Staffs at Dawn starts, the debug message reports the arena source used, player spawn count, arena half-size, respawn/ring-out half-size, effective fall-out Z, and future marker count. Use this to confirm that the authored arena is being used instead of the fallback, and that expected spawn markers are being found.

## Out-Of-Arena Bounds

The respawn failsafe checks whether a wizard is outside:

`ArenaActorLocation +/- ArenaHalfSize + HorizontalOutOfBoundsPadding`

or below `FallZThreshold`.

During Staffs at Dawn, the check also uses the arena's own relative fall-out line from `RingOutFallDistanceBelowArena`. This makes lower landscape or ground under the raised arena act like void for Staffs at Dawn, without globally deleting terrain used by other prototype spaces.

When editing the arena, keep walls, props, player spawns, and mug spawns within `ArenaHalfSize` unless you intentionally want a larger safety zone. If players are respawning too early or too late, tune the arena actor's `ArenaHalfSize` first, then tune `FWizardOutOfArenaRespawnTuning.HorizontalOutOfBoundsPadding`.
