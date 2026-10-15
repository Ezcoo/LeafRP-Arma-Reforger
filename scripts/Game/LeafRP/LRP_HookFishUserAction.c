// Second instant interaction for the local virtual fishing minigame.
class LRP_HookFishUserAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Hook Fish!";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		return ChimeraCharacter.Cast(user) != null && LRP_VirtualFishingUserAction.LRP_HasBite();
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
		LRP_VirtualFishingUserAction.LRP_HookFish();
	}

	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
}
