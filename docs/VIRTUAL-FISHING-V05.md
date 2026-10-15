# LeafRP Virtual Fishing v0.5 — Keyboard Hook and Audio Integration

## Status

Prototype code is committed, but **NOT compiled or gameplay-tested** in Workbench. The source audio is supplied separately as `LeafRP_Fishing_v05_Audio.zip` (not included in GitHub, because the available GitHub connector only writes UTF-8 text). Sound playback requires a valid configured **Audio Project (.acp)** and a **SoundComponent** on the fishing spot. A named custom input action must also be created and bound; the source code alone does not register a new key binding.

## What changed

- `LRP_FishingInputPlayerController.c` subscribes to the `LRP_HookFish` input action when the local player takes control of a character; pressing it during the one-second bite window hooks the fish without requiring a contextual interaction.
- `LRP_VirtualFishingUserAction.c` triggers `LRP_FISH_CAST_1` through `LRP_FISH_CAST_4` (random), `LRP_FISH_BITE` (on bite), and `LRP_FISH_REEL` (on successful hook) via the fishing spot's SoundComponent.
- The 5–120 second random wait, one-second reaction, 5 m radius and v0.3 catch statistics/rare finds are unchanged.
- `LRP_HookFishUserAction` remains as a fallback interaction during testing.

## Workbench input configuration required

1. In **Input Manager configuration**, add a **digital** action named `LRP_HookFish` and bind it to **F** using a click/down/pressed-style source; consult the current `chimeraInputCommon.conf` configuration and create a LeafRP addon override or supported mod input preset. **Do not edit the vanilla game input file directly.** Resolve any existing F key conflict in the relevant input context.
2. Verify that `LRP_HookFish` is registered in the live Input Manager and that pressing F fires the listener only once per keypress. The code listens using `AddActionListener("LRP_HookFish", EActionTrigger.DOWN, ...)`.
3. If F conflicts with vanilla bindings, use an alternative default and update the on-screen hint accordingly. The current hint deliberately says `Press your Hook Fish key` rather than promising F before binding is configured.

## Workbench audio configuration required

1. Unzip the six OGG sound samples into your addon, e.g. `Audio/Source/`. You may import/convert them as required by the Workbench Audio Editor.
2. Create a **LeafRP audio project (.acp)** in Audio Editor with six Sound nodes/events named exactly:
   - `LRP_FISH_CAST_1` ... `LRP_FISH_CAST_4` → corresponding water splash OGG samples
   - `LRP_FISH_BITE` → reel_2 OGG
   - `LRP_FISH_REEL` → reel_1 OGG
3. Configure the sample bank, graph connection, and **valid FinalMix/output routing**. Verify events individually in Audio Editor.
4. Select your fishing spot, add **SoundComponent**, and add the LeafRP `.acp` resource to its `Filenames` array. The script fetches it from the fishing spot owner entity during **Start Fishing**.
5. Reload/compile scripts in Workbench Edit mode and test audio from Play mode. Missing sound components log a warning and do not prevent fishing.

## Verification

- [ ] Scripts compile successfully; no additional Workbench instance launched.
- [ ] The new input action is registered and F (or selected key) responds during a bite.
- [ ] Hooking requires no interaction-menu selection.
- [ ] The one-second window starts at bite event / sound trigger, not after the audio clip finishes.
- [ ] Four casting splashes vary across attempts.
- [ ] Bite and reel sounds play correctly, without `SOUND_WAVE` or `OutputState/FinalMix` errors.
- [ ] No sounds or catches trigger from stale expired timers.
- [ ] Old fishing statistics, old-boot and golden-perch counters, and apple inventory actions still work.
- [ ] All multiplayer and per-player persistence behavior remains explicitly unverified.

## Reference

Bohemia Input Manager: https://community.bistudio.com/wiki/Arma_Reforger:Input_Manager

Bohemia audio guide: https://reforger.armaplatform.com/news/modding-boot-camp-7-audio

Bohemia SoundComponent: https://community.bistudio.com/wiki/Arma_Reforger:Audio:_Sound_Components
