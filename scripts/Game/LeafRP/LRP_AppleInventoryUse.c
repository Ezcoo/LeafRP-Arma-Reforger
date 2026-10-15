// LeafRP Apple: the native inventory UI keeps using the selected item AFTER
// SCR_CharacterInventoryStorageComponent.UseItem returns. In particular,
// SCR_InventorySlotUI.UseItem accesses item.FindComponent on its next line.
//
// Never delete the Apple synchronously here: that caused a VM NULL exception
// from SCR_InventorySlotQuickSlotUI when the user pressed "Use".
//
// Schedule the deletion after this UI callback returns. All non-Apple items
// continue through the original vanilla behavior unchanged.
modded class SCR_CharacterInventoryStorageComponent
{
	// Prevent double consumption if a player presses Use repeatedly before
	// the postponed inventory operation completes.
	protected ref set<IEntity> m_sLRPPendingAppleUse = new set<IEntity>();

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

		// An already scheduled Apple use is handled; do not delete it twice.
		if (m_sLRPPendingAppleUse.Contains(item))
			return true;

		IEntity eater = GetOwner();
		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(eater.FindComponent(InventoryStorageManagerComponent));

		if (!inventory || !inventory.Contains(item) || !appleEffect.CanApplyEffect(eater, eater))
		{
			Print("[LeafRP] Eat Apple: rejected inventory Use (not owned or eater invalid).", LogLevel.WARNING);
			return false;
		}

		// The original UseItem must return with the item STILL ALIVE, because
		// vanilla inventory UI reads item.FindComponent after our return.
		m_sLRPPendingAppleUse.Insert(item);
		GetGame().GetCallqueue().CallLater(LRP_CompleteAppleUse, 100, false, item, eater);
		Print("[LeafRP] Eat Apple: inventory Use queued safely after UI handler.", LogLevel.NORMAL);
		return true;
	}

	protected void LRP_CompleteAppleUse(IEntity item, IEntity eater)
	{
		m_sLRPPendingAppleUse.RemoveItem(item);

		if (!item || !eater || eater != GetOwner())
			return;

		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(eater.FindComponent(InventoryStorageManagerComponent));
		if (!inventory || !inventory.Contains(item))
		{
			Print("[LeafRP] Eat Apple: delayed use aborted because item is no longer in inventory.", LogLevel.WARNING);
			return;
		}

		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		LRP_ConsumableEffectEatApple appleEffect;
		if (consumable)
			appleEffect = LRP_ConsumableEffectEatApple.Cast(consumable.GetConsumableEffect());

		if (!appleEffect || !appleEffect.CanApplyEffect(eater, eater))
			return;

		if (!inventory.TryDeleteItem(item))
		{
			Print("[LeafRP] Eat Apple: delayed inventory deletion request was rejected.", LogLevel.WARNING);
			return;
		}

		Print("[LeafRP] Eat Apple: deferred deletion accepted.", LogLevel.NORMAL);
		appleEffect.LRP_ReportAppleEaten(eater);
	}
}
