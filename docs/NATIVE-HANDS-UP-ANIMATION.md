# LeafRP original hands-up animation (no ACE dependency)

This branch starts a **native, self-contained** two-hands-up animation. It does not import or depend on ACE, ACE Captives, or any other mod.

## Status

- An original Blender pose-generator script is committed in `tools/blender/make_hands_up_pose.py`.
- The actual Enfusion `.txa` and `.anm` resources **have not been exported yet**.
- No runtime surrender animation is enabled; `main` retains the proven one-hand STOP-gesture prototype.
- The pose generator has **not yet been tested in Blender with the official rig**, so its target positions might need adjustment. Treat the first result as a starting pose, not a finished asset.

## Source files and tools

1. Obtain the official character animation rig [Character_AnimationRig_Example.blend](https://github.com/BohemiaInteractive/Arma-Reforger-Samples/blob/main/SampleMod_NewCharacter/Assets/Characters/SampleCharacter/Character_AnimationRig_Example.blend) from Bohemia Interactive's Arma Reforger Samples repo. Do **not** commit the 10 MB example rig to LeafRP; we need it only for authoring.
2. Install Blender (currently the official Enfusion Blender Tools documentation recommends Blender 4.5 LTS) and Bohemia's **Enfusion Blender Tools** plugin included with Arma Reforger Tools.
3. Extract the Enfusion Blender Tools animation export profiles (the `EBT-ArmaReforger-Data.zip` package in the Steam Tools installation) and set the Export Profile Folder in the Blender plugin preferences.
4. In Workbench's **Options → Workbench**, enable **net API** for communication with Blender. Run Workbench with the LeafRP project loaded during export.
5. Open the official sample `.blend`. Confirm both `hand_ik.L` and `hand_ik.R` controls on the Rig armature are present, and choose an unarmed starting pose.
6. In Blender → Scripting, load and execute `tools/blender/make_hands_up_pose.py` from the LeafRP repository. It creates a new Action called `LRP_HandsUp_Loop` and places both hands above the head.
7. **Inspect the result in Blender Pose Mode.** Tune offsets in the script if needed; the elbow position and palm orientation are not automatically solved or validated by this prototype. Delete/rename the generated action if you need to rerun the script.
8. Use **Enfusion Tools → Export → Enfusion Animation (.txa)** and choose the generated Action. For the first attempt, try profile `252_UpperbodyADD_ArmsABS` (suitable for upper-body movement with absolute arms), and export as `anims/LeafRP/lrp_hands_up_loop.txa` into the loaded LeafRP addon.
9. In Workbench Resource Manager, register/import the new `.txa`. Workbench converts it to an `.anm` and generates resource metadata. With symlinked addons, **manual registration may be required**.
10. Commit the original `.txa` and generated source/resource metadata that the build needs. Do not commit the generated `resourceDatabase.rdb`.

## Next: integrate and test

When we have a real `lrp_hands_up_loop.anm` resource with a stable Workbench GUID, replace the one-handed `ECharacterGestures.COMMAND_STOP` placeholder in `LRP_HandsUpPlayerController.c` with the new animation. Enfusion exposes `CharacterAnimGraphComponent.PlayPostGraphAnimation` and `StopPostGraphAnimation`; the correct animation mask and actual character attachment/component must be verified in Workbench. In particular:

- Confirm the pose visibly raises **both** hands on the stock soldier rig.
- Confirm blend-in/out works and the pose doesn't break locomotion or hand-held objects.
- Confirm multiplayer replication and controls are appropriate. The current one-handed prototype is local-only.
- Add a dedicated keybind once the animation is stable.

A successful GitHub Actions script compilation on its own **cannot verify Blender output or animation behavior**.

## Documentation

- [Bohemia: Enfusion Blender Tools](https://community.bistudio.com/wiki/Arma_Reforger:Enfusion_Blender_Tools)
- [Bohemia: Import / Export Animation](https://community.bistudio.com/wiki/Arma_Reforger:Enfusion_Blender_Tools:_Import/Export_Animation)
- [Bohemia: Animation Export Profiles](https://community.bistudio.com/wiki/Arma_Reforger:Animation_Export_Profiles)
