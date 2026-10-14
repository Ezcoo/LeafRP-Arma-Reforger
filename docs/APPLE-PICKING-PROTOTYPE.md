# LeafRP: pick apples from a tree (phase 1)

**Dependencies:** vanilla Arma Reforger only; no ACE or other Workshop mods.

## What this prototype does

Walking to a configured apple tree exposes a contextual **Pick apple (5 left)** action. Each use adds one **virtual apple** to the local player's session counter and shows a LeafRP hint. A tree begins with five apples (configurable) and stops offering the action when empty.

**This is not yet an inventory item or a multiplayer farming/economy system.** There is no physical apple prefab, eating, trading, persistence, server-authoritative stock, or replication. Do not rely on the virtual tally for multiplayer.

## Add an interactive apple tree in Workbench

The script alone does not attach actions to every tree in the base game; it must be configured on a **placeable, editable tree prefab or scene entity**. This is especially important with world vegetation instances, which may not be editable prefab entities.

1. Check out `feature/apple-tree-picking`, open LeafRP in Workbench, and compile scripts (Shift+F7). Ensure the LeafRP project is among the loaded addons.
2. Open **Resource Manager** and search the vanilla project for an apple tree (try `apple` and the botanical name `Malus`). Pick a **placeable prefab/entity**; avoid modifying any vanilla prefab directly. Copy or inherit it into your **LeafRP** addon, or prepare an editable tree entity in your test world.
3. Open your own tree prefab in Prefab Editor, or place the editable tree in World Editor and select the tree entity.
4. Add **ActionsManagerComponent** to the tree if one is not already present.
5. In **Action Contexts**, add one context named `LRP_ApplePick`. Set its **Position** using `PointInfo` near the tree trunk (e.g., roughly 1 m above ground), and set a practical context **Radius** (try 2 m initially).
6. Under **Additional Actions**, add **LRP_PickAppleUserAction**.
7. Under the action's **Parent Context List**, add `LRP_ApplePick`. Add an action **UIInfo** (name `Pick apple`); set visibility distance to roughly 2–3 m and duration to 1–2 s for a short picking progress interaction. You can adjust the action's **Number of apples available on this tree** (default 5).
8. If the tree does not appear in the interaction system, make sure the prefab has the required **physical representation / interaction geometry** (see Bohemia's Action Context Setup wiki). For a production synchronized action it will also require a suitable `RplComponent`.
9. **Apply To Prefab** (if editing a prefab instance) and save. Place your **LeafRP** tree into the test world.
10. Run the world, take control of a soldier, approach the tree trunk, look at the action context, and hold your normal interaction key to pick an apple.

## Expected behavior

- The nearby action says `Pick apple (5 left)`.
- Each completed use shows a LeafRP hint `Picked an apple! Collected: N`.
- The action label counts down to zero, and then disappears.
- Workbench gameplay logs contain `[LeafRP] Apple tree initialized`, `Apple picked` and `Player now holds`.

## Troubleshooting

- **No action on the tree:** Make sure you edited an entity/prefab rather than non-interactive terrain vegetation, that `ActionsManagerComponent` has a valid action context, that the scripted action is in Additional Actions *and* points at the same context, and that the tree has working interaction geometry.
- **Action says a different name or does not appear:** Confirm scripts were recompiled and `LRP_PickAppleUserAction` is the chosen class.
- **Action appears but nothing happens:** Check `[LeafRP]` logs. You must be controlling the interacting character in the local client.
- **Tree empties after five apples:** Intended; the stock does not replenish until the local level is reset.
- **Works locally but differs on clients/servers:** Expected for this phase, because both harvest counts are local only.

## Planned next steps (if desired)

1. Replace virtual tally with a native LeafRP apple inventory prefab.
2. Make the tree stock server-authoritative and synchronized; prevent double-picking and add replenishment timers.
3. Add eating and/or selling apples as gameplay.
4. Optionally package a ready-to-place tree prefab created and validated in Workbench.

Official documentation: [Action Context Setup](https://community.bistudio.com/wiki/Arma_Reforger:Action_Context_Setup) and [Creating a User Action](https://community.bistudio.com/wiki/Arma_Reforger:Arma_Reforger:Creating_a_User_Action).
