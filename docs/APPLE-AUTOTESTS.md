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

## Five automated test cases

File: `scripts/Game/Tests/LRP_AppleHarvestTests.c`

Suite: **`LRP_TEST_AppleHarvestSuite`**

- `LRP_TEST_AppleHarvest_StartsFull`
- `LRP_TEST_AppleHarvest_PickReducesStock`
- `LRP_TEST_AppleHarvest_EmptyRejectsHarvest`
- `LRP_TEST_AppleHarvest_RegrowsAfterDelay`
- `LRP_TEST_AppleHarvest_DisabledRegrowth`

The tests are compiled in diagnostic builds under `ENABLE_DIAG`, allowing both Workbench and the diagnostic game test runner to discover them while avoiding test classes in release-game code.

## Run the tests manually in Workbench (recommended first check)

1. Check out `feature/apple-autotests`; reopen the LeafRP project in Enfusion Workbench.
2. Open **Script Editor**, search for `LRP_AppleHarvestTests.c`, and locate the `LRP_TEST_AppleHarvestSuite` class.
3. Hover the class declaration or put the caret inside it and select **Plugins → Run test**.
4. Workbench opens World Editor for the test. The default test world should load automatically, and a test-results dialog should appear after the suite completes.
5. Confirm **5 succeeded, 0 failed**. Inspect `autotest.log` and `junit.xml` in the latest Workbench logs if necessary.

Alternative: in **World Editor**, open **Autotest Tool**, enter `LRP_TEST_AppleHarvestSuite`, then select **Run class**.

## GitHub Actions (verified)

The Linux runner compiles scripts through `scripts/validate-reforger-linux.sh`, then launches the real **ArmaReforgerSteamDiag.exe** through Proton using `-autotest LRP_TEST_AppleHarvestSuite`.

The runner verifies a **fresh** `junit.xml` report containing all five expected tests, with no errors, failures or skips. It fails the workflow if the suite did not run or the game times out (even if Proton exits with code 0). The reports and logs are uploaded as `apple-autotest-results` artifacts.

Verified against GitHub Actions run https://github.com/Ezcoo/LeafRP-Arma-Reforger/actions/runs/38680240538: **5 cases, 0 failures, 0 skipped**.

After merging, the regular `main`-branch GitHub Actions workflow will execute **both compilation and these tests** for every relevant push. Pull requests do not automatically execute remote code on the user's machine until merged, preserving our existing self-hosted runner trust boundary.

### Diagnosing failures

- Compiler fail: inspect `workbench-logs/script.log`.
- Autotest doesn't launch: inspect `apple-autotest-results/ArmaReforger/logs/.../script.log`; `Invalid -autotest parameter value` means the test class was not discovered in the diagnostic game build.
- Test case fail: open `junit.xml`, which lists failing assertions.
- No fresh JUnit report: the run is **not** counted as passing.

## What remains

- Testing the player/tree action in a real scenario after this refactor.
- Physical apple inventory prefab and collecting it (not created in this PR).
- Authoritative shared stock on a dedicated server, persistence, anti-duplication and inventory-full handling.
- True end-to-end interactions with entities in the game world.

Official docs: https://community.bistudio.com/wiki/Arma_Reforger:Autotest_Framework
