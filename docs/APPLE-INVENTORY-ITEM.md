# LeafRP: physical Apple inventory item

This is a **standalone** vanilla-only inventory prototype stacked on the working apple-harvest PR #9.

## What changes

- Picking an apple uses `InventoryStorageManagerComponent.CanInsertResource` to verify space and `TrySpawnPrefabToStorage` to insert one **real prefab entity** into the controlled character's native inventory.
- Only a successful insert consumes one unit of tree stock. If inventory is full or missing, no apple is removed from the tree.
- The old *virtual apple counter* has been removed; open the real inventory to see the item.
- The new item, `Prefabs/Items/LeafRP_Apple.et`, is labelled **Apple**, uses `InventoryItemComponent` with `FOOD` item type, and is compact (approx. 150 g and 1x1 inventory slot).
- Tree stock and regrowth remain local/session-only. **No server-authoritative multiplayer** is claimed.
- Eating, sale, perishability, and persistence are not implemented.

## Appearance disclaimer

The prototype currently references a **vanilla M67 grenade mesh** as a temporary round object. **This item is inert**, since the prefab derives from the vanilla inert `Attachment_Base.et`, not from a live grenade prefab. It should display in inventory but does **not yet look like an apple**. Swapping the mesh for a dedicated apple model is a separate art task; no third-party assets or dependencies were introduced.

## Tests

Seven existing harvest/prefab tests plus two new game-side tests:

- `LRP_TEST_AppleHarvest_InventoryApplePrefab` ensures the LeafRP item resource loads and spawns as an entity with `InventoryItemComponent`.
- `LRP_TEST_AppleHarvest_AppleIsInsertedIntoCharacterInventory` checks a vanilla soldier has inventory capacity for apples and accepts `TrySpawnPrefabToStorage`.

Tests use Reforger's built-in Autotest Framework and run through the existing Linux/Proton CI. **All 9 tests passed** (0 failures, 0 skipped) in [GitHub Actions run 38688928970](https://github.com/Ezcoo/LeafRP-Arma-Reforger/actions/runs/38688928970). A passing test confirms storage accepted the insertion request; the final in-game check must verify an apple actually appears in inventory, as some inventory operations can complete asynchronously.

## In-game manual test

1. Check out `feature/apple-inventory-items`, then reopen the **LeafRP** project in Workbench so its resource database discovers the new `.et` and `.meta`.
2. Load `worlds/applePicking_Arland.ent`.
3. Approach the tree, trigger **Pick apple**, and open the native inventory. Expect an item named **Apple** with the temporary model.
4. Pick five apples, confirm they appear as item entries and the tree becomes empty.
5. Fill your inventory or remove storage capacity and verify you cannot harvest another apple when there's no room.
6. If the prefab fails to load, verify `Prefabs/Items/LeafRP_Apple.et` appears in Resource Browser and rebuild the addon resource database if necessary.

Runtime logs to search: `[LeafRP] Real apple item added to inventory` and `[LeafRP] Apple picking aborted: inventory insertion failed`.

Official inventory API reference: https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceInventoryStorageManagerComponent.html
