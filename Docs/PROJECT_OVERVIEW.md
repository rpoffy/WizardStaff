# Wizard Staff Project Overview

**Last Updated:** 2026-07-15
**Evidence basis:** Current C++ source/configuration, existing documentation, build records, and recorded human playtest observations.

Wizard Staff is a comedic multiplayer party-game prototype about wizards who drink mana beverages, become increasingly sloshed, grow unwieldy staffs, and create problems for one another. The project is local-first, has a direct-connect listen-server baseline, and has a narrowly scoped Steam development integration for private testing.

## Current Product Shape

| Area | Status | What this means |
| --- | --- | --- |
| Normal match loop | **Implemented and verified locally and in two-player listen-server PIE** | Party Hall separates Mug Run, Staffs at Dawn, Cauldron Catastrophe, and Grand Wizard Final. Full-loop and cleanup-focused sessions passed on 2026-07-15. |
| Local multiplayer | **Implemented and verified** | Local one-human-plus-bot and couch workflows are represented in current code and repeatedly documented as preserved. |
| Direct-connect online | **Implemented and verified** | A host listen path and `open 127.0.0.1` client path have recorded successful full-loop validation. |
| Steam session smoke path | **Partially implemented** | Steam host/session creation is observed. A same-machine join limitation was observed; two-machine/two-account search/join/travel remains unverified. |
| Steam leaderboard submission | **Implemented but unverified** | The result-submission scaffold and Steamworks leaderboard configuration exist, but a real write/flush/read-back test is not recorded. |
| Cauldron Catastrophe | **Implemented and verified** | The vial, banking, hazard, curse, and scoring Trial is the third normal Trial. Full-loop transitions and the focused hazard, long-staff intake, slippery-skid, and cursed-bombardment behavior are human-verified. |
| Production release systems | **Planned / deferred** | Production lobby UX, matchmaking, reconnect, host migration, full UI, and other release work are intentionally out of scope. |

## Core Identity: Intent Versus Current Evidence

| Identity statement | Current status | Evidence and caveat |
| --- | --- | --- |
| Comedic, physics-driven multiplayer party game | **Partially implemented** | Runtime staff collision, loose local snapped pieces, knockback, ring-outs, and dynamic arena interactions exist. The current staff component also contains obstruction/stuck recovery helpers, so the project is not yet purely emergent physics. |
| Mana drinks make wizards sloshed and harder to control | **Implemented and verified** | Mana Slosh, movement/readability mirrors, Mug Run pickups, and player documentation are present. |
| Containers stack as staff segments and grow staffs | **Implemented and verified** | Runtime staff segment construction and replicated segment count are present. Do not assume every segment is a fully simulated drink container: the code uses runtime staff visual/collision components. |
| Long staffs create navigation and collision problems | **Implemented and verified** | Staff collision, arena props, obstruction tracking, stress, and shared camera behaviors exist. |
| Wizards bonk one another and may snap segments | **Implemented and verified** | Quick Bonk, Staff Clash, stress, snapping, local loose segment behavior, and online snap readability scaffolding exist. |
| Staff problems should arise naturally rather than from arbitrary stuck states | **Established design direction; partially met** | This is a current design tension: `UWizardStaffComponent` includes scripted obstruction detection, control reduction, recovery, and a failsafe. Any future change should be explicitly approved and tested rather than silently removing safety behavior. |
| Steam private playtest preparation | **Partially implemented** | Steam app/depot/private build history is documented; private access/package management and multiplayer validation are still incomplete. |

## Normal Prototype Loop

The locked normal loop is:

1. Party Hall / intermission
2. Mug Run
3. Results / Party Hall return
4. Staffs at Dawn
5. Results / Party Hall return
6. Cauldron Catastrophe
7. Results / Party Hall return
8. Grand Wizard Final
9. Results and rematch

Cauldron Catastrophe is now the approved third Trial in the normal rotation. See [MINIGAME_CATALOG.md](MINIGAME_CATALOG.md).

## Documentation Map

- [GAME_DESIGN.md](GAME_DESIGN.md): player-facing pillars, mechanics, and scope boundaries.
- [TECHNICAL_ARCHITECTURE.md](TECHNICAL_ARCHITECTURE.md): Unreal structure, runtime ownership, networking, Steam, and map behavior.
- [CURRENT_STATE.md](CURRENT_STATE.md): evidence-based snapshot of what is working now.
- [DESIGN_DECISIONS.md](DESIGN_DECISIONS.md): active, historical, and superseded decisions.
- [MINIGAME_CATALOG.md](MINIGAME_CATALOG.md): trial inventory and status.
- [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md): repeatable validation paths.
- [ROADMAP.md](ROADMAP.md): suggested sequencing, not an approval to build everything listed.
- [KNOWN_ISSUES.md](KNOWN_ISSUES.md): confirmed gaps, verification gaps, and design tensions.
- [CauldronCatastropheDesign.md](CauldronCatastropheDesign.md): focused third-Trial rules, tuning, authority, and checks.
- [PlayerGuide.md](PlayerGuide.md): concise player-facing controls and rules.
- [TwoClientListenServerTestGuide.md](TwoClientListenServerTestGuide.md): beginner launch guide; detailed checks stay in `PLAYTEST_PLAN.md`.
- [OnlineMultiplayerArchitecturePlan.md](OnlineMultiplayerArchitecturePlan.md): condensed online milestone and authority record.
