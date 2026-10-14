// LeafRP gameplay smoke test: display a local welcome hint when taking control
// of a character (covers normal player spawning and Game Master possession).
modded class SCR_PlayerController
{
	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		Print(string.Format("[LeafRP] Controlled entity changed: player=%1, local=%2", GetPlayerId(), localPlayerId), LogLevel.NORMAL);

		// The client that owns the character must request the local UI hint.
		if (!to || !ChimeraCharacter.Cast(to))
			return;

		if (localPlayerId <= 0 || GetPlayerId() != localPlayerId)
			return;

		Print("[LeafRP] Local character controlled. Scheduling welcome hint.", LogLevel.NORMAL);
		GetGame().GetCallqueue().CallLater(LRP_ShowWelcomeHint, 1500, false);
	}

	protected void LRP_ShowWelcomeHint()
	{
		if (GetPlayerId() != SCR_PlayerController.GetLocalPlayerId())
			return;

		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
		{
			Print("[LeafRP] Welcome hint unavailable: no SCR_HintManagerComponent on this game mode.", LogLevel.WARNING);
			return;
		}

		bool shown = SCR_HintManagerComponent.ShowCustomHint(
			"Welcome to LeafRP! Your journey begins here.",
			"LeafRP",
			8.0
		);

		if (shown)
			Print("[LeafRP] Welcome hint displayed successfully.", LogLevel.NORMAL);
		else
			Print("[LeafRP] Hint manager refused the welcome hint.", LogLevel.WARNING);
	}
}
