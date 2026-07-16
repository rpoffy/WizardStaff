# Cauldron Catastrophe

**Last Updated:** 2026-07-15
**Role:** Current design, implementation, tuning, and focused-test reference for the third Trial. General status belongs in [CURRENT_STATE.md](CURRENT_STATE.md); shared regression steps belong in [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md).

## Status

Cauldron Catastrophe is implemented in the normal rotation:

```text
Party Hall -> Mug Run -> Party Hall -> Staffs at Dawn -> Party Hall
-> Cauldron Catastrophe -> Party Hall -> Grand Wizard Final -> Party Hall
```

The full loop, arena entry/exit, transition floor, cleanup, deposit-triggered hazard mapping, long-staff intake reliability, Slosh-scaled slippery behavior, and cursed-orb bombardment have human verification.

## Trial Identity

Cauldron Catastrophe is a staff-risk and banking Trial. Vials become physical staff segments and grant the newest carried vial's effect. Players choose whether to keep that length/effect or expose themselves at the cauldron to bank segments before opponents, snapping, ring-outs, hazards, or the curse take them away.

Core loop:

1. A 90-second match starts in an open octagonal arena.
2. Five vials spawn initially; ambient respawns respect a seven-vial cap.
3. A vial becomes a tagged staff segment and authoritative LIFO stack entry.
4. Only the newest remaining vial effect is active.
5. A server-confirmed Quick Bonk on the active cauldron intake starts sequential banking.
6. Each valid top vial transfers separately, awards one point, and visibly arcs into the cauldron.
7. Banking ends when the eligible stack is empty or the player is interrupted.
8. Highest banked score at timeout receives the current winner/tie Favor outcome.

## Arena and Intake

- Open octagon, no enclosing collision walls; ring-outs are intentional.
- `ArenaHalfSize = 1600`.
- Vials spawn away from the cauldron, players, other vials, and the drop-off edge.
- The cauldron has North/East/South/West intakes; exactly one is active.
- Passive contact, a resting staff, inactive sides, and relocation periods cannot bank.
- Only the server's authoritative Quick Bonk impact can start banking.
- Validation uses the closest point on the actual staff collision box so long staffs can reach reliably.
- A Bonk sequence can begin at most one transaction.

The active intake is gold and pulsing; inactive sides remain dim. After at least one successful transfer, the old intake disables, a different side previews orange for `0.85s`, then becomes active. Defaults: center distance `315`, acceptance radius `220`, and banking hold range `360`.

## Vials and Staff Risk

| Vial | Segment color | Active top-stack effect |
| --- | --- | --- |
| Speed | Cyan/blue | `1.35x` movement, `1.20x` acceleration |
| Burdening Power | Orange/red | `0.75x` movement, `1.75x` Quick Bonk knockback |

Duplicate Speed vials are separate segments but do not multiply the buff. Removing/depositing/snapping the newest vial reveals the next newest effect.

Each vial segment has a unique tag. The server uses tags to distinguish vials from normal, Mega Staff, debug, and cosmetic segments. A normal stress snap removes the real top segment; if its tag is a vial, the matching stack entry disappears, awards no point, and immediately refreshes active effect and instability.

### Vial Instability

Only enemy Quick Bonks during active Cauldron play use vial instability. It increases target Staff Stress, not knockback:

```text
ExtraVials = max(0, carried unbanked vials - 2)
Multiplier = clamp(1.0 + ExtraVials * 0.15, 1.0, 1.60)
```

| Carried vials | Incoming enemy-bonk Stress |
| --- | --- |
| 0-2 | `1.00x` |
| 3 | `1.15x` |
| 4 | `1.30x` |
| 5 | `1.45x` |
| 6+ | `1.60x` cap |

One authoritative attacker/target Bonk sequence applies at most once. Banking, spills, snaps, and recollection immediately refresh the multiplier. Hazards, curse explosions, intake Bonks, passive contact, and clients cannot apply it.

## Sequential Banking

1. Server validates participant, Quick Bonk sequence, active-intake strike, top vial tag, and no competing banker.
2. Banker receives a `0.30s` initial delay, `0.45x` movement, `0.50x` acceleration, and cannot Quick Bonk.
3. Server removes one valid top vial, awards one point, refreshes staff/vial mirrors, and pulses the cauldron.
4. Remaining eligible vials transfer at `0.35s` intervals.
5. A non-vial top segment safely stops the session rather than removing unrelated staff.

Bonk interruption, leaving the hold range, Broom Boost, Staff Clash, pending respawn, lost pawn, intake relocation, or Trial cleanup ends banking. Already transferred score remains safe; untransferred vials remain carried. Intake relocation occurs only if at least one vial transferred.

Every transfer captures the real top segment transform before removal and spawns a replicated, collision-free colored copy that arcs into the cauldron. That arc is readability only: no collision, pickup, physics authority, Stress, score, or knockback.

## Ring-Out Spill

The server interrupts banking and spills up to the two newest valid unbanked vials near the victim's last safe in-arena position. Spills become neutral normal pickups and can be recollected by anyone. Older vials and already banked score remain. A non-vial top segment safely blocks further spill removal. Spill actors bypass only the ambient active-vial cap and still use server-owned collection/cleanup.

## Deposit-Triggered Hazards

Each successful transfer makes one server-owned `25%` roll:

- Speed -> slippery puddle.
- Burdening Power -> sticky sludge.

The old unrelated timed hazard cadence is removed. Hazards erupt visibly from the cauldron, arc outward, settle flush to the floor, replicate presentation, and clean up at Results.

### Slippery

Entering applies a server-owned skid in current travel direction. Entry impulse scales from `720` at zero Mana Slosh to `1480` at full Slosh. Low friction/braking and `1350` directional acceleration carry for `1.5s`. It should cause brief readable loss of control, not permanently change movement.

### Sticky

Sticky sludge reduces movement/acceleration/turning and shows a tether to its center. Outside the slack range, the server gently reels the player back. Broom Boost breaks the tether. It is deliberately mild rather than a hard root.

## Cursed Orb

- First assignment is scheduled after `18s`; later cadence is `15s`.
- The cauldron boils red for the final `3.5s` warning.
- Holder receives a three-second timer, top-segment orb, and purple orbiting aura.
- A valid player Bonk passes the curse.
- Staff Clash slows the holder's timer to `0.25x`.
- Expiration launches the holder in a random cardinal direction plus upward force (`3100` horizontal, `1250` vertical).
- If the cursed top segment snaps, the orb follows available loose debris and later makes a small ground blast (`220` radius, `1250` horizontal, `480` vertical).

### Cauldron Curse Deposit

The cursed holder can server-confirm a Quick Bonk on the active intake to replace the personal explosion with arena risk:

1. Clear the held curse and suspend reassignment.
2. Boil violently for `1.15s`; show eight hollow red landing circles.
3. Launch eight collision-free bombs at `0.16s` intervals over `0.55s`. Landing angles use jittered arena sectors and randomized radial distance, then launch in shuffled order so coverage is broad without forming a uniform ring or clockwise sweep.
4. Each `210`-radius impact applies one server-owned blast at 75% holder-curse force (`2325` horizontal, `937.5` vertical).

This grants no score, removes no vial/segment, applies no Staff Stress, and cannot be initiated or resolved by clients. Trial cleanup destroys outstanding bombs and warning presentation.

## Readability

| State | Cue |
| --- | --- |
| Active intake | Gold outward marker; other sides dim |
| Relocation | Orange preview while deposits are disabled |
| Banking | Faster pale-gold pulse plus banker/count/progress HUD |
| Transfer | Colored segment arcs into cauldron; body/rim/brew pulse |
| Hazard | Cauldron eruption, projectile arc, animated floor puddle |
| Curse incoming | Red boiling bubbles |
| Curse holder | Top-segment red orb and purple aura |
| Instability | HUD shows multiplier only above `1.00x` |

## Authority and Cleanup

GameMode/server owns Trial/timer, intake selection, banking, stack/tags, score, hazards, movement effects, curse, explosions, Favor outcome, and cleanup. Replicated state and actors provide presentation only.

Results immediately clears banking, curse, vials, hazards, bombs, arcs, movement modifiers, timers, callbacks, and readable state. The inert Cauldron floor remains through the following intermission so players never fall during the handoff; full arena destruction happens at the established next-phase cleanup boundary.

## Main Tuning Surface

Values live in `FWizardCauldronCatastropheTuning` in `Source/WizardStaff/Public/WizardStaffGameMode.h`.

| Concern | Important defaults |
| --- | --- |
| Trial/arena | `90s`, half-size `1600` |
| Vials | initial `5`, cap `7`, respawn `2s` |
| Banking | `0.30s` initial, `0.35s` transfer, range `360` |
| Ring-out spill | `2` vials |
| Instability | safe `2`, `+0.15`, cap `1.60x` |
| Deposit hazards | `25%`, lifetime `7s`, radius `185`, cap `4` |
| Slippery skid | `720-1480` impulse, `1.5s`, `1350` acceleration |
| Curse bombardment | `8` bombs, `0.35` sector jitter, radius `210`, 75% force |

Tune banking readability/interruption first, vial choice second, center risk third, arena recovery fourth, and only then duration/spawn density.

## Development Commands

Host-authority/non-shipping; indices are zero-based:

```text
DebugStartCauldronCatastrophe
DebugEndCauldronCatastrophe
DebugSpawnCauldronVial Speed|BurdeningPower
DebugGiveCauldronVial <PlayerIndex> Speed|BurdeningPower
DebugPrintCauldronVialStack <PlayerIndex>
DebugValidateCauldronVialState <PlayerIndex>
DebugBreakTopCauldronVialSegment <PlayerIndex>
DebugStartCauldronBanking <PlayerIndex>
DebugInterruptCauldronBanking
DebugSpillCauldronVials <PlayerIndex>
DebugSetCauldronIntake North|East|South|West
DebugPrintCauldronIntakeState
DebugPrintCauldronInstability <PlayerIndex>
DebugApplyCauldronInstabilityTestHit <AttackerIndex> <TargetIndex>
DebugAssignCauldronCurse <PlayerIndex>
DebugClearCauldronHazards
```

`DebugSpawnCauldronIngredient` is legacy diagnostic scaffolding, not part of the current vial loop.

## Focused Validation

Use [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md) for the shared loop. Cauldron-specific checks:

1. Verify open arena, safe staging, five initial vials, and one active gold intake.
2. Confirm passive/wrong-side contact fails and short/long staffs can validly start banking.
3. Confirm newest-only vial effect and non-stacking duplicate Speed behavior.
4. Interrupt a multi-vial bank; transferred points stay and remaining segments stay carried.
5. Verify intake relocation and no deposits during preview.
6. Observe Speed/Burdening deposits until the 25% matching hazard behavior is credible.
7. Compare slippery skid at low/full Slosh; verify sticky reel and Broom Boost escape.
8. Verify instability at 0-2/3/4/5/6+ vials and one event per enemy Bonk/Clash resolution.
9. Ring out a loaded banker; verify at most two newest spills, safe score, and neutral recollection.
10. Verify curse pass, Clash slowdown, holder explosion, snapped-segment ground explosion, and that the eight-bomb cauldron bombardment spreads broadly without looking uniform.
11. End while effects are active; Results must clear gameplay immediately while the transition floor remains safe.

## Deferred

No additional vial types, recipes, inventory, generalized non-top extraction, production UI/audio/VFX, or replicated loose debris. Legacy ingredient actors are not current normal gameplay.
