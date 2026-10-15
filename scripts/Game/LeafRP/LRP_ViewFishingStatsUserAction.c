// LeafRP v0.3: a second action on the same fishing spot.
// Set this action's duration to 0 in Workbench so it displays instantly.
class LRP_ViewFishingStatsUserAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "View Fishing Stats";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
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

		SCR_HintManagerComponent.ShowCustomHint(
			LRP_VirtualFishingUserAction.LRP_GetStatsText(),
			"LeafRP Fishing Records", 8.0);
		Print("[LeafRP] Viewed local fishing statistics.", LogLevel.NORMAL);
	}

	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
}
