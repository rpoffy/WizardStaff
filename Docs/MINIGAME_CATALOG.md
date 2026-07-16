# Minigame Catalog

**Last Updated:** 2026-07-15

## Normal Rotation

### Party Hall

**Status:** Implemented and verified.

- Runtime intermission space and readiness/standings context.
- The server retains match/transition authority.
- Online hosts wait for real joining players rather than creating local-only online helpers.
- It is a transition state, not a production menu or lobby UI.

### Mug Run

**Status:** Implemented and verified.

**Purpose:** Grow staff segments and Mana Slosh by collecting mugs before the timer ends.

**Current systems:** server-owned mug collection/respawn, replicated pickup active state, staff count and Slosh readability, carried brew reward display, Arcane Pinball reward use, and event-feed milestones.

**Known validation boundary:** Core direct-connect behavior is recorded as working. Continue to verify timer/cleanup behavior whenever pickup, reward, or reset code changes.

### Staffs at Dawn

**Status:** Implemented and verified.

**Purpose:** Combat Trial built around Quick Bonk, Staff Clash, ring-outs, and staff-growth pressure.

**Current systems:** server-owned bonk/hit/readability scaffold, Clash resolution, ring-out/respawn, Staffs score/Favor attribution, powerup pickup state, Mega Staff readability and cleanup.

**Known validation boundary:** Existing direct-connect full-loop evidence supports the baseline; detailed feel and balance remain ongoing local playtest work.

### Grand Wizard Final

**Status:** Implemented and verified.

**Purpose:** Match-deciding Candidate control and steal phase.

**Current systems:** server-owned Candidate selection, timer, steal eligibility/progress, swaps, winner selection, ritual/readability state, and bounded event-feed messages.

**Known validation boundary:** Readability and reset were specifically hardened in earlier online work. Preserve server authority and clear state at results/rematch boundaries.

## Normal-Rotation Trial

### Cauldron Catastrophe

**Status:** Implemented and verified in the normal rotation after Staffs at Dawn.

**Purpose:** A more volatile staff-conversion Trial involving vials, cauldron intake/banking, deposits, hazards, sticky/slippery effects, and a cursed orb.

**Current code evidence:** `WizardStaffCauldronArena`, `WizardStaffCauldronVialPickup`, `WizardStaffCauldronHazard`, `WizardStaffCauldronDepositArc`, and associated wizard/GameMode logic exist.

**Recorded design/playtest direction:**

- Vials attach in a stack and apply the top vial's effect.
- Banking converts staff segments through a staged cauldron interaction.
- Segment deposits have an explicit visual arc/readability path.
- Each successful Speed or Burdening Power vial deposit has a server-owned 25% chance to eject its matching hazard: Speed -> slippery, Burdening Power -> sticky. Sticky uses a mild tether/reel direction.
- A curse/orb is intended to be visually associated with the affected wizard or snapped segment and to create readable risk.
- Arena is an open octagon intended to support ring-outs.

**Validation boundary:** Local and two-player listen-server full-loop transitions are verified. Focused human testing also confirmed deposit-hazard mapping/frequency, slippery behavior, long-staff intake reliability, and cursed-orb bombardment.

See [CauldronCatastropheDesign.md](CauldronCatastropheDesign.md) for detailed tuning and [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md) for recommended validation.

## Retained / Legacy Components

`WizardStaffCauldronIngredient` exists in the source tree. Historical task records mention an early ingredient/bonk-to-cauldron concept. Current code/design documentation emphasizes vials and banking; classify any live use of the ingredient actor as implementation detail or legacy until a call-site review proves it remains a current player-facing mechanic.
