// LeafRP Apple: the native inventory UI keeps using the selected item AFTER
// SCR_CharacterInventoryStorageComponent.UseItem returns. In particular,
// SCR_InventorySlotUI.UseItem accesses item.FindComponent on its next line.
//
// Never delete the Apple synchronously here: that caused a VM NULL exception
// from SCR_InventorySlotQuickSlotUI when the user pressed "Use".
//
// Schedule the deletion after this UI callback returns. All non-Apple items
// continue through the original vanilla behavior unchanged.
class LRP_AppleUseCallback : ScriptedInventoryOperationCallback
{
	SCR_CharacterInventoryStorageComponent m_Storage;
	IEntity m_Item;
	IEntity m_Eater;
	// Keep the effect alive even if deletion destroys its owning component.
	ref LRP_ConsumableEffectEatApple m_Effect;

	override protected void OnComplete()
	{
		if (m_Storage)
			m_Storage.LRP_FinishAppleUse(this, true);
	}

	override protected void OnFailed()
	{
		if (m_Storage)
			m_Storage.LRP_FinishAppleUse(this, false);
	}
}

modded class SCR_CharacterInventoryStorageComponent
{
	// Prevent double consumption if a player presses Use repeatedly before
	// the postponed inventory operation completes.
	// Track operation identity: the item reference may become null on deletion.
	protected ref array<ref LRP_AppleUseCallback> m_aLRPPendingAppleUse = {};

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
		foreach (LRP_AppleUseCallback pending : m_aLRPPendingAppleUse)
		{
			if (pending.m_Item == item)
				return true;
		}

		IEntity eater = GetOwner();
		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(eater.FindComponent(InventoryStorageManagerComponent));

		if (!inventory || !inventory.Contains(item) || !appleEffect.CanApplyEffect(eater, eater))
		{
			Print("[LeafRP] Eat Apple: rejected inventory Use (not owned or eater invalid).", LogLevel.WARNING);
			return false;
		}

		// The original UseItem must return with the item STILL ALIVE, because
		// vanilla inventory UI reads item.FindComponent after our return.
		LRP_AppleUseCallback callback = new LRP_AppleUseCallback();
		callback.m_Storage = this;
		callback.m_Item = item;
		callback.m_Eater = eater;
		callback.m_Effect = appleEffect;
		m_aLRPPendingAppleUse.Insert(callback);
		GetGame().GetCallqueue().CallLater(LRP_CompleteAppleUse, 100, false, callback);
		Print("[LeafRP] Eat Apple: inventory Use queued safely after UI handler.", LogLevel.NORMAL);
		return true;
	}

	protected void LRP_CompleteAppleUse(LRP_AppleUseCallback callback)
	{
		if (m_aLRPPendingAppleUse.Find(callback) == -1)
			return;

		IEntity item = callback.m_Item;
		IEntity eater = callback.m_Eater;
		if (!item || !eater || eater != GetOwner())
		{
			LRP_FinishAppleUse(callback, false);
			return;
		}

		InventoryStorageManagerComponent inventory = InventoryStorageManagerComponent.Cast(eater.FindComponent(InventoryStorageManagerComponent));
		if (!inventory || !inventory.Contains(item))
		{
			Print("[LeafRP] Eat Apple: delayed use aborted because item is no longer in inventory.", LogLevel.WARNING);
			LRP_FinishAppleUse(callback, false);
			return;
		}

		SCR_ConsumableItemComponent consumable = SCR_ConsumableItemComponent.Cast(item.FindComponent(SCR_ConsumableItemComponent));
		LRP_ConsumableEffectEatApple appleEffect;
		if (consumable)
			appleEffect = LRP_ConsumableEffectEatApple.Cast(consumable.GetConsumableEffect());

		if (!appleEffect || !appleEffect.CanApplyEffect(eater, eater))
		{
			LRP_FinishAppleUse(callback, false);
			return;
		}

		if (!inventory.TryDeleteItem(item, callback))
		{
			Print("[LeafRP] Eat Apple: delayed inventory deletion request was rejected.", LogLevel.WARNING);
			LRP_FinishAppleUse(callback, false);
			return;
		}
	}

	// Called by the native operation callback, or by an aborted/rejected request.
	void LRP_FinishAppleUse(LRP_AppleUseCallback callback, bool successful)
	{
		int index = m_aLRPPendingAppleUse.Find(callback);
		if (index == -1)
			return;

		IEntity eater = callback.m_Eater;
		ref LRP_ConsumableEffectEatApple effect = callback.m_Effect;
		m_aLRPPendingAppleUse.Remove(index);
		if (!successful)
		{
			Print("[LeafRP] Eat Apple: inventory consumption did not complete.", LogLevel.WARNING);
			return;
		}

		Print("[LeafRP] Eat Apple: inventory deletion completed.", LogLevel.NORMAL);
		if (eater && eater == GetOwner() && effect)
			effect.LRP_ReportAppleEaten(eater);
	}
}
