# LeafRP gameplay smoke test: welcome on character control

This feature shows a local **LeafRP** hint about 1.5 seconds after your client starts controlling a soldier character. It also works with **Game Master possession**, which does not necessarily trigger the ordinary player-spawn callback.

## Source
- `scripts/Game/LeafRP/LRP_WelcomePlayerController.c` mods `SCR_PlayerController`.
- The code listens to `OnControlledEntityChanged` and only shows a hint on the local player's controller.
- The hint is displayed through the local `SCR_HintManagerComponent` and its boolean result is logged.
- `ShowCustomHint` is a quick prototyping tool and should eventually be replaced with a configured and localized hint.

## Test in Workbench
1. Ensure this branch is checked out in the LeafRP mod directory and reopen `RP_TEST` in Workbench. Make sure your mod is among the loaded projects.
2. Compile scripts.
3. Open a playable scenario (Game Master is suitable), press Play, then **take control of a soldier character**. Simply opening the editor or placing an AI character is not enough.
4. Around 1.5 seconds after taking control, look for the **LeafRP** hint.

## Debugging (important)
Search the **gameplay / World Editor Play session** logs for `[LeafRP]`, not the GitHub Actions compiler logs.

- No `[LeafRP]` messages: wrong mod loaded, stale scripts, or the scenario did not initialize a `SCR_PlayerController`.
- `Controlled entity changed` with a different local player ID, or `local=0`: the local-player check rejected the event. Report these values.
- `Local character controlled` but `no SCR_HintManagerComponent`: the scenario lacks the hint-manager game-mode component. Try a standard scenario with UI/hints or add the component to a test game mode.
- `Hint manager refused`: inspect hint settings and whether the UI is active.
- `Welcome hint displayed successfully` but no visible popup: the hint subsystem accepted the request; investigate the presentation/UI context.

The Linux GitHub Actions job **only verifies compilation**; the actual visibility test must happen in-game. The feature is not yet verified for dedicated-server multiplayer sessions.
