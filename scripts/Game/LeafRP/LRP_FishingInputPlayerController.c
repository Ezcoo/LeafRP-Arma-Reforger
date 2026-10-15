// Local player input listener for the fishing minigame.
// Requires a registered InputManager action named LRP_HookFish, bound in Workbench.
modded class SCR_PlayerController
{
	protected bool m_bLRPFishingHookListenerRegistered;

	override void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		super.OnControlledEntityChanged(from, to);
		if (GetPlayerId() != SCR_PlayerController.GetLocalPlayerId())
			return;
		if (m_bLRPFishingHookListenerRegistered)
			return;

		InputManager inputs = GetGame().GetInputManager();
		if (!inputs)
			return;

		inputs.AddActionListener("LRP_HookFish", EActionTrigger.DOWN, LRP_OnHookFishPressed);
		m_bLRPFishingHookListenerRegistered = true;
		Print("[LeafRP] Registered fishing hook input listener (LRP_HookFish).", LogLevel.NORMAL);
	}

	protected void LRP_OnHookFishPressed()
	{
		if (GetPlayerId() != SCR_PlayerController.GetLocalPlayerId())
			return;
		Print("[LeafRP] LRP_HookFish key action received.", LogLevel.NORMAL);
		if (!LRP_VirtualFishingUserAction.LRP_HasBite())
		{
			Print("[LeafRP] Hook key ignored: no active bite window.", LogLevel.NORMAL);
			return;
		}
		LRP_VirtualFishingUserAction.LRP_HookFish();
	}
}
