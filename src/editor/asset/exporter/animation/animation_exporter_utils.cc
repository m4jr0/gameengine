// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "animation_exporter_utils.h"

#include "comet/core/c_string.h"
#include "comet_pch.h"

namespace comet {
namespace editor {
namespace asset {
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

u32 ResolveKeyIndex(const aiVectorKey* vector_keys, u32 vector_key_count,
                    f64 time) {
  for (u32 i{0}; i < vector_key_count - 1; ++i) {
    if (time < vector_keys[i + 1].mTime) {
      return i;
    }
  }

  return vector_key_count - 1;
}

u32 ResolveKeyIndex(const aiQuatKey* quat_keys, u32 quat_key_count, f64 time) {
  for (u32 i{0}; i < quat_key_count - 1; ++i) {
    if (time < quat_keys[i + 1].mTime) {
      return i;
    }
  }

  return quat_key_count - 1;
}

animation::JointPose ExtractPose(const aiNodeAnim* channel, f64 time) {
  animation::JointPose pose{};

  if (channel->mNumPositionKeys > 0) {
    auto pos_key_index{ResolveKeyIndex(channel->mPositionKeys,
                                       channel->mNumPositionKeys, time)};
    const auto& pos_key{channel->mPositionKeys[pos_key_index].mValue};
    pose.translation = math::Vec3{pos_key.x, pos_key.y, pos_key.z};
  }

  if (channel->mNumRotationKeys > 0) {
    auto rot_key_index{ResolveKeyIndex(channel->mRotationKeys,
                                       channel->mNumRotationKeys, time)};
    const auto& rot_key{channel->mRotationKeys[rot_key_index].mValue};
    pose.rotation = math::Quat{rot_key.x, rot_key.y, rot_key.z, rot_key.w};
  }

  if (channel->mNumScalingKeys > 0) {
    auto scale_key_index{
        ResolveKeyIndex(channel->mScalingKeys, channel->mNumScalingKeys, time)};
    const auto& scale_key{channel->mScalingKeys[scale_key_index].mValue};
    pose.scale = (scale_key.x + scale_key.y + scale_key.z) / 3.0f;
  } else {
    pose.scale = 1.0f;
  }

  return pose;
}

Array<resource::AnimationClipResource> LoadAnimationClips(
    memory::Allocator* allocator, const aiScene* scene) {
  Array<resource::AnimationClipResource> clips{allocator};
  auto clip_count{scene->mNumAnimations};

  if (clip_count <= 0) {
    return clips;
  }

  clips.Reserve(clip_count);
  constexpr auto kDefaultFrameRate{30};

  for (u32 i{0}; i < clip_count; ++i) {
    const auto* raw_animation{scene->mAnimations[i]};
    auto& clip_resource{clips.EmplaceBack()};
    clip_resource.id =
        resource::GenerateAnimationClipId(raw_animation->mName.C_Str());
    clip_resource.type_id = resource::AnimationClipResource::kResourceTypeId;

    auto& clip{clip_resource.clip};
    clip.animation_id = static_cast<animation::AnimationId>(clip_resource.id);
    clip.frames_per_second = static_cast<u32>(
        raw_animation->mTicksPerSecond != 0 ? raw_animation->mTicksPerSecond
                                            : kDefaultFrameRate);
    clip.frame_count = static_cast<u32>(raw_animation->mDuration);
    clip.is_loop = IsLoop(raw_animation->mName.C_Str());
    clip.samples = Array<animation::CompressedAnimationSample>(allocator);
    clip.samples.Reserve(clip.frame_count);
    const u32 joint_count = raw_animation->mNumChannels;

    for (u32 frame{0}; frame < clip.frame_count; ++frame) {
      auto& sample{clip.samples.EmplaceBack()};
      sample.joint_poses = Array<animation::CompressedJointPose>{allocator};
      sample.joint_poses.Resize(joint_count);
      auto time{frame * clip.frames_per_second};

      for (u32 joint_index{0}; joint_index < joint_count; ++joint_index) {
        const auto* channel{raw_animation->mChannels[joint_index]};
        const auto pose{ExtractPose(channel, time)};
        const auto compressed{CompressJointPose(pose)};
        sample.joint_poses.EmplaceBack(compressed);
      }
    }
  }

  return clips;
}
}  // namespace asset
}  // namespace editor
}  // namespace comet
