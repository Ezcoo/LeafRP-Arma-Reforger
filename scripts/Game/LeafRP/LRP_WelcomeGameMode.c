// LeafRP first gameplay feature: a lightweight welcome hint after local player spawn.
// This uses a modded base game mode, so no custom prefab is required.
//
// The custom hint API is intended for prototyping; use a configured/localized
// SCR_HintUIInfo in the production UI.
modded class SCR_BaseGameMode
{
	override void OnPlayerSpawned(int playerId, IEntity controlledEntity)
	{
		super.OnPlayerSpawned(playerId, controlledEntity);

		if (!controlledEntity)
			return;

		// UI is local-only: never show the message for other players or on
		// a dedicated server where there is no local player.
		if (playerId != SCR_PlayerController.GetLocalPlayerId())
			return;

		Print("[LeafRP] Local player spawned; scheduling welcome hint.", LogLevel.NORMAL);
		GetGame().GetCallqueue().CallLater(LRP_ShowWelcomeHint, 1500, false);
	}

	protected void LRP_ShowWelcomeHint()
	{
		if (!SCR_HintManagerComponent.GetInstance())
		{
			Print("[LeafRP] Hint manager is not available.", LogLevel.WARNING);
			return;
		}

		SCR_HintManagerComponent.ShowCustomHint(
			"Welcome to LeafRP! Your journey begins here.",
			"LeafRP",
			8.0
		);
		Print("[LeafRP] Welcome hint requested.", LogLevel.NORMAL);
	}
}
