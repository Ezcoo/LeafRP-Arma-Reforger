// LeafRP's first food effect. Uses only vanilla Reforger's consumable
// inventory lifecycle: SCR_ConsumableItemComponent runs this after a
// successful item-use animation and deletes the consumed item via its
// built-in m_bDeleteOnUse behavior.
//
// Hunger/health restoration is deliberately not implemented yet.
[BaseContainerProps()]
class LRP_ConsumableEffectEatApple : SCR_ConsumableEffectBase
{
	// An apple can be eaten by a living character, with no health requirement.
	// It should still work at full health.
	override bool CanApplyEffect(notnull IEntity target, notnull IEntity user, out SCR_EConsumableFailReason failReason = SCR_EConsumableFailReason.NONE)
	{
		failReason = SCR_EConsumableFailReason.NONE;

		// The native inventory UI calls CanApplyEffect(character, character).
		// However SCR_ConsumableItemComponent.ActivateAction() calls
		// CanApplyEffect(character, GetOwner()), where GetOwner() is the ITEM!
		// We must accept both call conventions or the actual gadget action
		// silently returns before triggering our animation/effect.
		ChimeraCharacter eater = ChimeraCharacter.Cast(target);
		if (!eater)
			return false;

		if (user != target)
		{
			SCR_ConsumableItemComponent apple = SCR_ConsumableItemComponent.Cast(user.FindComponent(SCR_ConsumableItemComponent));
			if (!apple || apple.GetConsumableEffect() != this)
				return false;

			Print("[LeafRP] Eat Apple activation gate reached by native gadget.", LogLevel.NORMAL);
		}

		CharacterControllerComponent controller = eater.GetCharacterController();
		return controller != null && controller.GetLifeState() == ECharacterLifeState.ALIVE;
	}

	// Reuse the vanilla self-use command until LeafRP has its own eating pose.
	override bool UpdateAnimationCommands(IEntity user)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CharacterAnimationComponent animation = character.GetAnimationComponent();
		if (!animation)
			return false;

		m_iPlayerApplyToSelfCmdId = animation.BindCommand("CMD_HealSelf");
		return m_iPlayerApplyToSelfCmdId >= 0;
	}

	// Native consumables must send the animation command to the controller.
	// The inherited default uses TryUseItem(item) without the self-use
	// override command, which leaves this custom food item inert in practice.
	override bool ActivateEffect(IEntity target, IEntity user, IEntity item, ItemUseParameters animParams = null)
	{
		if (!target || !user || !item || !CanApplyEffect(target, user))
			return false;

		ItemUseParameters parameters = animParams;
		if (!parameters)
			parameters = GetAnimationParameters(item, target);

		if (!parameters || parameters.GetCommandID() < 0)
		{
			Print("[LeafRP] Eat Apple cannot start: no valid use animation command.", LogLevel.WARNING);
			return false;
		}

		bool started = super.ActivateEffect(target, user, item, parameters);
		Print(string.Format("[LeafRP] Eat Apple animation request: started=%1, command=%2.",
			started, parameters.GetCommandID()), LogLevel.NORMAL);
		return started;
	}

	// Called only after the inventory use completes successfully.
	// The owning SCR_ConsumableItemComponent handles deleting this one apple.
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		LRP_ReportAppleEaten(user);
	}

	// Called after a native animation completes OR after Inventory -> Use
	// successfully requests deletion of one inventory Apple.
	void LRP_ReportAppleEaten(IEntity eater)
	{
		Print("[LeafRP] Player ate one Apple.", LogLevel.NORMAL);

		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (player && player.GetControlledEntity() == eater && SCR_HintManagerComponent.GetInstance())
			SCR_HintManagerComponent.ShowCustomHint("You ate an apple.", "LeafRP", 3.0);
	}
}
