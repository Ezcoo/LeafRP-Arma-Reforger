# LeafRP Virtual Fishing v0.2

This extends the **tested v0.1 virtual fishing spot** with fish species, weights, and locally maintained session statistics. The existing fishing action, 15-second hold duration, and 70% chance of success remain unchanged.

## Catch generation

After a successful attempt:
- **Perch:** 50% of successful catches, 0.2–1.6 kg in 0.1 kg increments
- **Herring:** 35% of successful catches, 0.1–0.6 kg in 0.1 kg increments
- **Pike:** 15% of successful catches, 1.0–12.0 kg in 0.1 kg increments

Catches show a hint with the species, weight, total caught fish, accumulated weight, and largest fish weight. A new personal best adds another line.

The statistics are **client-local static script state**, shared across all fishing spots in that running script context. They are **not persistent**, **not guaranteed to reset on player respawn**, and **not independently stored per player on a multiplayer server**. Reloading scripts or restarting the game can reset them. No real item, money, or persistence is involved. Before multiplayer release, replace this with correctly scoped authoritative state.

## Updating your local setup

1. Ensure your local work is saved or committed, then switch to `feature/virtual-fishing-v02` and pull.
2. Open the active **LeafRP** addon (not the former RP_TEST project).
3. Keep the existing fishing spot and its `LRP_VirtualFishingUserAction`. There is no new prefab or action to place.
4. Keep the existing hold duration at 15 seconds.
5. In Workbench Edit mode, compile scripts. If using Enfusion MCP, after confirming saved editor work, invoke **only** `wb_reload(target="scripts", allowValidationFallback=false)`; do not retry automatically or launch additional Workbench instances.
6. Review compilation diagnostics and new logs, then enter Play mode manually and test.

## Acceptance tests

- [ ] `LRP_VirtualFishingUserAction.c` compiles without errors.
- [ ] The existing action still appears at the coastal spot.
- [ ] Cancelled fishing attempt produces no reward.
- [ ] Completed attempts produce successes and failures, approximately 70/30 over many trials.
- [ ] Successful catches report perch, herring, or pike and a weight in the listed range.
- [ ] Catch count rises by one for each success only.
- [ ] Total weight accumulates correctly and largest catch never decreases.
- [ ] New record announcement appears only when a fish exceeds the old record.
- [ ] Apple picking and eating still function.
- [ ] Do **not** claim multiplayer authority, per-player persistent statistics, or server validation.

## Notes

The previous v0.1 documentation remains at `docs/VIRTUAL-FISHING-PROTOTYPE.md`.
