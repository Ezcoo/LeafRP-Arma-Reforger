// LeafRP virtual fishing v0.1.
// Attach this action to an ActionsManagerComponent on an existing coastal
// scene entity. Set its hold/action duration to 15 seconds in Workbench.
// No new models, equipment, inventory items or shoreline detection required.
//
// Prototype limitation: results and UI are local to the interacting player.
// Do not treat catches as server-validated or persistent economy items.
class LRP_VirtualFishingUserAction : ScriptedUserAction
{
	override bool GetActionNameScript(out string outName)
	{
		outName = "Fish";
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

		// PerformAction is called after the configured hold duration completes.
		// Math.RandomInt's upper bound is exclusive: 0..69 = 70% success.
		int roll = Math.RandomInt(0, 100);
		if (roll < 70)
		{
			Print("[LeafRP] Virtual fishing: caught a fish.", LogLevel.NORMAL);
			SCR_HintManagerComponent.ShowCustomHint("You caught a fish! (Virtual catch)", "LeafRP Fishing", 4.0);
		}
		else
		{
			Print("[LeafRP] Virtual fishing: no bite.", LogLevel.NORMAL);
			SCR_HintManagerComponent.ShowCustomHint("No bite this time. Try again!", "LeafRP Fishing", 4.0);
		}
	}

	override bool HasLocalEffectOnlyScript()
	{
		// This v0.1 prototype grants no real items or server-side rewards.
		return true;
	}
}
