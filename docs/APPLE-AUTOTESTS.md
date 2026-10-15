# LeafRP: automated apple-picking tests (built-in framework)

No external runtime mod dependencies. We use the built-in **Arma Reforger Autotest Framework**, not NUnit/xUnit or ACE.

## Current feature behavior

- Each configured apple-tree action uses an independent `LRP_AppleHarvestStock` state object.
- The tree starts with **5 apples** (editable action property).
- Each successful action reduces the remaining stock and increments the **local, virtual** apple count.
- Once emptied, the action disappears. With the default **Regrow seconds = 300**, the tree refills 5 minutes after the last apple is picked. Set **Regrow seconds = 0** to disable.
- Tree stock and the player's virtual apple count remain **local session state**. The script does **not** spawn a physical inventory apple, synchronize multiplayer state, or persist across a restart. Those are separate features requiring more design and testing.

## Why a separate state class?

`scripts/Game/LeafRP/LRP_AppleHarvestStock.c` handles counts, exhaustion and regrowth without any knowledge of entities or Workbench UI. The real `LRP_PickAppleUserAction` uses that class; our automated tests call exactly the same class with controlled timestamps.

## Seven automated test cases

File: `scripts/Game/Tests/LRP_AppleHarvestTests.c`

Suite: **`LRP_TEST_AppleHarvestSuite`**

- `LRP_TEST_AppleHarvest_StartsFull`
- `LRP_TEST_AppleHarvest_PickReducesStock`
- `LRP_TEST_AppleHarvest_EmptyRejectsHarvest`
- `LRP_TEST_AppleHarvest_RegrowsAfterDelay`
- `LRP_TEST_AppleHarvest_DisabledRegrowth`
- `LRP_TEST_AppleHarvest_TreePrefabRegistersContext` (real prefab and action context registration)
- `LRP_TEST_AppleHarvest_TreeActionVisibleToCharacter` (visible to a spawned vanilla character)

The tests are compiled in diagnostic builds under `ENABLE_DIAG`, allowing both Workbench and the diagnostic game test runner to discover them while avoiding test classes in release-game code.

## Run the tests manually in Workbench (recommended first check)

1. Check out `feature/apple-autotests`; reopen the LeafRP project in Enfusion Workbench.
2. Open **Script Editor**, search for `LRP_AppleHarvestTests.c`, and locate the `LRP_TEST_AppleHarvestSuite` class.
3. Hover the class declaration or put the caret inside it and select **Plugins → Run test**.
4. Workbench opens World Editor for the test. The default test world should load automatically, and a test-results dialog should appear after the suite completes.
5. Confirm **7 succeeded, 0 failed**. Inspect `autotest.log` and `junit.xml` in the latest Workbench logs if necessary.

Alternative: in **World Editor**, open **Autotest Tool**, enter `LRP_TEST_AppleHarvestSuite`, then select **Run class**.

## GitHub Actions (verified)

The Linux runner compiles scripts through `scripts/validate-reforger-linux.sh`, then launches the real **ArmaReforgerSteamDiag.exe** through Proton using `-autotest LRP_TEST_AppleHarvestSuite`.

The runner verifies a **fresh** `junit.xml` report containing all seven expected tests, with no errors, failures or skips. It fails the workflow if the suite did not run or the game times out (even if Proton exits with code 0). The reports and logs are uploaded as `apple-autotest-results` artifacts.

Original five stock tests verified against https://github.com/Ezcoo/LeafRP-Arma-Reforger/actions/runs/38680240538. The two new prefab integration tests are being validated separately.

After merging, the regular `main`-branch GitHub Actions workflow will execute **both compilation and these tests** for every relevant push. Pull requests do not automatically execute remote code on the user's machine until merged, preserving our existing self-hosted runner trust boundary.

### Diagnosing failures

- Compiler fail: inspect `workbench-logs/script.log`.
- Autotest doesn't launch: inspect `apple-autotest-results/ArmaReforger/logs/.../script.log`; `Invalid -autotest parameter value` means the test class was not discovered in the diagnostic game build.
- Test case fail: open `junit.xml`, which lists failing assertions.
- No fresh JUnit report: the run is **not** counted as passing.

## Troubleshooting: apple interaction disappeared after refactor

The initial refactor checked whether its runtime stock object existed inside `CanBeShownScript` and hid the action if it did not. The updated action initializes its stock on first use/query and logs stock initialization and empty-tree visibility decisions.

1. Confirm the **same playable sub-scene** and saved editable tree prefab are loaded in Workbench; the `.et` prefab and Arland sub-scene are now present in this branch; ensure the scene opens with the LeafRP addon loaded.
2. On the tree's `ActionsManagerComponent`, confirm the action is still **LRP_PickAppleUserAction**, its parent context still matches `LRP_ApplePick`, and the context is positioned near the trunk.
3. In the action's properties, check **Number of apples available per harvest** (should be `5`, not `0`) and **Regrow seconds** (start with `10` for rapid testing).
4. Reopen/reload the world after recompiling scripts. Approach the tree as the controlled soldier.
5. Search the **live gameplay** `script.log` for `[LeafRP] Apple action`. If it says `starting=0`, fix the prefab's serialized value. If it says `hidden: remaining=0`, the tree is currently depleted.
6. If there are **no** `Apple action` logs, Workbench is probably not instantiating the action at all; investigate whether the edited prefab, scenario sub-scene, action context or mod project is loaded.

Our new tests cover actual prefab action context registration and scripted visibility in addition to harvest/regrowth logic, but do **not** simulate the HUD interaction selection or player input. The edited tree prefab and Arland sub-scene are now committed in this branch. The new suite directly instantiates the real tree prefab and checks action registration and visibility. A complete on-screen interaction test with actual player input is still separate.

## What remains

- Testing the player/tree action in a real scenario after this refactor.
- Physical apple inventory prefab and collecting it (not created in this PR).
- Authoritative shared stock on a dedicated server, persistence, anti-duplication and inventory-full handling.
- True end-to-end interactions with entities in the game world.

Official docs: https://community.bistudio.com/wiki/Arma_Reforger:Autotest_Framework
