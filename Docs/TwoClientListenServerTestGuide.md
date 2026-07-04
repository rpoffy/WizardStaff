# Two-Client Listen-Server Test Guide

Last updated: July 4, 2026

This guide explains how to run the first hands-on two-client listen-server smoke test for Wizard's Staff.

It is written for someone who has never used Unreal multiplayer PIE before.

## What This Test Is

This is a local editor test where one Unreal Editor PIE window acts as the host/listen server and another PIE window acts as a connected client.

The goal is to confirm the current online scaffolding can survive the existing prototype loop:

1. Party Hall.
2. Mug Run.
3. Staffs at Dawn.
4. Grand Wizard Final.
5. Winner/results.
6. Rematch back to Party Hall.

This is not Steam multiplayer. It does not use lobbies, invites, matchmaking, or a public server.

## Before You Start

Build the project first if you have changed C++ recently.

Open the project:

`C:\Users\Roger\Documents\Wizard's Staff game\WizardStaff.uproject`

Open the normal prototype map or the map you usually use for PIE. If the current map has no authored arena/hall, the prototype GameMode should still use its runtime fallback actors.

## Controls You Will Need

Keyboard/mouse controls:

- Move: `WASD` or arrow keys.
- Turn/facing: mouse X, or `Q` / `E`.
- Hop / broom boost: `Space`.
- Quick Bonk: `F` or left mouse button.
- Use held reward: right mouse button.
- Cycle HUD mode: `H`.

Controller controls, if using one:

- Move: left stick.
- Turn: right stick.
- Hop / broom boost: bottom face button.
- Quick Bonk: right shoulder.
- Use held reward: left shoulder.

For two PIE windows on one machine, input focus can be awkward. Click a window to control that client. If you only have one keyboard/mouse, it is okay to control one window at a time for this smoke test. A controller can make the second window easier to drive.

## PIE Setup

1. In Unreal Editor, find the `Play` button on the toolbar.
2. Open the small dropdown/three-dot menu next to `Play`.
3. Open the play settings or advanced settings panel.
4. Set `Number of Players` to `2`.
5. Set `Net Mode` to `Play As Listen Server`.
6. Enable separate windows if there is an option for it.
7. Leave single-process/multi-PIE enabled unless you specifically need separate processes.
8. Start PIE.

Expected result:

- Two game windows appear.
- One window is the listen-server host.
- One window is the client.
- Each window should have one wizard.
- Each player should control only their own wizard.

If only one window appears, stop PIE and recheck `Number of Players = 2`.

If the second window appears but does not control a wizard, write that down as a possession/spawn bug.

If either window starts in a first-person/pawn view instead of the top-down shared camera, write down which window it was. A previous bug affected the secondary PIE client window specifically when its view target stayed on the pawn before the replicated shared camera was ready.

If either window sees only the open world map without the Party Hall, arena walls, floor, or placeholder meshes, write that down as a runtime arena replication bug.

If the client window camera is correct but constantly jitters, test once while both wizards stand still and once while either wizard moves. Write down whether the camera itself jitters, the remote wizard jitters, or both.

## Separate Process Direct-Connect Setup

Use this only after the PIE path is good enough. This is still not Steam multiplayer. It is one local host process running as a listen server and one local client process joining by address.

Important current map note:

- This repo currently has no project `.umap` assets under `Content` and `Config/DefaultEngine.ini` does not set a project `GameDefaultMap`.
- The prototype GameMode can spawn its runtime Party Hall and arenas in the default/open-world map.
- The commands below use `/Engine/Maps/Templates/OpenWorld` as the explicit fallback map because that matches the open-world editor map you have been seeing. If you later make or choose a real prototype map, replace that map path with the project map path.

Close PIE before starting this test.

Host window:

1. Open PowerShell.
2. Run:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Roger\Documents\Wizard's Staff game\WizardStaff.uproject" /Engine/Maps/Templates/OpenWorld?listen -game -log -windowed -ResX=1280 -ResY=720
```

Expected host result:

- A standalone game window opens.
- The host is the listen server.
- The host should get one wizard.
- The top-down shared camera should be active.
- Party Hall/runtime arena readability should appear after the GameMode starts.
- The host should stay in Party Hall/intermission until a second player connects.

Client window:

1. Open a second PowerShell window.
2. Run:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "C:\Users\Roger\Documents\Wizard's Staff game\WizardStaff.uproject" -game -log -windowed -ResX=1280 -ResY=720
```

3. In the client game window, open the console with the backtick/tilde key.
4. Run:

```text
open 127.0.0.1
```

If it does not connect, try:

```text
open 127.0.0.1:7777
```

Expected client result:

- The client joins the host.
- The client gets exactly one wizard.
- The client uses the top-down shared camera.
- The client can see the host wizard, Party Hall/runtime arenas, HUD readability, and replicated event feed.
- The client should not create local bots or extra local prototype players.
- Once the client joins, the Party Hall countdown can proceed into Mug Run.

Fallback:

- If separate-process direct connect fails early, go back to the PIE setup above and retest with `Number of Players = 2` and `Net Mode = Play As Listen Server`.
- Write down the exact host/client command, the first bad symptom, and any warning/error lines from either log window.

## How To Read The Windows

Unreal usually labels PIE windows in the title bar. The exact label can vary by editor version, but you are looking for one host/listen-server window and one client window.

If you are unsure which is which:

1. Click one window.
2. Move the wizard.
3. Check whether that movement appears in the other window.
4. Repeat with the other window.

For a successful smoke test, movement from either controlled wizard should be visible in both windows.

## Smoke Test Checklist

Use this checklist in order. If something fails, stop and write down the phase, which window failed, and what you saw.

### 1. Party Hall

Expected:

- Both windows start in or return to Party Hall.
- Each window has exactly one controlled wizard.
- Player colors/slots are stable.
- The HUD shows the party/intermission state.
- The Ready Bell and standings are readable enough to use.

Test:

1. Click the host window and move the host wizard.
2. Confirm the client window sees the host wizard move.
3. Click the client window and move the client wizard.
4. Confirm the host window sees the client wizard move.
5. Bonk the Ready Bell with each wizard if practical.
6. Confirm the countdown/ready state updates on both windows.

Failure notes to record:

- Duplicate wizards.
- Missing wizard.
- Wrong player controls the wrong wizard.
- Client HUD only works on the host.
- Ready state appears on one window but not the other.

### 2. Mug Run

Expected:

- Mug Run starts after Party Hall/countdown.
- During the Mug Run countdown, neither host nor client-owned wizard should be able to move out of their staged start position.
- Mugs are visible on both windows.
- Mug collection is server-owned.
- Collected mugs disappear on both windows.
- Respawned mugs reappear on both windows.
- Staff count and Mana Slosh readability update on both windows.
- Brew reward display appears on both windows when a reward is granted.

Test:

1. Let the game progress into Mug Run.
2. Collect a mug with the host wizard.
3. Confirm both windows see the mug hide/deactivate.
4. Confirm both windows see the host wizard staff count grow.
5. Confirm Slosh readability changes if visible.
6. Collect a mug with the client-owned wizard.
7. Confirm both windows see the client wizard staff count grow.
8. Wait for a mug respawn if practical.
9. Confirm no duplicate mugs appear.

Failure notes to record:

- Client can move during the Mug Run countdown.
- Mug disappears only on one window.
- Staff grows only on one window.
- Client can appear to collect without the server updating.
- Mug never respawns.
- Duplicate mug appears.
- Reward display appears on one window but not the other.

### 3. Use Reward / Arcane Pinball

Expected:

- Arcane Pinball is only used after the server has granted the reward.
- One accepted Use Reward input spawns one projectile.
- The projectile is visible on both windows.
- Movement, bounce, hit, and cleanup are server-owned.
- Projectile bounce direction and trail/streak readability stay reasonably aligned in the client window.
- Event messages are useful but not spammy.

Test:

1. During Mug Run, collect mugs until Arcane Pinball is granted.
2. Use the reward on the host wizard.
3. Confirm the reward display clears on both windows.
4. Confirm one Arcane Pinball projectile appears on both windows.
5. If practical, bounce it off a wall.
6. If practical, hit the other wizard.
7. Confirm hit/knockback/readability appears on both windows.
8. Repeat with the client-owned wizard if a reward is granted.
9. Let the projectile expire or transition out of Mug Run.
10. Confirm no old projectile survives into Staffs at Dawn or Party Hall.

Failure notes to record:

- Reward clears but no projectile spawns.
- Two projectiles spawn from one use.
- Projectile appears only on the host.
- Hit/knockback happens only on one window.
- Old projectile survives after Trial transition.
- Event feed prints every bounce or repeats the same message.

### 4. Staffs At Dawn

Expected:

- Staffs at Dawn starts after Mug Run Results and Party Hall.
- During the Staffs at Dawn countdown, neither host nor client-owned wizard should be able to move out of their staged start position.
- Host and client can move and bonk.
- Quick Bonk requests from either wizard reach the server.
- Staff Clash only starts from valid server-owned bonk flow.
- Ring-out/respawn remains server-owned.
- Broom boost works for the client-owned wizard when pressing `Space` while falling.

Test:

1. Progress to Staffs at Dawn.
2. Quick Bonk with the host wizard.
3. Confirm both windows see the host bonk readability.
4. Quick Bonk with the client wizard.
5. Confirm both windows see the client bonk readability.
6. Put the wizards near each other and try to land a bonk.
7. Confirm hit/knockback appears on both windows if contact succeeds.
8. If practical, bonk near-simultaneously to trigger Staff Clash.
9. If Staff Clash starts, mash bonk from the active participant windows.
10. Confirm the clash resolves and movement unlocks.
11. If a wizard falls out, confirm respawn readability appears and clears.
12. Hop/fall with the client-owned wizard and press `Space` again to confirm broom boost reaches the server.

Failure notes to record:

- Client can move during the Staffs at Dawn countdown.
- Client bonk does nothing.
- Bonk only animates locally.
- Hit/knockback desyncs.
- Staff Clash starts on one window only.
- Staff Clash never clears.
- Respawn countdown gets stuck.
- Client broom boost does nothing.
- Player remains input-locked after clash or respawn.

### 5. Staffs Powerup / Mega Staff

Expected:

- Staffs powerup appears on both windows.
- Collection is server-owned.
- Pickup hides/deactivates on both windows after collection.
- Mega Staff state appears on both windows.
- Mega Staff expires cleanly.
- No duplicate powerups appear.

Test:

1. In Staffs at Dawn, find the powerup pickup.
2. Collect it with the host wizard if practical.
3. Confirm both windows see the pickup hide.
4. Confirm both windows see Mega Staff readability/staff growth.
5. Wait for expiration if practical.
6. Confirm temporary staff state clears safely.
7. Repeat with the client-owned wizard if the pickup respawns.

Failure notes to record:

- Pickup visible on one window but hidden on the other.
- Client appears to grant itself Mega Staff without server update.
- Mega Staff never expires.
- Staff segments are removed incorrectly on expiration.
- Powerup duplicates after respawn or rematch.

### 6. Grand Wizard Final

Expected:

- Final starts after the two Trials.
- Client movement is blocked in any non-active Final/Results setup state.
- Server chooses the Candidate.
- Candidate readability appears on both windows.
- The ritual circle is visible on both windows.
- SAFE/VULNERABLE readability updates on both windows.
- Steal progress is server-owned.
- Winner/result is server-owned and readable on both windows.

Test:

1. Progress to Grand Wizard Final.
2. Confirm both windows show the same Candidate.
3. Confirm both windows can see the ritual circle.
4. Move the non-Candidate into the ritual circle when stealing should be possible.
5. Confirm steal progress appears on both windows if the server considers the steal valid.
6. Leave the circle or get knocked away.
7. Confirm steal progress clears or decays.
8. Complete a steal if practical.
9. Confirm Candidate changes on both windows.
10. Let the Final timer end.
11. Confirm both windows show the same winner.

Failure notes to record:

- Client can move before Final/Results state says movement should be active.
- Candidate differs between windows.
- Ritual circle appears only on the host.
- SAFE/VULNERABLE differs between windows.
- Steal progress appears only locally.
- Steal progress continues after leaving the circle.
- Winner differs between windows.
- Final state remains visible after rematch.

### 7. Event Feed

Expected:

- Both windows see important server-owned event messages.
- Messages are short and bounded.
- The feed does not spam per-frame events.
- Old messages clear at major transitions.

Watch for messages during:

- Mug pickup.
- Brew reward grant/use.
- Arcane Pinball cast/hit.
- Ring-out/respawn.
- Staffs powerup/Mega Staff.
- Candidate change.
- Final winner.
- Rematch/new match.

Failure notes to record:

- Host gets duplicate messages.
- Client does not see messages.
- Feed lists every bounce/tick/overlap.
- Old Trial messages remain in Party Hall or next match.

### 8. Rematch / Reset

Expected:

- After the Final winner, the game returns to Party Hall.
- Each player still has exactly one possessed wizard.
- No old pickups, projectiles, powerups, Mega Staff state, combat state, Final state, HUD labels, or event messages survive into the next match.
- The next loop starts cleanly.

Test:

1. Let the Final choose a winner.
2. Wait for automatic rematch/reset.
3. Confirm both windows return to Party Hall.
4. Move host and client wizards again.
5. Confirm both still possess exactly one wizard.
6. Confirm Candidate/STEALING/WINNER labels are gone unless the current state should show them.
7. Start the next Trial loop.
8. Confirm the first Trial begins cleanly.

Failure notes to record:

- Duplicate wizard after rematch.
- Missing possessed wizard.
- Stale Final Candidate or winner label.
- Old projectile still exists.
- Old powerup still exists.
- Event feed still shows previous match messages.
- Next Trial fails to start.

## What To Write Down When Something Breaks

Use this format:

```text
Phase:
Window:
What I did:
Expected:
Actual:
Does it reproduce:
Screenshot/video:
Log warning/error if any:
```

Example:

```text
Phase: Mug Run
Window: Client
What I did: Client wizard collected a mug
Expected: Both windows hide the mug and client staff grows by 1
Actual: Host saw the mug hide, client still saw the mug visible
Does it reproduce: Happened twice on the same mug
Screenshot/video: yes
Log warning/error if any: none noticed
```

## Quick Pass Criteria

The smoke test passes if:

- Host and client each possess one wizard.
- Movement/facing is visible between windows.
- Mug collection, staff count, Slosh, reward display, and pickup respawn are readable on both windows.
- Arcane Pinball reward use spawns one server projectile and cleans up.
- Quick Bonk, Staff Clash, and ring-out/respawn do not get stuck.
- Staffs powerup/Mega Staff state appears and clears.
- Final Candidate/steal/winner state appears and clears.
- Event feed stays useful and not spammy.
- Rematch returns to clean Party Hall.
- The next loop can start.

## Known Scope Limits

Do not expect this test to prove:

- Steam lobbies.
- Matchmaking.
- Friend invites.
- Public lobby browser.
- Dedicated servers.
- Prediction, rewind, or lag compensation.
- Exact cross-client physics fidelity.
- Loose snapped segment physics replication.
- Final production UI.

This is only the first practical listen-server smoke test for the current prototype scaffolding.
