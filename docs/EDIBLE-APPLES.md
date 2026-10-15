# Eating apples in LeafRP

This feature uses **vanilla Arma Reforger's built-in consumable gadget pipeline**, without any external mod dependency or new animation assets.

## How it works

- Each `Prefabs/Items/LeafRP_Apple.et` now has an `SCR_ConsumableItemComponent`, configured with `LRP_ConsumableEffectEatApple`.
- When a player uses an Apple from inventory and the underlying vanilla use action **finishes successfully**, the custom effect shows **"You ate an apple."** and the built-in consumable component removes that one inventory entity (`m_bDeleteOnUse = 1`).
- The animation currently reuses the vanilla `CMD_HealSelf` command and one-handed item animation setup; this is **not an eating-specific animation**.
- The effect is allowed even when the player has full health. Eating provides **no health/hunger bonus yet** because we have not introduced a survival system.
- The visual mesh is still the temporary inert vanilla M67 round-object placeholder, **not a grenade item**.
- New apples harvested from the existing Arland tree automatically use the updated edible prefab; no changes to the tree interaction or stock logic are needed.

## Testing in Workbench

1. Check out `feature/edible-apples`, reopen the addon in Workbench, and load `worlds/applePicking_Arland.ent`.
2. Pick **two** apples from the tree.
3. Open inventory, select one **Apple** and use the inventory **Use** action. This may **only equip the gadget** on the first press; close inventory and trigger the normal gadget-use/activate input while holding it. The exact input depends on your control bindings.
4. Confirm **"You ate an apple."** appears **only after finishing use**, and exactly **one** apple disappears from inventory.
5. Use the second apple, check that it disappears too.
6. Test an interrupted item-use animation; it should leave the apple intact because the built-in consumable cleanup is only called on successful completion.
7. Check that harvesting and 300-second regrowth still behave as before.

## Automated regression checks

The native `LRP_TEST_AppleHarvestSuite` checks the consumable component, delete-on-use effect, eligibility for healthy soldiers, inventory-use mask, gadget animation mode, and that `GetAnimationParameters` binds a valid self-use animation command.

**CI verified:** [11/11 native Reforger Autotests passed](https://github.com/Ezcoo/LeafRP-Arma-Reforger/actions/runs/38690295814) with no script compilation errors, test failures, or skipped tests. These tests do **not** simulate the player's inventory button presses or the completed use animation; the manual acceptance test covers those.

## If pressing Use appears to do nothing

- If the apple is merely placed in the character's hand, activate that held gadget after closing inventory.
- Check the current game `script.log` for `[LeafRP] Eat Apple animation request: started=...`. `started=0` means the engine rejected the animation start; `started=1` means it accepted it.
- `Eat Apple cannot start: no valid use animation command` means the self-use command was unavailable in that character's animation graph.
- `[LeafRP] Player ate one Apple.` indicates the consumable's successful completion callback ran. If an apple remains afterward, investigate item deletion/replication instead of animation triggering.
- If none of these logs appear, the inventory/gadget activation path did not invoke our custom effect at all.


## Known limitations

- No hunger, thirst or health effects (future reusable survival stats feature).
- No eating-specific animation or real apple mesh.
- Tree harvesting stock remains local to the session and is not yet a server-authoritative multiplayer economy.
- Using and deleting an item is based on vanilla Reforger inventory behavior; full multiplayer verification is still future work.
