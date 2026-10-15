// LeafRP apple picking. Attach to an ActionsManagerComponent on a placeable
// apple-tree prefab (details in docs/APPLE-PICKING-PROTOTYPE.md).
//
// Phase 3: finite per-tree harvest, regrowth and physical inventory apples.
// This prototype still uses local action state; multiplayer authority is future work.
class LRP_PickAppleUserAction : ScriptedUserAction
{
	[Attribute("5", UIWidgets.EditBox, "Number of apples available per harvest")]
	protected int m_iStartingApples;

	[Attribute("300", UIWidgets.EditBox, "Seconds before an empty tree refills; 0 disables regrowth")]
	protected int m_iRegrowSeconds;

	protected ref LRP_AppleHarvestStock m_LRPStock;
	protected bool m_bLRPLoggedEmptyTree;

	// Dedicated, vanilla-based LeafRP item prefab; NOT an external addon.
	ResourceName LRP_GetAppleItemPrefab()
	{
		return "{7D12171C82B54550}Prefabs/Items/LeafRP_Apple.et";
	}

	// Actions may be queried for visibility before their Init callback has
	// initialized our runtime-only stock object. Avoid silently hiding them.
	protected void LRP_EnsureStock()
	{
		if (m_LRPStock)
			return;

		m_LRPStock = new LRP_AppleHarvestStock();
		m_LRPStock.Reset(m_iStartingApples, m_iRegrowSeconds);
		Print(string.Format("[LeafRP] Apple action stock initialized: starting=%1, remaining=%2, regrow=%3s.",
			m_iStartingApples, m_LRPStock.GetRemaining(), m_iRegrowSeconds), LogLevel.NORMAL);
	}

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		LRP_EnsureStock();
	}

	override bool GetActionNameScript(out string outName)
	{
		LRP_EnsureStock();
		m_LRPStock.Update(System.GetUnixTime());
		outName = string.Format("Pick apple (%1 left)", m_LRPStock.GetRemaining());
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (!ChimeraCharacter.Cast(user))
			return false;

		LRP_EnsureStock();
		m_LRPStock.Update(System.GetUnixTime());

		if (m_LRPStock.GetRemaining() <= 0)
		{
			if (!m_bLRPLoggedEmptyTree)
			{
				Print(string.Format("[LeafRP] Apple action hidden: remaining=%1, starting=%2, regrowAt=%3.",
					m_LRPStock.GetRemaining(), m_iStartingApples, m_LRPStock.GetRegrowAt()), LogLevel.NORMAL);
				m_bLRPLoggedEmptyTree = true;
			}
			return false;
		}

		m_bLRPLoggedEmptyTree = false;
		return true;
	}

	override bool CanBePerformedScript(IEntity user)
	{
		if (!CanBeShownScript(user))
			return false;

		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!player || player.GetControlledEntity() != user)
			return false;

		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(user.FindComponent(InventoryStorageManagerComponent));
		if (!inventory)
			return false;

		// Do not permit picking when there is no usable storage space.
		if (!inventory.CanInsertResource(LRP_GetAppleItemPrefab()))
		{
			SetCannotPerformReason("Inventory full");
			return false;
		}

		return true;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!CanBePerformedScript(pUserEntity))
			return;

		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(pUserEntity.FindComponent(InventoryStorageManagerComponent));
		if (!inventory)
			return;

		// In the current local-player prototype, stock does not change between
		// the availability check and this synchronous inventory insertion.
		// Never consume an apple unless the inventory operation is accepted.
		if (!inventory.TrySpawnPrefabToStorage(LRP_GetAppleItemPrefab()))
		{
			Print("[LeafRP] Apple picking aborted: inventory insertion failed.", LogLevel.WARNING);
			SCR_HintManagerComponent.ShowCustomHint("Could not put apple in inventory", "LeafRP", 3.0);
			return;
		}

		if (!m_LRPStock.TryPick(System.GetUnixTime()))
		{
			Print("[LeafRP] Unexpected harvest state after successful item insertion.", LogLevel.ERROR);
			return;
		}

		Print(string.Format("[LeafRP] Real apple item added to inventory. Tree remaining=%1.",
			m_LRPStock.GetRemaining()), LogLevel.NORMAL);
		SCR_HintManagerComponent.ShowCustomHint("Picked an apple! Check your inventory", "LeafRP", 3.0);
	}

	override bool HasLocalEffectOnlyScript()
	{
		// A real multiplayer economy will require server-authoritative stock.
		return true;
	}
}

