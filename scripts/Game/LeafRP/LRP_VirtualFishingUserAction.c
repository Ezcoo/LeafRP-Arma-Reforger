// LeafRP virtual fishing v0.4.
// Start Fishing and Hook Fish are instant actions. The randomized waiting
// period and 1-second hook window run in the local script. No physical items.
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
	protected static int s_iFishingState; // 0 idle, 1 waiting, 2 bite
	protected static int s_iAttemptToken;
	protected static vector s_vFishingOrigin;

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


	static bool LRP_IsWaiting()
	{
		return s_iFishingState == 1;
	}

	static bool LRP_HasBite()
	{
		return s_iFishingState == 2;
	}

	static bool LRP_IsFishing()
	{
		return s_iFishingState != 0;
	}

	static bool LRP_IsWithinRange()
	{
		SCR_PlayerController player = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!player)
			return false;
		IEntity character = player.GetControlledEntity();
		if (!character || !ChimeraCharacter.Cast(character))
			return false;
		return (character.GetOrigin() - s_vFishingOrigin).Length() <= 5.0;
	}

	static void LRP_EndAttempt()
	{
		s_iFishingState = 0;
		s_iAttemptToken++;
	}

	static void LRP_CheckRange(int token)
	{
		if (token != s_iAttemptToken || !LRP_IsFishing())
			return;
		if (!LRP_IsWithinRange())
		{
			Print("[LeafRP] Fishing cancelled: moved too far or lost character.", LogLevel.NORMAL);
			SCR_HintManagerComponent.ShowCustomHint("Fishing cancelled: moved too far away.", "LeafRP Fishing", 3.0);
			LRP_EndAttempt();
			return;
		}
		GetGame().GetCallqueue().CallLater(LRP_CheckRange, 250, false, token);
	}

	static void LRP_Bite(int token)
	{
		if (token != s_iAttemptToken || s_iFishingState != 1)
			return;
		if (!LRP_IsWithinRange())
		{
			LRP_EndAttempt();
			return;
		}
		if (Math.RandomInt(0, 100) >= 70)
		{
			Print("[LeafRP] Fishing: no bite this time.", LogLevel.NORMAL);
			SCR_HintManagerComponent.ShowCustomHint("Nothing took the bait. Try again!", "LeafRP Fishing", 4.0);
			LRP_EndAttempt();
			return;
		}

		s_iFishingState = 2;
		Print("[LeafRP] Fishing BITE event: hook within 1 second. Sound cue hook point.", LogLevel.NORMAL);
		// TODO: Play the user's bite sound here once imported as a sound event.
		SCR_HintManagerComponent.ShowCustomHint("BITE! Hook the fish NOW!", "LeafRP Fishing", 1.0);
		GetGame().GetCallqueue().CallLater(LRP_BiteExpired, 1000, false, token);
	}

	static void LRP_BiteExpired(int token)
	{
		if (token != s_iAttemptToken || s_iFishingState != 2)
			return;
		Print("[LeafRP] Fishing: hook window expired.", LogLevel.NORMAL);
		SCR_HintManagerComponent.ShowCustomHint("The fish got away!", "LeafRP Fishing", 3.0);
		LRP_EndAttempt();
	}

	// Called only from the separate Hook Fish user action.
	static void LRP_HookFish()
	{
		if (s_iFishingState != 2 || !LRP_IsWithinRange())
			return;
		LRP_EndAttempt();
		LRP_ResolveCatch();
	}

	override bool GetActionNameScript(out string outName)
	{
		outName = "Start Fishing";
		return true;
	}

	override bool CanBeShownScript(IEntity user)
	{
		return ChimeraCharacter.Cast(user) != null && !LRP_IsFishing();
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
		if (!CanBePerformedScript(pUserEntity) || LRP_IsFishing())
			return;
		s_vFishingOrigin = pUserEntity.GetOrigin();
		s_iFishingState = 1;
		s_iAttemptToken++;
		int token = s_iAttemptToken;
		int waitMs = Math.RandomInt(30000, 120001);
		Print(string.Format("[LeafRP] Fishing started; waiting %1 milliseconds.", waitMs), LogLevel.NORMAL);
		SCR_HintManagerComponent.ShowCustomHint("Waiting for a bite... Stay nearby!", "LeafRP Fishing", 3.0);
		GetGame().GetCallqueue().CallLater(LRP_CheckRange, 250, false, token);
		GetGame().GetCallqueue().CallLater(LRP_Bite, waitMs, false, token);
	}

	static void LRP_ResolveCatch()
	{
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
