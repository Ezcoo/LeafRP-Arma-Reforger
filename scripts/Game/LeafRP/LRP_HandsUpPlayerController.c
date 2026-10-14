// LeafRP hands-up system, phase 1: input/state smoke test.
//
// NOTE: The base game does not expose a stock two-handed surrender gesture.
// COMMAND_STOP is a one-handed visual placeholder until a real animation is added.
// We temporarily reuse "TacticalPingHold" so there is no new input config asset.
// This overrides the vanilla hold-to-point gesture; do not ship this binding.

modded class SCR_PlayerController
{
	protected bool m_bLRPHandsRaised;
	protected IEntity m_LRPHandsRaisedEntity;

	// Existing gameplay input action: TacticalPingHold.
	override void ActionGesturePingHold(float value = 0.0, EActionTrigger reason = 0)
	{
		// Vanilla registers this callback for both DOWN and UP.
		// Toggle only on DOWN; ignore UP to keep the pose persistent.
		if (reason != EActionTrigger.DOWN)
			return;

		if (GetPlayerId() != SCR_PlayerController.GetLocalPlayerId())
			return;

		IEntity character = GetControlledEntity();
		if (!character || !ChimeraCharacter.Cast(character))
			return;

		CharacterControllerComponent controller = CharacterControllerComponent.Cast(character.FindComponent(CharacterControllerComponent));
		if (!controller)
		{
			Print("[LeafRP] Hands-up: no CharacterControllerComponent.", LogLevel.WARNING);
			return;
		}

		// Never carry the pose state across Game Master possession or respawning.
		if (m_LRPHandsRaisedEntity != character)
		{
			m_LRPHandsRaisedEntity = character;
			m_bLRPHandsRaised = false;
		}

		if (m_bLRPHandsRaised)
		{
			controller.StopCharacterGesture();
			m_bLRPHandsRaised = false;
			Print("[LeafRP] Hands-up prototype: gesture stopped.", LogLevel.NORMAL);
			SCR_HintManagerComponent.ShowCustomHint("Hands lowered", "LeafRP", 2.0);
			return;
		}

		// Reforger's COMMAND_STOP gesture is a one-handed stand-in.
		// A true hands-up system will require a suitable animation/graph.
		if (!controller.TryStartCharacterGesture(ECharacterGestures.COMMAND_STOP))
		{
			Print("[LeafRP] Hands-up prototype: gesture could not start (try unarmed and standing).", LogLevel.WARNING);
			SCR_HintManagerComponent.ShowCustomHint("Can't raise hands right now (try unarmed)", "LeafRP", 3.0);
			return;
		}

		m_bLRPHandsRaised = true;
		Print("[LeafRP] Hands-up prototype: gesture started.", LogLevel.NORMAL);
		SCR_HintManagerComponent.ShowCustomHint("Hands-up prototype: one-hand STOP gesture", "LeafRP", 3.0);
	}
}
