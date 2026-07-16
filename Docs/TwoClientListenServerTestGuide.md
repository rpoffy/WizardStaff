# Two-Client Listen-Server Test Guide

**Last Updated:** 2026-07-15
**Audience:** A first-time Unreal multiplayer tester. The authoritative regression checklist is [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md); this file explains how to launch and observe it.

## What This Tests

One host/listen server and one connected client run the current loop:

```text
Party Hall -> Mug Run -> Party Hall -> Staffs at Dawn -> Party Hall
-> Cauldron Catastrophe -> Party Hall -> Grand Wizard Final -> rematch
```

This is not matchmaking, a public browser, reconnect testing, or exact cross-client physics validation.

## Controls

- Move: `WASD` or left stick.
- Turn: mouse X, `Q/E`, or right stick.
- Jump/Broom Boost: `Space` or bottom face button; press again airborne.
- Quick Bonk: left mouse/`F` or right shoulder.
- Use reward: right mouse or left shoulder.
- Cycle HUD: `H`.

Click a PIE window before controlling it. One keyboard can test one window at a time; a controller makes simultaneous control easier.

## PIE Setup

1. Open `C:\Users\Roger\Documents\WizardStaffGame\WizardStaff.uproject`.
2. Open `/Game/Maps/WizardStaff_Prototype`.
3. Open the dropdown beside the Unreal Editor `Play` button.
4. Set `Number of Players` to `2`.
5. Set `Net Mode` to `Play As Listen Server`.
6. Enable separate windows if practical.
7. Start PIE.

Expected startup:

- Two game views, one listen server and one client.
- Exactly one possessed wizard per player; no playtest bots.
- Top-down shared camera in both views.
- Stable P1/P2 slot and color.
- Party Hall/runtime presentation visible.
- Inputs locked during countdown/setup and unlocked only during active play.

Record a possession, camera, presentation, or input-lock bug immediately before continuing.

## Separate-Process Direct Connect

Close PIE first. This remains non-Steam.

Host PowerShell:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Roger\Documents\WizardStaffGame\WizardStaff.uproject" /Game/Maps/WizardStaff_Prototype?listen -game -log -windowed -ResX=1280 -ResY=720
```

Client PowerShell:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Roger\Documents\WizardStaffGame\WizardStaff.uproject" -game -log -windowed -ResX=1280 -ResY=720
```

Open the client console and run:

```text
open 127.0.0.1
```

Fallback: `open 127.0.0.1:7777`.

Expected: host accepts P2, client enters `OnlineClient`, both PlayerState mirrors appear, and Party Hall proceeds only after the real client joins.

## Full-Loop Smoke Pass

### Party Hall

- Move each owned wizard and verify both views see it.
- Confirm stable slots/colors, one pawn each, standings, next Trial, and Ready Bell.
- Bonk Ready Bell if practical; ready/countdown mirrors should agree.

### Mug Run

- Confirm arena appears before staging and countdown input remains locked.
- Collect one mug per player; both views should agree on pickup hiding, staff count, Slosh, reward, and respawn.
- Use Arcane Pinball if available; exactly one server projectile should spawn, bounce/hit readably, and clean up.

### Staffs at Dawn

- Test Quick Bonk from each player, one hit, and Broom Boost.
- Trigger Staff Clash and ring-out/respawn if practical.
- Collect Mega Staff; pickup and temporary segments should appear and clear in both views.
- Online snapped staff count/cue should replicate; loose snapped debris is intentionally standalone-only.

### Cauldron Catastrophe

- Confirm floor exists before placement and both views see the open arena.
- Collect a vial per player and bank at least one through the active intake.
- Verify score/readability and no passive or wrong-side banking.
- Exercise one hazard or curse interaction if practical.
- Results must clear Cauldron gameplay effects while preserving a safe transition floor.

### Grand Wizard Final

- Confirm prototype arena and ritual circle both appear.
- Candidate, SAFE/VULNERABLE, steal progress, timer, and winner must agree.
- Server remains authoritative for swaps and winner selection.

### Rematch

- Both views return to clean Party Hall.
- Each PlayerState keeps its P1/P2 slot, color, pawn, HUD row, spawn identity, score/Favor attribution, and event labels.
- No stale pickup, projectile, powerup, Mega Staff, combat, respawn, Cauldron, Final, HUD, or event-feed state remains.
- Start the next Mug Run once to prove the next generation is clean.

## Observation Notes

The separately spawned client window may look smoother than an unfocused embedded PIE viewport because Unreal throttles background editor views. Focus the viewport being evaluated before judging camera, pawn movement, or facing smoothness; choppiness confined to an unfocused view is not a gameplay networking defect.

Use this failure template:

```text
Phase:
Host, client, or both:
Action:
Expected:
Observed:
Reproducible:
Screenshot/video/log:
```

## Pass Criteria

- One owned wizard per player; stable P1/P2 identity.
- Top-down camera and runtime presentation in both views.
- Input locks, Trial order, Results/Party Hall handoffs, and rematch are correct.
- Gameplay outcomes are server-owned and readable remotely.
- No stale actors, timers, effects, mirrors, or messages cross boundaries.

For detailed system checks and Steam validation, continue in [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md).
