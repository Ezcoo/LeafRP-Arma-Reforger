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


// Runtime prefab integration tests: exercise the registered ActionsManager,
// context routing and action visibility, not only the harvest-stock model.
// GUID points to the LeafRP prefab in this repository, not the vanilla tree.
[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 25)]
class LRP_TEST_AppleHarvest_TreePrefabRegistersContext : SCR_AutotestCaseBase
{
	[TestStep(TestStage.Main)]
	void Execute_PrefabActions()
	{
		Resource prefab = Resource.Load("{354709C8FFC6DEC2}Prefabs/Vegetation/Tree/t_malus_domestica/t_malus_domestica_2s.et");
		AssertTrue(prefab.IsValid(), "LeafRP apple tree prefab must be a valid resource");
		if (!prefab.IsValid())
			return;

		IEntity tree = GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld());
		AssertTrue(tree != null, "Apple tree prefab must spawn");
		if (!tree)
			return;

		ActionsManagerComponent manager = ActionsManagerComponent.Cast(tree.FindComponent(ActionsManagerComponent));
		AssertTrue(manager != null, "Apple tree must contain ActionsManagerComponent");
		if (!manager)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(tree);
			return;
		}

		UserActionContext context = manager.GetContext("LRP_ApplePick");
		AssertTrue(context != null, "Tree must have LRP_ApplePick context");
		if (!context)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(tree);
			return;
		}

		array<BaseUserAction> contextActions = {};
		context.GetActionsList(contextActions);
		LRP_PickAppleUserAction appleAction;
		foreach (BaseUserAction candidate : contextActions)
		{
			appleAction = LRP_PickAppleUserAction.Cast(candidate);
			if (appleAction)
				break;
		}

		AssertTrue(appleAction != null, "LRP_PickAppleUserAction must be linked to the LRP_ApplePick context");
		if (appleAction)
		{
			AssertTrue(appleAction.GetUIInfo() != null, "Apple-picking action must have UIInfo");
			string name = appleAction.GetActionName();
			AssertTrue(name.Contains("Pick apple"), "Apple action name should be visible to the player");
		}

		SCR_EntityHelper.DeleteEntityAndChildren(tree);
	}
}

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 25)]
class LRP_TEST_AppleHarvest_TreeActionVisibleToCharacter : SCR_AutotestCaseBase
{
	[TestStep(TestStage.Main)]
	void Execute_Visibility()
	{
		Resource treePrefab = Resource.Load("{354709C8FFC6DEC2}Prefabs/Vegetation/Tree/t_malus_domestica/t_malus_domestica_2s.et");
		Resource characterPrefab = Resource.Load("{26A9756790131354}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Rifleman.et");
		AssertTrue(treePrefab.IsValid(), "Apple tree prefab should load");
		AssertTrue(characterPrefab.IsValid(), "Vanilla US Army character prefab should load");
		if (!treePrefab.IsValid() || !characterPrefab.IsValid())
			return;

		IEntity tree = GetGame().SpawnEntityPrefab(treePrefab, GetGame().GetWorld());
		IEntity entity = GetGame().SpawnEntityPrefab(characterPrefab, GetGame().GetWorld());
		AssertTrue(tree != null, "Apple tree should spawn");
		AssertTrue(ChimeraCharacter.Cast(entity) != null, "Vanilla character should spawn");
		if (!tree || !ChimeraCharacter.Cast(entity))
		{
			if (entity)
				SCR_EntityHelper.DeleteEntityAndChildren(entity);
			if (tree)
				SCR_EntityHelper.DeleteEntityAndChildren(tree);
			return;
		}

		ActionsManagerComponent manager = ActionsManagerComponent.Cast(tree.FindComponent(ActionsManagerComponent));
		AssertTrue(manager != null, "Tree must have action manager");
		LRP_PickAppleUserAction action;
		if (manager)
		{
			array<BaseUserAction> actions = {};
			manager.GetActionsList(actions);
			foreach (BaseUserAction candidate : actions)
			{
				action = LRP_PickAppleUserAction.Cast(candidate);
				if (action)
					break;
			}
		}

		AssertTrue(action != null, "Tree must register apple-picking action");
		if (action)
		{
			AssertTrue(action.CanBeShownScript(entity), "Fresh tree action should be visible to a real character");
			AssertTrue(!action.CanBeShownScript(null), "Apple action should not show without a character");
		}

		SCR_EntityHelper.DeleteEntityAndChildren(entity);
		SCR_EntityHelper.DeleteEntityAndChildren(tree);
	}
}

#endif
