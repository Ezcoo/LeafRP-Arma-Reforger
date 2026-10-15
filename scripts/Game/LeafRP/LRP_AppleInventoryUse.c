// The vanilla Inventory -> Use UI calls SCR_CharacterInventoryStorageComponent.UseItem.
// For gadgets that normally means equip/toggle, not "consume" on the first press.
//
// Make Apple's Inventory -> Use action consume one actual stored Apple directly.
// Other inventory items keep their original vanilla behavior.
// Held-gadget activation can still use LRP_ConsumableEffectEatApple's native
// animation path; this direct inventory shortcut is deliberately instant.
modded class SCR_CharacterInventoryStorageComponent
{
	override bool UseItem(notnull IEntity item, ESlotFunction slotFunction = ESlotFunction.TYPE_GENERIC, SCR_EUseContext context = SCR_EUseContext.FROM_QUICKSLOT)
	{
		if (context != SCR_EUseContext.FROM_INVENTORY)
			return super.UseItem(item, slotFunction, context);

		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		LRP_ConsumableEffectEatApple appleEffect;
		if (consumable)
			appleEffect = LRP_ConsumableEffectEatApple.Cast(consumable.GetConsumableEffect());

		if (!appleEffect)
			return super.UseItem(item, slotFunction, context);

		IEntity eater = GetOwner();
		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(eater.FindComponent(InventoryStorageManagerComponent));

		// A selected item must belong to THIS character's managed inventory.
		if (!inventory || !inventory.Contains(item) || !appleEffect.CanApplyEffect(eater, eater))
		{
			Print("[LeafRP] Eat Apple: rejected inventory Use (not owned or eater invalid).", LogLevel.WARNING);
			return false;
		}

		// Request deletion of this single inventory entity only.
		if (!inventory.TryDeleteItem(item))
		{
			Print("[LeafRP] Eat Apple: inventory deletion request was rejected.", LogLevel.WARNING);
			return false;
		}

		Print("[LeafRP] Eat Apple: inventory Use accepted; deleting one Apple.", LogLevel.NORMAL);
		appleEffect.LRP_ReportAppleEaten(eater);
		return true;
	}
}
