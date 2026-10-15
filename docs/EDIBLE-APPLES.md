# Edible apples — LeafRP (vanilla only)

## Working design

**Inventory → Use on an Apple is now a direct, instant eat operation.** The vanilla UI passes this choice to `SCR_CharacterInventoryStorageComponent.UseItem(..., SCR_EUseContext.FROM_INVENTORY)`; we handle only apples in a targeted `modded` override, leaving all other items' behavior unchanged.

1. The selected entity must contain our `LRP_ConsumableEffectEatApple`.
2. It must actually be contained in the current character's managed inventory and the character must be alive.
3. Inventory **Use** schedules a callback about 100 ms later, leaving the item alive for vanilla `SCR_InventorySlotUI.UseItem` to finish its post-call `item.FindComponent` inspection (line 801 in the checked Reforger sources).
4. The delayed callback revalidates item ownership and requests deletion of this **one** apple entity with `TryDeleteItem`. If the request is accepted, LeafRP logs the action and shows **"You ate an apple."** The second apple, other items, and tree stock are not touched. Repeated Use presses cannot queue duplicate callbacks for the same item.

The direct inventory operation is **instant: no eating animation or interruption window**. This is deliberate, because the first implementation did nothing when the user pressed **Use**.

The separate vanilla held-gadget activation path still uses our existing `LRP_ConsumableEffectEatApple` and its placeholder `CMD_HealSelf` animation. The effect now supports both vanilla `CanApplyEffect(character, character)` (inventory UI) and `CanApplyEffect(character, appleItem)` (held gadget activation). For now, **use Inventory → Use**, not the held-gadget controls, as the acceptance path.

No hunger, thirst or health restoration yet. Those require a separate survival system. The item retains its harmless temporary visual model; no third-party dependencies or animation assets were introduced.

## Why the earlier version did nothing

The first consumable attempt never reached the effect. Vanilla `SCR_ConsumableItemComponent.ActivateAction` checks `CanApplyEffect(character, GetOwner())`, where `GetOwner()` is the **item**. Our original check required its second argument to be the character, so activation was silently rejected. Even after fixing this, Inventory → Use typically **equips/toggles a gadget** rather than directly consuming it.

The latest fix handles **both** problems and adds the actual inventory UI method to the game-side test suite.

## Crash regression: synchronous deletion during Inventory Use

The first direct-use version called `TryDeleteItem` **inside** `SCR_CharacterInventoryStorageComponent.UseItem`. Although this removed the Apple, vanilla `SCR_InventorySlotUI.UseItem` then immediately accessed `item.FindComponent` while updating consumable analytics. The deleted item pointer threw a **Virtual Machine Exception (NULL pointer to instance, variable item)**. The corrected override schedules the deletion after the UI call completes, with a pending-item guard and another inventory ownership check in the callback.

## Manual acceptance test

1. Check out `feature/edible-apples` and reopen the addon in Workbench.
2. Open `worlds/applePicking_Arland.ent` and harvest **two apples**.
3. Open inventory, focus one apple and choose **Use**.
4. Expect **"You ate an apple."** and the item to disappear a fraction of a second after using it. No animation is expected in this inventory shortcut.
5. Verify **the second apple remains** and can also be eaten.
6. Verify the original apple-tree stock and regrowth still work.

Diagnostics:
- `[LeafRP] Eat Apple: inventory Use queued safely after UI handler.` — the item remains alive while the native inventory UI finishes.
- `[LeafRP] Eat Apple: deferred deletion accepted.` — the delayed inventory operation accepted the deletion request.
- `[LeafRP] Player ate one Apple.` — notification path executed.
- `[LeafRP] Eat Apple: rejected inventory Use` — item wasn't in the current inventory, or eater invalid.
- `[LeafRP] Eat Apple: delayed inventory deletion request was rejected.` — the inventory couldn't delete the item.
- `[LeafRP] Eat Apple activation gate reached by native gadget.` — the **separate held-gadget** action reached the effect.

## Automated tests

The `LRP_TEST_AppleHarvestSuite` now contains **12 tests** in total. The added `LRP_TEST_AppleHarvest_EatUsingInventoryUse` test:

- Spawns a real soldier and two Apple entities in native inventory.
- Obtains the soldier's actual `SCR_CharacterInventoryStorageComponent`.
- Invokes `UseItem(firstApple, ESlotFunction.TYPE_GENERIC, SCR_EUseContext.FROM_INVENTORY)` — exactly the UI path.
- Requires that the item **remain accessible immediately after UseItem returns**, mirroring the native UI's following `item.FindComponent` call.
- Issues a second Use while deletion is pending to verify it cannot schedule duplicate consumption.
- **Waits until the consumed Apple actually disappears** from the character's inventory, and verifies that the **other apple remains**.

The existing consumable test also checks the native gadget's `CanApplyEffect(character, appleItem)` call convention.

**Validation:** the updated regression suite passed **12/12 game-side tests with zero failures or skips** after the deferred deletion fix: https://github.com/Ezcoo/LeafRP-Arma-Reforger/actions/runs/38695016110. Workbench compilation also reported no script errors. The test checks that `FindComponent` remains safe immediately after inventory Use and that the item subsequently disappears. A full in-game HUD smoke test remains essential.

## Limitations

- No dedicated eating animation from inventory (immediate consumption).
- No nutrition, medical effects, stamina effects, persistence, or server-authoritative multiplayer resource handling.
- `TryDeleteItem` returning true means the inventory system **accepted** the deletion request. Final UI state must be confirmed in-game.
