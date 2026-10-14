# LeafRP: hands-up interaction prototype (phase 1)

This is a **hands-up control-system prototype**, not yet a complete surrender animation. Arma Reforger's stock `ECharacterGestures` enum has a **one-handed COMMAND_STOP** gesture, which we use temporarily while we implement a genuine two-hands-up animation.

## What's implemented
- A player-controlled gesture that toggles on and off.
- Local player filtering, safe handling of controlled-entity changes, logging and UI feedback.
- Reuses vanilla `TacticalPingHold` input event so no input config or prefab modifications are needed for this first test.

## Limitations
- **The pose raises one hand, not both.**
- Temporarily hijacks the vanilla **hold-to-point** tactical gesture; its original behavior is unavailable while this prototype is enabled.
- Uses a local toggle, not a server-authoritative replicated surrender state. Other players' visuals and roleplay restrictions have **not** been tested.
- It does not block shooting, weapons, movement, or inventory; it is not a reliable surrender state for multiplayer.

## Test
1. Check out branch `feature/hands-up-prototype`, then reopen your LeafRP mod in Workbench and compile scripts.
2. Open a playable scenario (Game Master is fine) and possess a soldier.
3. Put away any weapons, stand still and look at the soldier in third person.
4. Press the existing control bound to **TacticalPingHold** (look in game Controls). Press again to lower the hand.
5. Confirm the hint says **Hands-up prototype: one-hand STOP gesture**, and that the hand goes up/down.

The action toggles on key DOWN and ignores release, so it should stay up until pressed again. If nothing happens, examine **local gameplay logs** for lines containing `[LeafRP] Hands-up prototype`.

## Next iteration
- Add or configure a real two-hands-up animation asset and connect it to the character animation graph.
- Create a dedicated `LeafRPHandsUp` rebindable action instead of taking over `TacticalPingHold`.
- Put surrender state and permitted/blocked actions under server authority, sync to all clients and reset on death, vehicle entry, weapon changes and possession.
