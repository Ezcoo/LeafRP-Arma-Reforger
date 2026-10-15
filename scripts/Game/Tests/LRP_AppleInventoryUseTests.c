// Test the actual Inventory -> Use call and wait until Reforger finishes
// removing precisely the selected Apple from the character inventory.
#ifdef ENABLE_DIAG

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 20)]
class LRP_TEST_AppleHarvest_EatUsingInventoryUse : SCR_AutotestCaseBase
{
	protected IEntity m_Soldier;
	protected IEntity m_FirstApple;
	protected IEntity m_SecondApple;
	protected InventoryStorageManagerComponent m_Inventory;
	protected bool m_bUseRequested;

	[TestStep(TestStage.Setup)]
	void Setup_ResetTest()
	{
		m_Soldier = null;
		m_FirstApple = null;
		m_SecondApple = null;
		m_Inventory = null;
		m_bUseRequested = false;
	}

	// A bool Main step is polled every frame until it returns true.
	[TestStep(TestStage.Main)]
	bool Execute_UseInventoryApple()
	{
		if (m_bUseRequested)
		{
			// Do not confuse acceptance with success: verify that the selected
			// Apple has actually gone from the managed inventory.
			if (m_Inventory && m_Inventory.Contains(m_FirstApple))
				return false;

			AssertTrue(m_Inventory != null, "Inventory manager must remain valid");
			if (m_Inventory)
				AssertTrue(m_Inventory.Contains(m_SecondApple), "The other Apple must still be present after eating");
			return true;
		}

		Resource soldierPrefab = Resource.Load("{26A9756790131354}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Rifleman.et");
		Resource applePrefab = Resource.Load("{7D12171C82B54550}Prefabs/Items/LeafRP_Apple.et");
		AssertTrue(soldierPrefab.IsValid() && applePrefab.IsValid(), "Soldier and Apple prefabs must load");
		if (!soldierPrefab.IsValid() || !applePrefab.IsValid())
			return true;

		m_Soldier = GetGame().SpawnEntityPrefab(soldierPrefab, GetGame().GetWorld());
		AssertTrue(m_Soldier != null, "Soldier must spawn");
		if (!m_Soldier)
			return true;

		m_Inventory = InventoryStorageManagerComponent.Cast(m_Soldier.FindComponent(InventoryStorageManagerComponent));
		SCR_InventoryStorageManagerComponent scrInventory = SCR_InventoryStorageManagerComponent.Cast(m_Inventory);
		AssertTrue(scrInventory != null, "Soldier must have a scripted inventory manager");
		if (!scrInventory)
			return true;

		SCR_CharacterInventoryStorageComponent storage = scrInventory.GetCharacterStorage();
		AssertTrue(storage != null, "Soldier must have native character inventory storage");
		if (!storage)
			return true;

		ResourceName appleName = "{7D12171C82B54550}Prefabs/Items/LeafRP_Apple.et";
		AssertTrue(m_Inventory.TrySpawnPrefabToStorage(appleName), "First Apple must enter inventory");
		AssertTrue(m_Inventory.TrySpawnPrefabToStorage(appleName), "Second Apple must enter inventory");

		array<IEntity> items = {};
		m_Inventory.GetItems(items);
		foreach (IEntity found : items)
		{
			if (!found)
				continue;

			SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(found.FindComponent(SCR_ConsumableItemComponent));
			if (!consumable || !LRP_ConsumableEffectEatApple.Cast(consumable.GetConsumableEffect()))
				continue;

			if (!m_FirstApple)
				m_FirstApple = found;
			else if (!m_SecondApple)
				m_SecondApple = found;
		}

		AssertTrue(m_FirstApple != null && m_SecondApple != null, "Two distinct Apples must be in the inventory");
		if (!m_FirstApple || !m_SecondApple)
			return true;

		AssertTrue(m_Inventory.Contains(m_FirstApple), "First Apple must be in inventory");
		AssertTrue(m_Inventory.Contains(m_SecondApple), "Second Apple must be in inventory");

		// Exactly the method and context used by Reforger's Inventory -> Use.
		bool accepted = storage.UseItem(m_FirstApple, ESlotFunction.TYPE_GENERIC, SCR_EUseContext.FROM_INVENTORY);
		AssertTrue(accepted, "Inventory Use must accept consuming an Apple");
		if (!accepted)
			return true;

		m_bUseRequested = true;
		return false;
	}

	[TestStep(TestStage.TearDown)]
	void TearDown_DeleteCharacter()
	{
		if (m_Soldier)
			SCR_EntityHelper.DeleteEntityAndChildren(m_Soldier);
	}
}

#endif
