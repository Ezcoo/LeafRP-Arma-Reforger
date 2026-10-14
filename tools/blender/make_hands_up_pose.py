"""
LeafRP: author a two-hands-up pose using Bohemia Interactive's official
Character_AnimationRig_Example.blend. No ACE assets or runtime dependencies.

Run in Blender's Scripting workspace after opening the official character rig.
This creates a Blender Action, NOT an Enfusion .anm file. Export via the
Enfusion Blender Tools (EBT) add-on as a .txa, then import in Workbench.

Preview the result and adjust the target positions before export: these
coordinates are a first-pass pose and depend on the particular rig/base pose.
"""
import bpy
from mathutils import Matrix, Vector

ACTION_NAME = "LRP_HandsUp_Loop"
FRAME_START = 1
FRAME_END = 40
# Hand placement is relative to the rig's head control (meters).
HAND_SIDE_OFFSET = 0.32
HAND_FRONT_OFFSET = -0.04
HAND_HEIGHT_OFFSET = 0.16

HAND_CONTROLS = ("hand_ik.L", "hand_ik.R")
HEAD_CONTROLS = ("head", "head_ctrl", "spine.006", "ORG-head")

def find_rig():
    for obj in bpy.data.objects:
        if obj.type != "ARMATURE":
            continue
        if all(name in obj.pose.bones for name in HAND_CONTROLS):
            return obj
    raise RuntimeError(
        "No Rigify control armature with hand_ik.L and hand_ik.R found. "
        "Open Bohemia's Character_AnimationRig_Example.blend first."
    )

def set_control_translation(bone, position):
    # PoseBone.matrix is in armature space. Preserve the hand's rotation.
    target = bone.matrix.copy()
    target.translation = position
    bone.matrix = target

def main():
    rig = find_rig()
    head = next((rig.pose.bones.get(name) for name in HEAD_CONTROLS
                 if rig.pose.bones.get(name)), None)
    if head is None:
        raise RuntimeError(
            "Head control not found; inspect your rig and update HEAD_CONTROLS."
        )

    # The provided sample scene already includes constraints transferring
    # Rigify motion to the character armature. Start from an unarmed base pose.
    bpy.context.view_layer.objects.active = rig
    rig.select_set(True)
    if not rig.animation_data:
        rig.animation_data_create()

    action = bpy.data.actions.get(ACTION_NAME)
    if action:
        raise RuntimeError(
            f"Action {ACTION_NAME!r} already exists. Rename or delete it "
            "to avoid overwriting your work."
        )
    action = bpy.data.actions.new(ACTION_NAME)
    rig.animation_data.action = action

    bpy.context.scene.frame_set(FRAME_START)
    bpy.context.view_layer.update()

    # Place both hands next to and just above the head.
    head_pos = head.matrix.translation.copy()
    for name in HAND_CONTROLS:
        hand = rig.pose.bones[name]
        current = hand.matrix.translation
        side = 1.0 if current.x >= head_pos.x else -1.0
        destination = Vector((
            head_pos.x + side * HAND_SIDE_OFFSET,
            head_pos.y + HAND_FRONT_OFFSET,
            head_pos.z + HAND_HEIGHT_OFFSET,
        ))
        set_control_translation(hand, destination)
        bpy.context.view_layer.update()
        hand.keyframe_insert(data_path="location", frame=FRAME_START, group=name)
        hand.keyframe_insert(
            data_path="rotation_quaternion", frame=FRAME_START, group=name
        )
        hand.keyframe_insert(data_path="location", frame=FRAME_END, group=name)
        hand.keyframe_insert(
            data_path="rotation_quaternion", frame=FRAME_END, group=name
        )

    bpy.context.scene.frame_start = FRAME_START
    bpy.context.scene.frame_end = FRAME_END
    bpy.context.scene.frame_set(FRAME_START)
    bpy.context.view_layer.update()

    print(
        f"[LeafRP] Created {ACTION_NAME} on '{rig.name}'. "
        "Inspect both hands and elbows in Pose Mode. "
        "Export with EBT to anims/LeafRP/lrp_hands_up_loop.txa "
        "using a suitable arms/upper-body export profile."
    )

if __name__ == "__main__":
    main()
