# LeafRP first gameplay smoke test: Welcome hint

This small feature verifies more than script compilation: it should show a visible welcome hint after the **local player spawns** in a scenario using a `SCR_BaseGameMode`-derived game mode.

## Code

- `scripts/Game/LeafRP/LRP_WelcomeGameMode.c` extends the built-in `SCR_BaseGameMode` with a `modded class`.
- Calls the original `OnPlayerSpawned` handler, checks that it is the local player, and schedules a hint after 1.5 seconds.
- Uses `SCR_HintManagerComponent.ShowCustomHint` (prototyping API; the eventual RP UI should use configured, localized UI infos).

No custom prefab is necessary for this prototype. Existing scenarios must actually use a compatible `SCR_BaseGameMode` and offer a spawnable player.

## Test in Workbench

1. Pull the feature branch into your local mod project (or, after merging, pull `main`).
2. Open **both base-game project and LeafRP** in Workbench.
3. Compile scripts. Start a playable scenario/world that uses `SCR_BaseGameMode` or one of its subclasses. You need a working player spawn.
4. Spawn your player. Around 1.5 seconds later, a hint titled **LeafRP** should appear for about eight seconds.
5. Inspect Workbench logs for `[LeafRP] Local player spawned` and `[LeafRP] Welcome hint requested`.

If you see neither log entry, verify that the scenario's game mode derives from `SCR_BaseGameMode` and that the mod is active. If you see the first but not the second, check callqueue and log errors. If the second appears but the UI does not, check whether gameplay hints are enabled and whether the client-side hint manager exists.

## Validation

GitHub Actions compiles the committed source through Workbench on the Linux self-hosted runner. Green CI confirms **compilation**, not that the welcome hint rendered in-game; the last part needs a manual test.

## Multiplayer note

The player-ID comparison is intended to avoid displaying the hint for unrelated players on a client. It is not proof of multiplayer client-side event delivery; confirm the behavior with a connected client before relying on it for RP servers.
