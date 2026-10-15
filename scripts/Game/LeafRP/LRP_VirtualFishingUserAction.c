// LeafRP virtual fishing v0.3.
// Existing 15-second action duration remains configured on the fishing
// spot's ActionsManagerComponent in Workbench. No physical fish items.
//
// Local prototype only: these static statistics are shared by fishing spots
// on this client, and last only as long as this script runtime is loaded.
// Do not treat them as replicated, per-player server data or persistence.
class LRP_VirtualFishingUserAction : ScriptedUserAction
{
	protected static int s_iCatchCount;
	protected static int s_iTotalWeightDecigrams;
	protected static int s_iBiggestWeightDecigrams;
	protected static int s_iBootCount;
	protected static int s_iGoldenPerchCount;

	// Each unit here is 0.1 kg, despite the legacy 'Decigrams' field name.
	protected static string LRP_FormatWeight(int tenthsKg)
	{
		return string.Format("%1.%2 kg", tenthsKg / 10, tenthsKg % 10);
	}

	// Shared by the Fish and View Fishing Stats actions on this client.
	static string LRP_GetStatsText()
	{
		return string.Format("Fish: %1 | Total: %2\nLargest: %3\nOld boots: %4 | Golden perch: %5",
			s_iCatchCount, LRP_FormatWeight(s_iTotalWeightDecigrams),
			LRP_FormatWeight(s_iBiggestWeightDecigrams), s_iBootCount, s_iGoldenPerchCount);
	}

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

		// The configured hold duration has completed at this point.
		if (Math.RandomInt(0, 100) >= 70)
		{
			Print("[LeafRP] Virtual fishing: no bite.", LogLevel.NORMAL);
			SCR_HintManagerComponent.ShowCustomHint("No bite this time. Try again!", "LeafRP Fishing", 4.0);
			return;
		}

		// Rare finds are decided after the successful bite:
		// 3% old boot (junk), 2% golden perch, otherwise ordinary fish.
		int rareRoll = Math.RandomInt(0, 100);
		if (rareRoll < 3)
		{
			s_iBootCount++;
			Print("[LeafRP] Virtual fishing: caught an old boot.", LogLevel.NORMAL);
			SCR_HintManagerComponent.ShowCustomHint("You fished up an old boot!\n" + LRP_GetStatsText(), "LeafRP Fishing", 6.0);
			return;
		}

		bool goldenPerch = rareRoll < 5;
		// Standard species probabilities remain 50/35/15 for non-rare fish.
		int speciesRoll = Math.RandomInt(0, 100);
		string species;
		int weightTenthsKg;
		if (speciesRoll < 50)
		{
			species = "Perch";
			weightTenthsKg = Math.RandomInt(2, 17); // 0.2 - 1.6 kg
		}
		else if (speciesRoll < 85)
		{
			species = "Herring";
			weightTenthsKg = Math.RandomInt(1, 7); // 0.1 - 0.6 kg
		}
		else
		{
			species = "Pike";
			weightTenthsKg = Math.RandomInt(10, 121); // 1.0 - 12.0 kg
		}

		if (goldenPerch)
		{
			species = "Golden perch";
			weightTenthsKg = Math.RandomInt(10, 31); // 1.0 - 3.0 kg
			s_iGoldenPerchCount++;
		}

		s_iCatchCount++;
		s_iTotalWeightDecigrams += weightTenthsKg;
		bool isRecord = weightTenthsKg > s_iBiggestWeightDecigrams;
		if (isRecord)
			s_iBiggestWeightDecigrams = weightTenthsKg;

		string message = string.Format("Caught a %1 (%2)!\nCatches: %3 | Total: %4 | Biggest: %5",
			species, LRP_FormatWeight(weightTenthsKg), s_iCatchCount,
			LRP_FormatWeight(s_iTotalWeightDecigrams), LRP_FormatWeight(s_iBiggestWeightDecigrams));
		if (isRecord)
			message += "\nNew personal best this session!";

		Print(string.Format("[LeafRP] Fishing catch: %1, weight=%2, catches=%3, total=%4, biggest=%5.",
			species, LRP_FormatWeight(weightTenthsKg), s_iCatchCount,
			LRP_FormatWeight(s_iTotalWeightDecigrams), LRP_FormatWeight(s_iBiggestWeightDecigrams)), LogLevel.NORMAL);
		SCR_HintManagerComponent.ShowCustomHint(message, "LeafRP Fishing", 6.0);
	}

	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
}
