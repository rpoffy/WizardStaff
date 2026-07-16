# Roadmap

**Last Updated:** 2026-07-15
**Rule:** This sequencing guide does not approve implementation by itself.

## Current Baseline

The three-Trial loop, focused Cauldron mechanics, and cleanup fixes are human-verified. The completed cleanup sequence covered Cauldron Results teardown, replicated arena presentation, transition responsibility separation, mirror dirty-checking, and stable PlayerState slot identity.

## Recommended Near-Term Order

1. **Private Steam leaderboard validation.** In an active Steam-hosted session, confirm authoritative queue, flush, KeepBest behavior, and Steamworks read-back.
2. **Steam session validation when equipment permits.** Use two machines/two accounts for host/search/join/travel. Do not select a new net driver without observed connect-string/travel evidence.
3. **Private tester preparation.** Keep instructions, branch/package access, known limitations, and feedback questions concise; run local, listen-server, and installed-build sanity checks before sharing.

## Decision Gates

| Gate | Required evidence |
| --- | --- |
| Rely on Steam leaderboard data | Successful private Steam write, flush, KeepBest retry, and read-back. |
| Treat Steam join as viable | Two-machine/two-account host, search, join, travel, possession, and Mug Run start. |
| Begin production online UX | Stable direct-connect/Steam coexistence plus an explicit product/UI decision. |

## Deferred Until Explicitly Approved

- Production lobby/browser/matchmaking/friend invites and reconnect/host migration/dedicated servers.
- Prediction, rewind, lag compensation, exact cross-client physics, and replicated loose debris.
- New Trials, spells, rewards, powerups, Hammer Time, broad retuning, progression, inventory, shops, cosmetics, chat, voice, and analytics.
- UMG/final UI, final art, audio, and production VFX passes.

See [CURRENT_STATE.md](CURRENT_STATE.md) for present evidence and [PLAYTEST_PLAN.md](PLAYTEST_PLAN.md) for repeatable checks.
