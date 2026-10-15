// Workbench Autotest framework: run suite LRP_TEST_AppleHarvestSuite.
// Uses Reforger's Autotest APIs in diagnostic Workbench and game builds.
// Tests use a pure state object and do not need a player or an apple-tree prefab.
#ifdef ENABLE_DIAG

class LRP_TEST_AppleHarvestSuite : SCR_AutotestSuiteBase
{
}

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 5)]
class LRP_TEST_AppleHarvest_StartsFull : SCR_AutotestCaseBase
{
	[TestStep(TestStage.Main)]
	void Execute_CheckInitialStock()
	{
		LRP_AppleHarvestStock stock = new LRP_AppleHarvestStock();
		stock.Reset(5, 300);
		AssertTrue(stock.GetCapacity() == 5, "New tree capacity should be 5");
		AssertTrue(stock.GetRemaining() == 5, "New tree should start with 5 apples");
		AssertTrue(stock.GetRegrowAt() == 0, "New tree should have no regrow deadline");
	}
}

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 5)]
class LRP_TEST_AppleHarvest_PickReducesStock : SCR_AutotestCaseBase
{
	[TestStep(TestStage.Main)]
	void Execute_Pick()
	{
		LRP_AppleHarvestStock stock = new LRP_AppleHarvestStock();
		stock.Reset(5, 300);
		AssertTrue(stock.TryPick(1000), "First apple should be pickable");
		AssertTrue(stock.GetRemaining() == 4, "Picking one apple should leave four");
		AssertTrue(stock.GetRegrowAt() == 0, "A nonempty tree should not be regrowing");
	}
}

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 5)]
class LRP_TEST_AppleHarvest_EmptyRejectsHarvest : SCR_AutotestCaseBase
{
	[TestStep(TestStage.Main)]
	void Execute_EmptyTree()
	{
		LRP_AppleHarvestStock stock = new LRP_AppleHarvestStock();
		stock.Reset(2, 300);
		AssertTrue(stock.TryPick(1000), "First apple should be pickable");
		AssertTrue(stock.TryPick(1001), "Second apple should be pickable");
		AssertTrue(!stock.TryPick(1002), "An empty tree must refuse another apple");
		AssertTrue(stock.GetRemaining() == 0, "Stock must not go below zero");
		AssertTrue(stock.GetRegrowAt() == 1301, "Deadline must start after last apple");
	}
}

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 5)]
class LRP_TEST_AppleHarvest_RegrowsAfterDelay : SCR_AutotestCaseBase
{
	[TestStep(TestStage.Main)]
	void Execute_Refill()
	{
		LRP_AppleHarvestStock stock = new LRP_AppleHarvestStock();
		stock.Reset(1, 30);
		AssertTrue(stock.TryPick(100), "Only apple should be pickable");
		stock.Update(129);
		AssertTrue(stock.GetRemaining() == 0, "Regrowth must not finish early");
		AssertTrue(!stock.TryPick(129), "Still empty before deadline");
		stock.Update(130);
		AssertTrue(stock.GetRemaining() == 1, "Stock should refill at deadline");
		AssertTrue(stock.GetRegrowAt() == 0, "Regrowth deadline should clear");
		AssertTrue(stock.TryPick(131), "Refilled apple should be pickable");
	}
}

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 5)]
class LRP_TEST_AppleHarvest_DisabledRegrowth : SCR_AutotestCaseBase
{
	[TestStep(TestStage.Main)]
	void Execute_NoRefill()
	{
		LRP_AppleHarvestStock stock = new LRP_AppleHarvestStock();
		stock.Reset(1, 0);
		AssertTrue(stock.TryPick(100), "Only apple should be pickable");
		stock.Update(2000000);
		AssertTrue(stock.GetRemaining() == 0, "Stock should remain empty when regrowth is disabled");
		AssertTrue(!stock.TryPick(2000001), "Exhausted tree should keep rejecting picks");
	}
}

#endif
