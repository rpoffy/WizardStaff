# Known Issues and Verification Gaps

**Last Updated:** 2026-07-16

Items are categorized so untested work is not confused with a confirmed bug.

## Validation Gaps

| Item | Status | Why it matters |
| --- | --- | --- |
| Steam session search/join/travel between two machines | **Blocked by environment / unverified** | Same-machine multi-process testing hit Steam API initialization limitations. A two-machine/two-account test is required. |
| Steam leaderboard write, flush, and read-back | **Implemented but unverified** | Existing code queues the Favor result only for an active Steam session. Private Steam validation is still required. |
| Automated test coverage | **Not found in repository inventory** | Current confidence relies on build output, manual PIE, direct-connect, and Steam smoke tests. |

## Design Tensions

| Tension | Status | Required care |
| --- | --- | --- |
| Emergent physics versus scripted staff obstruction recovery | **Open design question** | The desired game identity favors natural staff problems, while current code contains stuck-state and recovery/failsafe behavior. Do not silently remove it; gather playtest evidence and obtain explicit direction. |
| Steam branch history versus production readiness | **Expected limitation** | A private build can install and run while Steam join, stats, tester access, store presence, and release requirements remain incomplete. |

## Not Bugs: Intentional Deferrals

The following absences are deliberate, not defects: public lobby browser, matchmaking, friend invites, production lobby UI, reconnect UX, host migration, dedicated servers, replicated loose segment physics, prediction/rewind/lag compensation, final UI/cosmetics/progression, and extra approved gameplay content.

For test steps, see [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md). For suggested sequencing, see [ROADMAP.md](ROADMAP.md).
