# Wizard Staff Agent Guide

Read [Docs/PROJECT_OVERVIEW.md](Docs/PROJECT_OVERVIEW.md) and [Docs/CURRENT_STATE.md](Docs/CURRENT_STATE.md) before substantial work. They are the short entry point for the project's current reality.

Then read the relevant specialist document before changing a system:

- Game rules, pillars, or player-facing behavior: [Docs/GAME_DESIGN.md](Docs/GAME_DESIGN.md)
- Runtime, replication, Steam, maps, or ownership: [Docs/TECHNICAL_ARCHITECTURE.md](Docs/TECHNICAL_ARCHITECTURE.md)
- Trial-specific work: [Docs/MINIGAME_CATALOG.md](Docs/MINIGAME_CATALOG.md) and, for Cauldron Catastrophe, [Docs/CauldronCatastropheDesign.md](Docs/CauldronCatastropheDesign.md)
- Historical rationale: [Docs/DESIGN_DECISIONS.md](Docs/DESIGN_DECISIONS.md)
- Testing: [Docs/PLAYTEST_PLAN.md](Docs/PLAYTEST_PLAN.md)
- Active risk and next work: [Docs/KNOWN_ISSUES.md](Docs/KNOWN_ISSUES.md) and [Docs/ROADMAP.md](Docs/ROADMAP.md)

## Operating Rules

1. Treat current source, configuration, build output, and recorded human observations as the primary evidence. Record contradictions with older documents instead of silently resolving them.
2. Preserve established design decisions and the locked normal match loop unless the user explicitly asks for a change.
3. Prefer extending a working system over replacing it without a demonstrated reason.
4. Keep server authority separate from replicated/readable client presentation. Do not let UI, map actors, replicated mirrors, or Steam metadata control gameplay.
5. Preserve local one-human-plus-bot and couch workflows while changing online paths.
6. The worktree may contain intentional uncommitted changes. Never discard or revert unrelated work.
7. After meaningful implementation changes, update the existing affected entries in `CURRENT_STATE.md`, `DESIGN_DECISIONS.md`, `KNOWN_ISSUES.md`, and the relevant specialist document. Do not append prompt history, duplicate the same narrative across files, or create a new planning document when an existing one fits. Update `OnlineMultiplayerArchitecturePlan.md` only for an explicitly requested major online milestone.
8. Run proportionate validation after changes. Record what was built, observed, and not tested; never present unverified work as verified.

## Status Vocabulary

Use these terms consistently in project docs:

- **Implemented and verified**: present in the repository and supported by a recorded build or human test.
- **Implemented but unverified**: present in source/configuration but no relevant recorded test confirms the behavior.
- **Partially implemented**: a working portion exists, but a required path, boundary, or validation remains incomplete.
- **Planned**: explicitly accepted future work not yet implemented.
- **Suggested but not approved**: an idea worth preserving, but not authorized work.
