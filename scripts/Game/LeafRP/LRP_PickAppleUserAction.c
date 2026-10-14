// LeafRP apple-picking prototype. Requires no addon besides vanilla Reforger.
//
// Attach LRP_PickAppleUserAction to the Additional Actions of an
// ActionsManagerComponent on a *placeable apple tree* prefab.
// In this first iteration, harvested apples are a local session counter,
// NOT physical inventory items. Multiplayer synchronization comes later.
class LRP_PickAppleUserAction : ScriptedUserAction
{
	[Attribute("5", UIWidgets.EditBox, "Number of apples available on this tree")]
	protected int m_iStartingApples;

	protected int m_iRemainingApples;

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);
		m_iRemainingApples = Math.Max(m_iStartingApples, 0);
		Print(string.Format("[LeafRP] Apple tree initialized with %1 apples.", m_iRemainingApples), LogLevel.NORMAL);
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = string.Format("Pick apple (%1 left)", m_iRemainingApples);
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (m_iRemainingApples <= 0)
			return false;

		return ChimeraCharacter.Cast(user) != null;
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
		if (!player)
			return;

		m_iRemainingApples--;
		player.LRP_AddPickedApple();

		Print(string.Format("[LeafRP] Apple picked. Tree remaining=%1, player total=%2.",
			m_iRemainingApples, player.LRP_GetPickedAppleCount()), LogLevel.NORMAL);
	}

	override bool HasLocalEffectOnlyScript()
	{
		// Phase 1 is a local gameplay smoke test, NOT an MP economy system.
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
