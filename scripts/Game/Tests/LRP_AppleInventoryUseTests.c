// Test the actual Inventory -> Use call and wait until Reforger finishes
// removing precisely the selected Apple from the character inventory.
// Also reproduce the UI's use-after-return access that previously crashed.
#ifdef ENABLE_DIAG

// Test access stays out of the production API. Inject callback outcomes without
// relying on an intermittent native deletion failure.
modded class LRP_AppleUseCallback
{
	void LRP_TEST_Fail()
	{
		OnFailed();
	}

	void LRP_TEST_Complete()
	{
		OnComplete();
	}
}

modded class SCR_CharacterInventoryStorageComponent
{
	LRP_AppleUseCallback LRP_TEST_GetPendingAppleUse(IEntity item)
	{
		foreach (LRP_AppleUseCallback pending : m_aLRPPendingAppleUse)
		{
			if (pending.m_Item == item)
				return pending;
		}
		return null;
	}
}

[BaseContainerProps()]
modded class LRP_ConsumableEffectEatApple
{
	static int s_iLRPTestConsumptionReports;

	override void LRP_ReportAppleEaten(IEntity eater)
	{
		s_iLRPTestConsumptionReports++;
		super.LRP_ReportAppleEaten(eater);
	}
}

[Test(suite: LRP_TEST_AppleHarvestSuite, timeoutS: 20)]
class LRP_TEST_AppleHarvest_EatUsingInventoryUse : SCR_AutotestCaseBase
{
	protected IEntity m_Soldier;
	protected IEntity m_FirstApple;
	protected IEntity m_SecondApple;
	protected InventoryStorageManagerComponent m_Inventory;
	protected SCR_CharacterInventoryStorageComponent m_Storage;
	protected ref LRP_AppleUseCallback m_UseCallback;
	protected int m_iInitialReports;
	protected bool m_bUseRequested;

	[TestStep(TestStage.Setup)]
	void Setup_ResetTest()
	{
		m_Soldier = null;
		m_FirstApple = null;
		m_SecondApple = null;
		m_Inventory = null;
		m_Storage = null;
		m_UseCallback = null;
		m_iInitialReports = LRP_ConsumableEffectEatApple.s_iLRPTestConsumptionReports;
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
			if (m_Inventory && m_FirstApple && m_Inventory.Contains(m_FirstApple))
			{
				AssertTrue(m_Storage.LRP_TEST_GetPendingAppleUse(m_FirstApple) == m_UseCallback,
					"Pending guard must survive until native deletion completes");
				AssertTrue(m_Storage.UseItem(m_FirstApple, ESlotFunction.TYPE_GENERIC, SCR_EUseContext.FROM_INVENTORY),
					"Repeated Use while deletion is pending must be handled");
				return false;
			}

			// Removal can precede callback delivery; wait for both observations.
			if (m_Storage.LRP_TEST_GetPendingAppleUse(m_FirstApple))
				return false;

			AssertTrue(m_Inventory != null, "Inventory manager must remain valid");
			if (m_Inventory)
				AssertTrue(m_Inventory.Contains(m_SecondApple), "The other Apple must still be present after eating");
			AssertTrue(LRP_ConsumableEffectEatApple.s_iLRPTestConsumptionReports == m_iInitialReports + 1,
				"Confirmed deletion must report consumption exactly once");
			m_UseCallback.LRP_TEST_Complete();
			AssertTrue(LRP_ConsumableEffectEatApple.s_iLRPTestConsumptionReports == m_iInitialReports + 1,
				"Duplicate completion must not report consumption again");

			// Deliver a failure before dispatch. The queued request must then be
			// ignored, the Apple preserved, and a subsequent Use allowed.
			AssertTrue(m_Storage.UseItem(m_SecondApple, ESlotFunction.TYPE_GENERIC, SCR_EUseContext.FROM_INVENTORY),
				"Second Apple Use must be accepted");
			ref LRP_AppleUseCallback failed = m_Storage.LRP_TEST_GetPendingAppleUse(m_SecondApple);
			AssertTrue(failed != null, "Second Apple must have a pending operation");
			if (!failed)
				return true;
			failed.LRP_TEST_Fail();
			failed.LRP_TEST_Complete();
			AssertTrue(m_Storage.LRP_TEST_GetPendingAppleUse(m_SecondApple) == null,
				"Failure must release the pending guard");
			AssertTrue(m_Inventory.Contains(m_SecondApple), "Failed consumption must preserve the Apple");
			AssertTrue(LRP_ConsumableEffectEatApple.s_iLRPTestConsumptionReports == m_iInitialReports + 1,
				"Failure and stale completion must not report consumption");
			AssertTrue(m_Storage.UseItem(m_SecondApple, ESlotFunction.TYPE_GENERIC, SCR_EUseContext.FROM_INVENTORY),
				"Use must be retryable after failure");
			ref LRP_AppleUseCallback retry = m_Storage.LRP_TEST_GetPendingAppleUse(m_SecondApple);
			AssertTrue(retry != null && retry != failed, "Retry must create a new operation");
			if (retry)
				retry.LRP_TEST_Fail();
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
		m_Storage = storage;
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
		m_UseCallback = storage.LRP_TEST_GetPendingAppleUse(m_FirstApple);
		AssertTrue(m_UseCallback != null, "Accepted Use must retain its operation callback");
		if (!m_UseCallback)
			return true;
		AssertTrue(LRP_ConsumableEffectEatApple.s_iLRPTestConsumptionReports == m_iInitialReports,
			"Request acceptance must not report consumption");

		// CRASH REGRESSION: vanilla SCR_InventorySlotUI.UseItem accesses
		// item.FindComponent AFTER calling storage.UseItem. It must still
		// be alive when the function returns, before deferred deletion.
		AssertTrue(m_Inventory.Contains(m_FirstApple), "Apple must remain alive until inventory UI returns");
		AssertTrue(m_FirstApple.FindComponent(SCR_ConsumableItemComponent) != null,
			"Inventory UI must still be able to access item components after UseItem returns");

		// Repeated use must not queue another deletion of the same Apple.
		AssertTrue(storage.UseItem(m_FirstApple, ESlotFunction.TYPE_GENERIC, SCR_EUseContext.FROM_INVENTORY),
			"A duplicate Use should be safely ignored while Apple is pending");

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
