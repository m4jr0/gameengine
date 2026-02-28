// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "animation_export_utils.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/geometry/geometry_common.h"
#include "comet/math/geometry.h"
#include "comet/math/vector.h"
#include "comet/resource/resource.h"
#include "editor/asset/exporter/assimp_utils.h"

namespace comet {
namespace editor {
namespace asset {
geometry::SkeletonJointId GenerateSkeletonJointId(const aiNode* node) {
  return COMET_STRING_ID(node->mName.C_Str());
}

geometry::SkeletonJointId GenerateSkeletonJointId(const aiNodeAnim* node) {
  return COMET_STRING_ID(node->mNodeName.C_Str());
}

bool IsLoop(const schar* animation_name) {
  // TODO(m4jr0): Support animation metadata to avoid using this trick.
  constexpr StaticArray loop_animation_words{"idle", "walk", "run"};

  for (const auto* word : loop_animation_words) {
    if (IsContainedInsensitive(animation_name, word)) {
      return true;
    }
  }

  return false;
}

u32 ResolveAnimationKeyIndex(const aiVectorKey* vector_keys,
                             u32 vector_key_count, f64 tick_time) {
  for (u32 i{0}; i < vector_key_count - 1; ++i) {
    if (tick_time < vector_keys[i + 1].mTime) {
      return i;
    }
  }

  return vector_key_count - 1;
}

u32 ResolveAnimationKeyIndex(const aiQuatKey* quat_keys, u32 quat_key_count,
                             f64 tick_time) {
  for (u32 i{0}; i < quat_key_count - 1; ++i) {
    if (tick_time < quat_keys[i + 1].mTime) {
      return i;
    }
  }

  return quat_key_count - 1;
}

animation::JointPose ExtractPose(const aiNodeAnim* channel, f64 tick_time) {
  animation::JointPose pose{};

  if (channel->mNumPositionKeys > 0) {
    auto pos_key_index{ResolveAnimationKeyIndex(
        channel->mPositionKeys, channel->mNumPositionKeys, tick_time)};
    const auto& pos_key{channel->mPositionKeys[pos_key_index].mValue};
    pose.translation = ToVec3(pos_key);
  }

  if (channel->mNumRotationKeys > 0) {
    auto rot_key_index{ResolveAnimationKeyIndex(
        channel->mRotationKeys, channel->mNumRotationKeys, tick_time)};
    const auto& rot_key{channel->mRotationKeys[rot_key_index].mValue};
    pose.rotation = ToQuat(rot_key);
  }

  if (channel->mNumScalingKeys > 0) {
    auto scale_key_index{ResolveAnimationKeyIndex(
        channel->mScalingKeys, channel->mNumScalingKeys, tick_time)};
    const auto& scale_key{channel->mScalingKeys[scale_key_index].mValue};
    pose.scale = math::AverageComponents(ToVec3(scale_key));
  } else {
    pose.scale = 1.0f;
  }

  return pose;
}

animation::JointPose ToJointPose(const math::Mat4& transform) {
  animation::JointPose pose{};
  math::DecomposeTransform(transform, pose.translation, pose.rotation,
                           pose.scale);
  return pose;
}

void PopulateMissingAnimationChannelDataNodes(
    const aiNode* node, AnimationChannelData& channel_data) {
  auto joint_id{GenerateSkeletonJointId(node)};

  if (!channel_data.IsContained(joint_id)) {
    channel_data.Emplace(joint_id,
                         AnimationChannelDataNode{
                             nullptr, ToMat4x4(node->mTransformation), false});
  }

  for (u32 i{0}; i < node->mNumChildren; ++i) {
    PopulateMissingAnimationChannelDataNodes(node->mChildren[i], channel_data);
  }
}

AnimationChannelData GenerateAnimationChannelData(
    ModelExport& model_export, const aiAnimation* raw_animation) {
  AnimationChannelData channel_data{model_export.allocator,
                                    raw_animation->mNumChannels};

  for (u32 c{0}; c < raw_animation->mNumChannels; ++c) {
    const auto* channel{raw_animation->mChannels[c]};
    auto joint_id{COMET_STRING_ID(channel->mNodeName.C_Str())};
    channel_data.Emplace(joint_id,
                         AnimationChannelDataNode{channel, {1.0f}, true});
  }

  PopulateMissingAnimationChannelDataNodes(model_export.scene->mRootNode,
                                           channel_data);
  return channel_data;
}

void PopulateSample(ModelExport& model_export,
                    const geometry::Skeleton& skeleton,
                    const AnimationChannelData& channel_data, u32 tick_delta,
                    animation::FrameIndex frame,
                    animation::CompressedAnimationSample& sample) {
  const auto skeleton_joint_count{static_cast<u32>(skeleton.joints.GetSize())};
  sample.joint_poses =
      Array<animation::CompressedJointPose>(model_export.allocator);
  sample.joint_poses.Resize(skeleton_joint_count);
  auto tick_time{frame * tick_delta};

  for (u32 joint_index{0}; joint_index < skeleton_joint_count; ++joint_index) {
    const auto& joint{skeleton.joints[joint_index]};
    const auto* channel_node{channel_data.TryGet(joint.id)};
    animation::JointPose pose;

    if (channel_node != nullptr) {
      if (channel_node->has_channel) {
        pose = ExtractPose(channel_node->channel, tick_time);
      } else {
        pose = ToJointPose(channel_node->baked_transform);
      }
    } else {
      pose.translation = {0, 0, 0};
      pose.rotation = {1, 0, 0, 0};
      pose.scale = 1.0f;
    }

    sample.joint_poses[joint_index] = CompressJointPose(pose);
  }
}

void PopulateAnimationClip(ModelExport& model_export,
                           const aiAnimation* raw_animation,
                           const geometry::Skeleton& skeleton,
                           resource::AnimationClipResource& clip_resource) {
  const auto* animation_name{raw_animation->mName.C_Str()};
  COMET_LOG_GLOBAL_DEBUG("Processing ", animation_name, " animation...");
  clip_resource.id =
      resource::GenerateAnimationClipId(model_export.path, animation_name);
  clip_resource.type_id = resource::AnimationClipResource::kResourceTypeId;

  auto& clip{clip_resource.clip};
  clip.id = static_cast<animation::AnimationClipId>(clip_resource.id);
  clip.frames_per_second = kDefaultAnimationFrameRate;

  auto ticks_per_second{static_cast<u32>(raw_animation->mTicksPerSecond != 0
                                             ? raw_animation->mTicksPerSecond
                                             : kDefaultTicksPerSecond)};

  auto duration_seconds{raw_animation->mDuration / ticks_per_second};

  clip.frame_count =
      static_cast<u32>(duration_seconds * clip.frames_per_second);
  clip.is_loop = IsLoop(animation_name);
  auto* allocator{model_export.allocator};
  clip.samples = Array<animation::CompressedAnimationSample>(allocator);
  clip.samples.Reserve(clip.frame_count);

  auto channel_data{GenerateAnimationChannelData(model_export, raw_animation)};
  auto tick_delta{ticks_per_second / clip.frames_per_second};

  for (u32 frame{0}; frame < clip.frame_count; ++frame) {
    auto& sample{clip.samples.EmplaceBack()};
    PopulateSample(model_export, skeleton, channel_data, tick_delta, frame,
                   sample);
  }
}

Array<resource::AnimationClipResource> LoadAnimationClips(
    ModelExport& model_export, const geometry::Skeleton& skeleton) {
  auto* allocator{model_export.allocator};
  Array<resource::AnimationClipResource> clips{allocator};
  const auto* scene{model_export.scene};
  const auto clip_count{scene->mNumAnimations};

  if (clip_count == 0) {
    return clips;
  }

  clips.Reserve(clip_count);

  for (u32 i{0}; i < clip_count; ++i) {
    const auto* raw_animation{scene->mAnimations[i]};
    auto& clip_resource{clips.EmplaceBack()};
    PopulateAnimationClip(model_export, raw_animation, skeleton, clip_resource);
  }

  return clips;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
