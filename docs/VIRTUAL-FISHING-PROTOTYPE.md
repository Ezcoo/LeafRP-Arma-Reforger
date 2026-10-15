# LeafRP Virtual Fishing v0.1

## Scope

This is a **local, no-assets prototype**: one designated coastal interaction spot, a 15-second hold, and a 70% chance of a virtual catch. The result appears as a LeafRP hint and in the game log. It does **not** create items, keep a fish counter, change health/hunger, or grant money.

## Workbench setup

1. Open the **LeafRP** addon (not the old RP_TEST project) and compile the Game scripts. Confirm `LRP_VirtualFishingUserAction` is visible.
2. Choose an **existing, editable object by the coast** in your test scenario (for example, an existing pier prop). Do not alter vanilla prefabs globally. This entity marks the one allowed fishing spot; there is no automatic water proximity check.
3. Add or use its `ActionsManagerComponent`. Create an action context, e.g. `LRP_FishingSpot`, with a `PointInfo` near where the player should stand and a practical context radius (~2 m). The entity must have suitable action interaction/physical geometry.
4. Add `LRP_VirtualFishingUserAction` under **Additional Actions** and link it to the same context via **Parent Context List**.
5. Add action `UIInfo` with name `Fish`. Configure the action's **duration/hold time to 15 seconds** in the Workbench action properties. Scripted outcome is deliberately separate from duration, which is configured in the editor.
6. Save the scenario/entity (and custom prefab, if used), run the test world, take control of a soldier, aim at the spot, and hold the action until it completes.

## Expected behavior

- The fishing action appears when looking at the configured spot.
- Holding the interaction for ~15 seconds completes one attempt.
- On completion: 70% `You caught a fish! (Virtual catch)`; 30% `No bite this time. Try again!`.
- The Workbench log reports `[LeafRP] Virtual fishing: ...`.
- Releasing the hold early should not award a result (subject to the action's duration/hold setup).
- No fish appears in the inventory. The catch is a notification only.

## Test checklist

- [ ] New class compiles successfully in Workbench.
- [ ] Action appears only at the configured interaction entity.
- [ ] Duration is ~15 seconds, not instantaneous.
- [ ] Cancelling before completion produces no catch hint.
- [ ] Completed attempts result in one of the two hints.
- [ ] Repeated attempts work.
- [ ] Apple pickup and inventory **Use** still behave as before.
- [ ] Multiplayer behavior is **not** claimed as authoritative.

## Limits and next phase

For v0.1 `HasLocalEffectOnlyScript()` is true, matching the existing LeafRP local apple-action prototype. Catches have no reliable multiplayer persistence or anti-cheat properties. Before adding a real fish tally, trade, or currency, move outcome generation and rewards to a server-authoritative system and separately test replication.

Do not automatically launch another Workbench instance, change Play mode, or use fallback validation as a substitute for actual compilation. Once Workbench is in Edit mode and the test project is saved, reload scripts with `wb_reload(target=scripts, allowValidationFallback=false)` through the local Enfusion MCP workflow, then inspect new logs.
