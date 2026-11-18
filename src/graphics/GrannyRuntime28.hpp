#pragma once

// Source adapter for Granny 2.8.49.0. Include paths must point to the matching
// official granny.h. This header does not replace that SDK header.
#include <granny.h>

namespace Granny28
{
    using File = granny_file;
    using FileInfo = granny_file_info;
    using Animation = granny_animation;
    using TrackGroup = granny_track_group;
    using Model = granny_model;
    using ModelInstance = granny_model_instance;
    using Skeleton = granny_skeleton;
    using Bone = granny_bone;
    using BoneBinding = granny_bone_binding;
    using Mesh = granny_mesh;
    using MeshBinding = granny_mesh_binding;
    using Transform = granny_transform;
    using LocalPose = granny_local_pose;
    using WorldPose = granny_world_pose;
    using TrackMask = granny_track_mask;
    using Control = granny_control;
    using ControlledAnimationBuilder = granny_controlled_animation_builder;
    using BoxIntersection = granny_box_intersection;
    using LogCallback = granny_log_callback;
    using SystemClock = granny_system_clock;
    using Matrix4x4 = granny_matrix_4x4;

    using ::GrannyBeginControlledAnimation;
    using ::GrannyBuildWorldPose;
    using ::GrannyCompleteControlAt;
    using ::GrannyComputeBasisConversion;
    using ::GrannyControlIsComplete;
    using ::GrannyEaseControlIn;
    using ::GrannyEaseControlOut;
    using ::GrannyEndControlledAnimation;
    using ::GrannyFindBoneByName;
    using ::GrannyFreeControl;
    using ::GrannyFreeControlOnceUnused;
    using ::GrannyFreeFile;
    using ::GrannyFreeLocalPose;
    using ::GrannyFreeMeshBinding;
    using ::GrannyFreeModelInstance;
    using ::GrannyFreeTrackMask;
    using ::GrannyFreeWorldPose;
    using ::GrannyGetControlClock;
    using ::GrannyGetControlLoopCount;
    using ::GrannyGetControlDuration;
    using ::GrannyGetControlDurationLeft;

    using ::GrannyGetFileInfo;
    using ::GrannyGetMeshBindingToBoneIndices;
    using ::GrannyGetSourceSkeleton;
    using ::GrannyGetSystemSeconds;
    using ::GrannyGetWorldPose4x4Array;
    using ::GrannyGetWorldPoseComposite4x4Array;
    using ::GrannyInstantiateModel;
    using ::GrannyNewLocalPose;
    using ::GrannyNewMeshBinding;
    using ::GrannyNewTrackMask;
    using ::GrannyNewWorldPose;
    using ::GrannyRayIntersectsBoxAt;
    using ::GrannyReadEntireFileFromMemory;
    using ::GrannySampleModelAnimations;
    using ::GrannySetControlClock;
    using ::GrannySetControlLoopCount;
    using ::GrannySetControlSpeed;
    using ::GrannySetLogCallback;
    using ::GrannySetTrackGroupModelMask;
    using ::GrannySetTrackGroupTarget;
    using ::GrannyTransformFile;
    using ::GrannyVersionsMatch_;
}

// Engine-specific helper retained from the former GrannyRuntime21.hpp.
void Gfx_TransformGrannyFileToLHS(Granny28::FileInfo* file, float unitsPerMeter);
