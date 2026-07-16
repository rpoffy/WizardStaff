# Current State

**Last Updated:** 2026-07-16
**Purpose:** A current, evidence-based status snapshot. For historical details, use [DESIGN_DECISIONS.md](DESIGN_DECISIONS.md) and [OnlineMultiplayerArchitecturePlan.md](OnlineMultiplayerArchitecturePlan.md).

## Evidence Rules

- **Implemented and verified** means current repository evidence plus a recorded build, direct observation, or human playtest result.
- **Implemented but unverified** means source/configuration exists, but the relevant outcome was not recorded as tested.
- **Partially implemented** means a useful path exists but a boundary, validation, or intended behavior remains incomplete.
- The workspace currently contains substantial uncommitted implementation and documentation work. Treat current source as more current than the `main` branch history.

## Gameplay and Local Play

| Area | Status | Evidence |
| --- | --- | --- |
| Party Hall -> Mug Run -> Staffs at Dawn -> Cauldron Catastrophe -> Grand Wizard Final -> rematch | **Implemented and verified locally and in two-player listen-server PIE** | Multiple standalone playthroughs plus a 2026-07-15 two-player listen-server PIE pass confirmed the loop, intermission handoffs, and gameplay mechanics. |
| Local one-human-plus-bot, local couch, shared camera, keyboard fallback | **Implemented and verified** | Current mode-gating code and preserved-workflow records. Continue to regression-test after changes. |
| Mug pickups, staff growth, Slosh, brew readability | **Implemented and verified** | Current actor/character code and recorded online smoke results. |
| Arcane Pinball | **Implemented and verified** | Server-owned online scaffold, cleanup passes, and reported playthrough success. |
| Quick Bonk, Staff Clash, ring-out/respawn, Mega Staff | **Implemented and verified** | Existing authority/readability code and recorded direct-connect full-loop result. |
| Grand Wizard Final Candidate/steal/winner readability | **Implemented and verified** | Existing server/readability code and recorded full-loop result. |
| Snapped staff segments | **Implemented and verified** | Local loose physics remains. Online authoritative count/readability is verified without loose debris replication. |
| Transition reset/staging responsibility split | **Implemented and verified** | Mug Run state/pickup reset, wizard gameplay reset, arena phase presentation, and wizard placement now have separate explicit helpers. `WizardStaffEditor` built successfully, and a 2026-07-15 two-player listen-server session verified the resulting loop and handoffs. |

## Cauldron Catastrophe

| Area | Status | Evidence |
| --- | --- | --- |
| Cauldron trial classes and tuning | **Implemented and verified** | Arena, vial, hazard, ingredient, and deposit-arc classes are in current source and the focused Trial behavior has been human-tested. |
| Vial pickup, stack effects, banking/deposit, deposit-triggered hazards, curse, scoring/readability | **Implemented and verified** | Human testing confirmed deposit-triggered hazard mapping, long-staff intake reliability, Slosh-scaled slippery behavior, and the eight-bomb sector-spread cursed-orb bombardment alongside the existing vial/banking loop. Detailed behavior is in [CauldronCatastropheDesign.md](CauldronCatastropheDesign.md). |
| Normal match rotation | **Implemented and verified locally and in listen-server PIE** | Complete human playthroughs confirmed Party Hall between every Trial, Grand Wizard Final startup, and return to intermission. |
| Arena separation and start placement | **Implemented and verified locally** | Human playthroughs confirmed the separated runtime arenas and corrected transition placement. |
| Results/intermission gameplay cleanup | **Implemented and verified** | Results immediately neutralizes Cauldron hazards, movement effects, curse/banking state, bombs, arcs, vials, and callbacks while retaining only the inert transition floor. |

## Online and Steam

| Area | Status | Evidence |
| --- | --- | --- |
| Two-client PIE / direct-connect baseline | **Implemented and verified** | Human-observed separate-process direct-connect full loop plus a 2026-07-15 two-player listen-server PIE full-loop pass are recorded. |
| Prototype-arena phase visibility replication | **Implemented and verified** | Both listen-server PIE windows completed the full loop with correct arena handoffs after the actor-owned replicated phase-presentation contract was added. |
| Dirty-checked GameState/PlayerState mirrors | **Implemented and verified** | Unchanged PlayerState mirrors no longer force network updates, unchanged GameState fields are not rewritten, display timers update at 0.1-second granularity, progress mirrors at 1%, and empty event-feed clears no longer produce redundant sequences. The editor build and subsequent human validation both passed on 2026-07-15. |
| Stable PlayerState slot-first identity lookup | **Implemented and verified** | `WizardDisplaySlot` is preserved once assigned and is now the primary lookup for wizard, controller/spawn, replicated PlayerState, scoring, Favor, HUD identity, and Arcane Pinball readability paths. New PlayerStates receive the first unused nonnegative slot; `PlayerId` and controller order remain pre-assignment compatibility fallbacks. `WizardStaffEditor` built successfully and a subsequent two-player listen-server PIE session passed on 2026-07-15. Reconnect identity restoration remains deferred. |
| Steam host session creation | **Implemented and verified** | Prior host log and smoke records report Steam subsystem/session creation and listen travel. |
| Steam find/join/travel on two machines/two accounts | **Partially implemented** | Join helper is in current source. Same-machine multi-process testing hit Steam API limitations; no valid two-machine validation is recorded. |
| Steam private build installation | **Implemented and verified** | A private Steam build was installed and a full playthrough was reported working. |
| Current `private_test` playtest build | **Implemented and verified** | BuildID `24238419` (depot manifest `593837560765890906`) packaged, previewed, uploaded, and was assigned only to `private_test` on 2026-07-16. A Steam-installed human playthrough verified the complete current loop and in-game Party Hall standings board. `default` was intentionally left unchanged. |
| Runtime fallback lighting for packaged/private Steam build | **Implemented and verified** | Lighting bug was reported, fixed through a follow-up build, and confirmed by a later full playthrough. |
| Steam leaderboard/stat submission | **Implemented but unverified** | The C++ submit/flush scaffold and Steamworks leaderboard setup exist; no successful write/read-back test is recorded. |
| Production lobby/UI/matchmaking/invites/reconnect | **Planned / deferred** | Explicitly outside current scope. |

Focused unverified behavior is tracked once in [KNOWN_ISSUES.md](KNOWN_ISSUES.md); repeatable checks live in [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md).

## Deferred by Intent

- Production lobby/browser/matchmaking/friend invite UX.
- Robust reconnect, host migration, dedicated servers, and cross-internet/NAT validation.
- Client prediction, rewind, lag compensation, exact cross-client physics fidelity.
- Replicated loose snapped-segment physics.
- Production UI/UMG conversion, cosmetics, progression, shops, inventory, chat, voice, analytics.

See [ROADMAP.md](ROADMAP.md) for suggested next validation work and [KNOWN_ISSUES.md](KNOWN_ISSUES.md) for risks.
