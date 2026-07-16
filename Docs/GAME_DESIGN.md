# Wizard Staff Game Design

**Last Updated:** 2026-07-15

## Design Pillars

1. **Readable chaos.** Players should understand why a funny outcome happened: staff length, slosh, bonk contact, ring-out risk, a curse, or a visible trial objective.
2. **Physical comedy with consequences.** Long, awkward staffs should make space, reach, navigation, and recovery interesting rather than merely cosmetic.
3. **Social, short-form competition.** The prototype is built around short Trials, visible standings, reversals, and a decisive Final.
4. **Risk is legible.** Bigger staffs and temporary power matter, but they also make a wizard more cumbersome, fragile, or targetable.
5. **Authority does not replace fun.** Online scaffolding should preserve the local prototype's readable interactions while the server owns results.

## Core Player Systems

### Staff Growth

**Implemented and verified.** Wizards gain staff segments through existing Trial actions. The staff is rebuilt from runtime visual/collision segments. It increases reach and changes the physical problem the player presents to the arena and other wizards.

Current documented growth sources include Mug Run mugs, Staffs at Dawn bonk/ring-out progression, Mega Staff, Final advantage, and Cauldron scoring. Exact rules belong to the relevant Trial documentation and code tuning, not this overview.

### Mana Slosh

**Implemented and verified.** Growth contributes to Mana Slosh. Current player-facing documentation describes reduced steering/control and altered recovery alongside stronger/sillier combat. Replicated mirrors exist for online readability.

### Staff Stress and Snapping

**Implemented and verified locally; online readability scaffolded.** Stress accumulates from current interactions such as bonks and staff/environment contact. A snap removes the top segment and applies existing relief/side-effect logic. Local play retains loose snapped-segment physics comedy. Online play deliberately uses authoritative loss plus readable cues rather than replicated loose debris physics.

**Design tension:** the component has explicit obstruction detection/recovery and a stuck failsafe. The intended long-term direction favors natural physical problems. Do not remove or redesign those helpers without an explicit request and a measured replacement.

### Quick Bonk, Clash, and Ring-out

**Implemented and verified.** Quick Bonk is the basic staff attack. Server-owned online scaffolding covers hit confirmation/readability. Staff Clash is a short contested bonk interaction with server-owned resolution. Ring-outs and respawns are server-owned online and retain local behavior.

### Broom Boost

**Implemented and verified.** The current player guide describes a second airborne jump as a short recovery burst, not flight. It participates in movement and some hazard escape/readability interactions.

### Brews and Arcane Pinball

**Implemented and verified.** Mug Run can grant a carried brew reward. Arcane Pinball is the current documented reward: server-owned online projectile movement/hits/cleanup with replicated readability, while local behavior remains intact.

### Final Advantage

**Implemented and verified.** Grand Wizard Favor, round wins, and existing tie-breaker logic feed the Final Candidate selection. The Final is not a new spell or separate economy; it is the match-deciding control/steal phase.

## Trial Rules at a Glance

| Trial | Status | Purpose |
| --- | --- | --- |
| Mug Run | **Implemented and verified** | Grow staffs by collecting mugs; rewards may create Arcane Pinball risk. |
| Staffs at Dawn | **Implemented and verified** | Combat, bonks, ring-outs, Staff Clash, and Mega Staff moments. |
| Grand Wizard Final | **Implemented and verified** | Candidate control/steal phase deciding the match. |
| Cauldron Catastrophe | **Implemented and verified** | Vial, banking, hazard, and curse Trial between Staffs at Dawn and the Final; current focused mechanics and normal-loop integration are human-verified. |

See [MINIGAME_CATALOG.md](MINIGAME_CATALOG.md) for verification boundaries and [CauldronCatastropheDesign.md](CauldronCatastropheDesign.md) for the detailed Cauldron design.

## Scope Boundaries

The following are not current approved implementation work unless the user explicitly asks:

- Additional Trials beyond Mug Run, Staffs at Dawn, Cauldron Catastrophe, and Grand Wizard Final.
- New spells, rewards, Staffs at Dawn powerups, Hammer Time, progression, shops, inventory, or production cosmetics.
- Broad gameplay retuning disguised as networking or UI work.
- Production lobby/browser/matchmaking/friend-invite UI.
- Loose snapped-segment physics replication, client prediction, rewind, lag compensation, or exact cross-client physics fidelity.

## Player Controls

The repository's input configuration currently includes keyboard/mouse, gamepad, and same-keyboard fallback mappings. The player-facing reference is [PlayerGuide.md](PlayerGuide.md). Treat that guide as a player aid; confirm current input behavior in `Config/DefaultInput.ini` before changing controls.
