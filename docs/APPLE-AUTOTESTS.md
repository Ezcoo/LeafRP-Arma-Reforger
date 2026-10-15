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

## GitHub Actions (experimental)

The Linux runner already verifies compilation with `scripts/validate-reforger-linux.sh`. This branch additionally tries `scripts/run-reforger-autotests.sh`, which launches the **game diagnostic executable** through Proton with `-autotest LRP_TEST_AppleHarvestSuite`, and expects a fresh `junit.xml` report.

The runner **fails closed** if the report is missing, test cases are missing, any test failed/skipped, or the process fails/times out. This experimental step must be validated against our particular Proton + Workbench installation before enabling it for `main` branches. Until then the manual Workbench test is the supported verification method.

## What remains

- Testing the player/tree action in a real scenario after this refactor.
- Physical apple inventory prefab and collecting it (not created in this PR).
- Authoritative shared stock on a dedicated server, persistence, anti-duplication and inventory-full handling.
- True end-to-end interactions with entities in the game world.

Official docs: https://community.bistudio.com/wiki/Arma_Reforger:Autotest_Framework
