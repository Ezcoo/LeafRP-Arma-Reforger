// Real end-to-end call-site test: Inventory UI sends UseItem(... FROM_INVENTORY).
// This test reproduces the actual dispatch path instead of inspecting metadata.
#ifdef ENABLE_DIAG

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 30)]
class LRP_TEST_AppleHarvest_EatUsingInventoryUse : SCR_AutotestCaseBase
{
	[TestStep(TestStage.Main)]
	void Execute_UseInventoryApple()
	{
		Resource soldierPrefab = Resource.Load("{26A9756790131354}Prefabs/Characters/Factions/BLUFOR/US_Army/Character_US_Rifleman.et");
		Resource applePrefab = Resource.Load("{7D12171C82B54550}Prefabs/Items/LeafRP_Apple.et");
		AssertTrue(soldierPrefab.IsValid() && applePrefab.IsValid(), "Soldier and Apple prefabs must load");
		if (!soldierPrefab.IsValid() || !applePrefab.IsValid())
			return;

		IEntity soldier = GetGame().SpawnEntityPrefab(soldierPrefab, GetGame().GetWorld());
		AssertTrue(soldier != null, "Soldier must spawn");
		if (!soldier)
			return;

		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(soldier.FindComponent(InventoryStorageManagerComponent));
		SCR_InventoryStorageManagerComponent scrInventory = SCR_InventoryStorageManagerComponent.Cast(inventory);
		AssertTrue(scrInventory != null, "Soldier must have a scripted inventory manager");
		if (!scrInventory)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(soldier);
			return;
		}

		SCR_CharacterInventoryStorageComponent storage = scrInventory.GetCharacterStorage();
		AssertTrue(storage != null, "Soldier must have native character inventory storage");
		if (!storage)
		{
			SCR_EntityHelper.DeleteEntityAndChildren(soldier);
			return;
		}

		// Spawn two physical apples directly into the native inventory.
		ResourceName appleName = "{7D12171C82B54550}Prefabs/Items/LeafRP_Apple.et";
		AssertTrue(inventory.TrySpawnPrefabToStorage(appleName), "First Apple must enter inventory");
		AssertTrue(inventory.TrySpawnPrefabToStorage(appleName), "Second Apple must enter inventory");

		array<IEntity> items = {};
		inventory.GetItems(items);
		IEntity firstApple;
		IEntity secondApple;
		foreach (IEntity found : items)
		{
			if (!found)
				continue;

			SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(found.FindComponent(SCR_ConsumableItemComponent));
			if (!consumable || !LRP_ConsumableEffectEatApple.Cast(consumable.GetConsumableEffect()))
				continue;

			if (!firstApple)
				firstApple = found;
			else if (!secondApple)
				secondApple = found;
		}

		AssertTrue(firstApple != null && secondApple != null, "Two distinct Apples must exist in inventory");
		if (firstApple && secondApple)
		{
			AssertTrue(inventory.Contains(firstApple), "First Apple must be in soldier inventory");
			AssertTrue(inventory.Contains(secondApple), "Second Apple must be in soldier inventory");

			// The exact method and context used by the inventory Use button.
			bool accepted = storage.UseItem(firstApple, ESlotFunction.TYPE_GENERIC, SCR_EUseContext.FROM_INVENTORY);
			AssertTrue(accepted, "Inventory Use must accept and consume one Apple");
			AssertTrue(inventory.Contains(secondApple), "Eating one Apple must not remove another Apple");
		}

		SCR_EntityHelper.DeleteEntityAndChildren(soldier);
	}
}

#endif
