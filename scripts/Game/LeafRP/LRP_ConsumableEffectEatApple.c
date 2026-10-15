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

		ChimeraCharacter eater = ChimeraCharacter.Cast(user);
		if (!eater || target != user)
			return false;

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

	// Called only after the inventory use completes successfully.
	// The owning SCR_ConsumableItemComponent handles deleting this one apple.
	override void ApplyEffect(notnull IEntity target, notnull IEntity user, IEntity item, ItemUseParameters animParams)
	{
		Print("[LeafRP] Player ate one Apple.", LogLevel.NORMAL);

		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (player && player.GetControlledEntity() == user && SCR_HintManagerComponent.GetInstance())
			SCR_HintManagerComponent.ShowCustomHint("You ate an apple.", "LeafRP", 3.0);
	}
}
