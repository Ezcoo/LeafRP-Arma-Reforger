# Virtual Fishing v0.4 — Wait for the Bite

This is a **local single-player gameplay prototype**. There is no persistent inventory, server authority or sound playback yet.

## Gameplay

1. Press **Start Fishing** once (instant action).
2. Wait a random **5–120 seconds**. A distance check every 250 ms cancels when more than 5 m from the starting position, or when player control is lost.
3. 70% of attempts produce a **BITE!** hint; 30% finish with no bite.
4. On bite, **Hook Fish!** appears for **1 second**. Trigger it before the window expires. Missed bites do not award fish.
5. Successful hooks resolve the existing v0.3 species/rarity/weight logic and update the statistics. A caught old boot remains a non-fish find.

## Workbench manual setup

1. Save local modifications, check out `feature/virtual-fishing-v04` and pull. Load the **LeafRP** addon (not RP_TEST).
2. In Workbench Edit mode, compile scripts. Check logs before starting Play mode.
3. Select the existing `LRP_FishingSpot` entity; expand its `ActionsManagerComponent`.
4. Retain `LRP_VirtualFishingUserAction` but change its **action duration from 15 seconds to 0** (instant trigger). The action label becomes **Start Fishing**.
5. Add a new `LRP_HookFishUserAction` to **Additional Actions**; set its **Parent Context List** to the existing `LRP_FishingSpot` context, `UIInfo` to **Hook Fish!**, and **duration to 0 seconds**. Place it in an immediately accessible action slot so the one-second reaction window is practical.
6. Keep `LRP_ViewFishingStatsUserAction` unchanged.
7. Save and manually enter Play mode.

## Acceptance tests

- [ ] Scripts compile successfully.
- [ ] Starting fishing is instant and does not permit a second concurrent attempt.
- [ ] A bite or no-bite happens after 5–120 seconds.
- [ ] **Hook Fish!** appears only during the bite and lasts around one second.
- [ ] Hooking during the window records a catch (fish or rare junk); missing does not.
- [ ] Moving >5 m away cancels fishing without a catch.
- [ ] Cancelling/restarting does not allow stale timers to award anything.
- [ ] Existing fish records and apple interactions still work.
- [ ] Sound playback is **not** yet implemented or verified.

## Sound integration later

The bite sound belongs in `LRP_VirtualFishingUserAction.LRP_Bite` at the `TODO` line. The Workbench audio event, sound project routing (including output bus), and spatial emitter configuration must be verified with the actual supplied files before code integration. Do not assume raw .wav/.ogg files will play directly.

## Prototype limitations

- Static state and callbacks are local to the script context; multiplayer server validation, per-player state and persistence are NOT provided.
- Reaching the fishing interaction during a one-second reaction window may be hard depending on the object's angle and action selection. Evaluate usability before merging.
- Changing editor action durations is required; compiling scripts alone is not enough.
