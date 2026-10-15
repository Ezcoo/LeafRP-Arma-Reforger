// LeafRP apple picking. Attach to an ActionsManagerComponent on a placeable
// apple-tree prefab (details in docs/APPLE-PICKING-PROTOTYPE.md).
//
// Phase 2: finite per-tree harvest and optional regrowth. Virtual apples only:
// no physical inventory, persistence or multiplayer authority yet.
class LRP_PickAppleUserAction : ScriptedUserAction
{
	[Attribute("5", UIWidgets.EditBox, "Number of apples available per harvest")]
	protected int m_iStartingApples;

	[Attribute("300", UIWidgets.EditBox, "Seconds before an empty tree refills; 0 disables regrowth")]
	protected int m_iRegrowSeconds;

	protected ref LRP_AppleHarvestStock m_LRPStock;
	protected bool m_bLRPLoggedEmptyTree;

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
		return player && player.GetControlledEntity() == user;
	}

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!CanBePerformedScript(pUserEntity))
			return;

		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!player || !m_LRPStock.TryPick(System.GetUnixTime()))
			return;

		player.LRP_AddPickedApple();
		Print(string.Format("[LeafRP] Apple picked. Tree remaining=%1, player total=%2.",
			m_LRPStock.GetRemaining(), player.LRP_GetPickedAppleCount()), LogLevel.NORMAL);
	}

	override bool HasLocalEffectOnlyScript()
	{
		// A real multiplayer economy will require server-authoritative stock.
		return true;
	}
}

// Local, per-player tally (resets on session restart; not part of inventory).
modded class SCR_PlayerController
{
	protected int m_iLRPPickedApples;

	int LRP_GetPickedAppleCount()
	{
		return m_iLRPPickedApples;
	}

	void LRP_AddPickedApple()
	{
		m_iLRPPickedApples++;
		Print(string.Format("[LeafRP] Player now holds %1 virtual apples.", m_iLRPPickedApples), LogLevel.NORMAL);

		if (SCR_HintManagerComponent.GetInstance())
		{
			SCR_HintManagerComponent.ShowCustomHint(
				string.Format("Picked an apple! Collected: %1", m_iLRPPickedApples),
				"LeafRP",
				3.0
			);
		}
	}
}
