# Wizard's Staff Player Guide

Last updated: July 2, 2026

Wizard's Staff is a local multiplayer physics party game about goofy wizards building huge, awkward magical staffs and bonking each other with them.

This guide explains the current prototype slice. It is written for players, not developers, and does not include debug or tuning commands.

## The Goal

Become the Grand Wizard.

Each match is made of short Trials followed by the Grand Wizard Final. Trials help you earn advantage, grow your staff, win rounds, and build Grand Wizard Favor. At the end, the Final decides the whole match.

The current prototype match is:

1. Party Hall.
2. Mug Run.
3. Results.
4. Party Hall.
5. Staffs at Dawn.
6. Results.
7. Grand Wizard Final.
8. Winner and rematch.

## The Big Idea

Your staff is your power and your problem.

When your staff grows, you get more reach and stronger bonks. But a huge staff is also heavier, harder to swing, harder to steer with, easier to wedge on walls, and more likely to break.

Growing your staff also makes you Mana Sloshed. A sloshed wizard is stronger and sillier, but harder to control. Slosh can make turning, stopping, broom recovery, and careful movement more difficult.

## Core Mechanics

### Staff Segments

Staff segments are added to the top of your staff. The taller the staff, the more dangerous you become.

You can gain staff segments by:

- Drinking mugs in Mug Run.
- Landing bonks in Staffs at Dawn.
- Scoring ring-outs in Staffs at Dawn.
- Picking up Mega Staff Brew in Staffs at Dawn.
- Receiving a Final Round staff advantage from Grand Wizard Favor.

### Mana Slosh

Mana Slosh is magical instability from staff growth.

More Slosh means:

- You become harder to steer cleanly.
- Broom boost recovery becomes less reliable.
- Your wizard may feel wobblier.
- Your bonks hit a bit harder.

When staff segments snap off, some Mana Slosh is released. Breaking your staff is bad, but it can also help a completely sloshed wizard regain a little control.

### Staff Stress And Snapping

Your staff builds Stress when it bonks players, hits walls, collides with props, or casts certain magic.

When Staff Stress gets too high, the top staff segment snaps off. Snapped pieces become loose physics objects for a short time and can cause extra magical chaos.

### Bonking

Bonking is your main attack.

A bonk uses your staff's actual reach and collision. You need the staff to make contact. Bigger staffs reach farther and can hit harder, but they are slower, heavier, and more fragile.

If two wizards bonk each other at nearly the same time and angle, they can enter Staff Clash. During Staff Clash, both wizards lock into a short duel and mash bonk. The winner releases a stronger bonk. If both mash equally, both bonks fizzle.

### Broom Boost

Jump once to hop.

Press jump again while airborne to trigger a one-use broom boost. The broom boost gives you a short forward recovery burst and can save you from some ring-outs.

Broom boost is not full flight. A clean launch near an edge can still knock you out, especially if you are very sloshed or carrying a giant staff.

## Match Flow

### Party Hall

Party Hall is the playable intermission room.

You can:

- Move around.
- Bonk lightly.
- Read standings.
- See who is leading.
- See the active preset.
- Bonk the Ready Bell.

Bonk the Ready Bell when you are ready for the next Trial. When all active players are ready, the next Trial starts sooner. The normal timer still starts the Trial if players do not all ready up.

### Mug Run

Mug Run is the staff-growing Trial.

Run around the arena collecting mugs. A mug:

- Adds one staff segment.
- Adds Mana Slosh.
- May grant a one-use brew reward.

The player with the tallest staff at the end wins the Trial and earns Grand Wizard Favor. Ties are allowed.

#### Arcane Pinball

Arcane Pinball is the current brew reward spell.

If you receive Arcane Pinball, your spellbook glows fuchsia. Use your reward to fire a bouncing magical projectile. It ricochets around the arena, speeds up as it bounces, and can hit other players or even yourself.

Arcane Pinball does not cost mana. Its risk comes from chaos, self-hits, Slosh, and Staff Stress.

### Staffs At Dawn

Staffs at Dawn is the combat Trial.

There are no mugs in this Trial. Instead, you score by fighting.

You can score by:

- Landing staff bonks.
- Knocking recently bonked players out of the arena.

You can grow your staff by:

- Landing enough bonks.
- Scoring ring-outs.
- Picking up Mega Staff Brew.

If you are ringed out, you respawn and your physical staff segments reset. Your score and match performance stay, but the immediate big-staff threat is cleared.

#### Mega Staff Brew

Mega Staff Brew is the current Staffs at Dawn powerup.

It temporarily gives you a huge staff. You gain scary reach and power, but you also become easier to punish because the staff is heavier, more fragile, harder to recover with, and easier to wedge.

Mega Staff Brew is meant to create a short panic moment, not a safe super mode.

### Grand Wizard Final

After the Trials, the match enters the Grand Wizard Final.

The player with the strongest Final advantage starts as the Grand Wizard Candidate. Grand Wizard Favor matters most, then round wins, then current staff count as a fallback.

The Candidate starts with a large staff advantage and tries to control the ritual circle.

Challengers try to:

- Bonk the Candidate away from the circle.
- Enter or hold the circle.
- Steal the Candidate title.

Whoever is the Candidate when the Final timer ends wins the whole match.

## Controls

### Keyboard And Mouse

| Action | Input |
| --- | --- |
| Move | `WASD` or arrow keys |
| Turn | Mouse X or `Q` / `E` |
| Jump | `Space` |
| Broom boost | Press `Space` again while airborne |
| Quick Bonk | Left mouse button or `F` |
| Use brew reward | Right mouse button |

Mugs are collected by touching them. You do not need a button to drink a mug during Mug Run.

### Gamepad

| Action | Input |
| --- | --- |
| Move | Left stick |
| Turn | Right stick |
| Jump | Bottom face button, such as Xbox `A` or PlayStation `Cross` |
| Broom boost | Press jump again while airborne |
| Quick Bonk | Right shoulder, such as `RB` or `R1` |
| Use brew reward | Left shoulder, such as `LB` or `L1` |

### Same-Keyboard Player 2 Fallback

If the prototype is set up for a second player on the same keyboard, Player 2 can use:

| Action | Input |
| --- | --- |
| Move | `I` / `J` / `K` / `L` |
| Turn | `U` / `O` |
| Jump | Right Shift |
| Broom boost | Press Right Shift again while airborne |
| Quick Bonk | Right Control |
| Use brew reward | Semicolon |

## What The HUD Shows

The HUD shows the current Trial, timer, player colors, staff segment counts, Mana Slosh, Staff Stress, round wins, Grand Wizard Favor, carried reward, Ready Bell status, Trial scores, Candidate status, and winner messages.

Each wizard also has a flat colored marker under them. The marker is a circle with a pointed front, and the point shows which way that wizard is facing. Use it to keep your bearings when the camera is pulled back, staffs are huge, or the arena gets chaotic.

The most important things to watch are:

- Staff segment count: how tall and dangerous your staff is.
- Mana Slosh: how unstable your wizard is.
- Staff Stress: how close your staff is to snapping.
- Favor: how much Final Round advantage you are earning.
- Current Candidate: who is winning the Grand Wizard Final right now.

## Tips

- A bigger staff is not always better. It hits harder, but it is awkward and fragile.
- If your staff keeps catching on props, slow down or change your angle.
- Bonking near edges is dangerous for everyone.
- Use broom boost for near-miss saves, not guaranteed escapes.
- Being sloshed can make you stronger, but it also makes recovery harder.
- Snapping a segment hurts, but it can release some Slosh.
- Arcane Pinball can hit you too.
- Mega Staff Brew is powerful, but it makes you a huge target.
- In the Final, the Candidate wants the circle. Challengers want the Candidate out of it.
