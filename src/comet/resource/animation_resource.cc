// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "animation_resource.h"

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"
#include "comet_pch.h"

namespace comet {
namespace resource {
const ResourceTypeId AnimationClipResource::kResourceTypeId{
    COMET_STRING_ID("animation_clip")};

ResourceId GenerateAnimationClipId(const schar* animation_name) {
  return COMET_STRING_ID(animation_name);
}

AnimationClipHandler::AnimationClipHandler(
    memory::Allocator* loading_resources_allocator,
    memory::Allocator* loading_resource_allocator)
    : ResourceHandler{sizeof(AnimationClipResource),
                      loading_resources_allocator, loading_resource_allocator} {
}

usize AnimationClipHandler::GetAnimationClipSize(
    const AnimationClipResource& clip) const {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId)};

  size += sizeof(animation::AnimationId);
  size += sizeof(animation::FrameIndex);
  size += sizeof(animation::FrameIndex);

  size += sizeof(usize);

  for (const auto& sample : clip.clip.samples) {
    size += sizeof(usize);

    for (const auto& pose : sample.joint_poses) {
      size += sizeof(animation::CompressedJointPose);
    }
  }

  size += sizeof(bool);
  return size;
}

ResourceFile AnimationClipHandler::Pack(
    memory::Allocator& allocator, const Resource& resource,
    CompressionMode compression_mode) const {
  const auto& clip_resource{
      static_cast<const AnimationClipResource&>(resource)};
  ResourceFile file{};
  file.resource_id = clip_resource.id;
  file.resource_type_id = AnimationClipResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{&allocator};
  file.data = Array<u8>{&allocator};

  Array<u8> data{&allocator};
  data.Resize(GetAnimationClipSize(clip_resource));
  usize cursor{0};
  auto* buffer{data.GetData()};
  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kAnimationIdSize{sizeof(animation::AnimationId)};
  constexpr auto kFramesPerSecondSize{sizeof(animation::FrameIndex)};
  constexpr auto kFrameCountSize{sizeof(animation::FrameIndex)};
  constexpr auto kSampleCountSize{sizeof(usize)};
  constexpr auto kJointPoseCountSize{sizeof(usize)};
  constexpr auto kJointPoseRotationXSize{sizeof(u16)};
  constexpr auto kJointPoseRotationYSize{sizeof(u16)};
  constexpr auto kJointPoseRotationZSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationXSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationYSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationZSize{sizeof(u16)};
  constexpr auto kJointPoseScaleSize{sizeof(u16)};
  constexpr auto kIsLoopSize{sizeof(bool)};

  const auto& clip{clip_resource.clip};

  memory::CopyMemory(&buffer[cursor], &clip_resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &clip_resource.type_id,
                     kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], &clip.animation_id, kAnimationIdSize);
  cursor += kAnimationIdSize;

  memory::CopyMemory(&buffer[cursor], &clip.frames_per_second,
                     kFramesPerSecondSize);
  cursor += kFramesPerSecondSize;

  memory::CopyMemory(&buffer[cursor], &clip.frame_count, kFrameCountSize);
  cursor += kFrameCountSize;

  auto sample_count{clip.samples.GetSize()};
  memory::CopyMemory(&buffer[cursor], &sample_count, kSampleCountSize);
  cursor += kSampleCountSize;

  for (const auto& sample : clip.samples) {
    auto joint_pose_count{sample.joint_poses.GetSize()};
    memory::CopyMemory(&buffer[cursor], &joint_pose_count, kJointPoseCountSize);
    cursor += kJointPoseCountSize;

    for (const auto& pose : sample.joint_poses) {
      memory::CopyMemory(&buffer[cursor], &pose.rotation_x,
                         kJointPoseRotationXSize);
      cursor += kJointPoseRotationXSize;

      memory::CopyMemory(&buffer[cursor], &pose.rotation_y,
                         kJointPoseRotationYSize);
      cursor += kJointPoseRotationYSize;

      memory::CopyMemory(&buffer[cursor], &pose.rotation_z,
                         kJointPoseRotationZSize);
      cursor += kJointPoseRotationZSize;

      memory::CopyMemory(&buffer[cursor], &pose.translation_x,
                         kJointPoseTranslationXSize);
      cursor += kJointPoseTranslationXSize;

      memory::CopyMemory(&buffer[cursor], &pose.translation_y,
                         kJointPoseTranslationYSize);
      cursor += kJointPoseTranslationYSize;

      memory::CopyMemory(&buffer[cursor], &pose.translation_z,
                         kJointPoseTranslationZSize);
      cursor += kJointPoseTranslationZSize;

      memory::CopyMemory(&buffer[cursor], &pose.scale, kJointPoseScaleSize);
      cursor += kJointPoseScaleSize;
    }
  }

  PackPodResourceDescr(clip_resource.descr, file);
  PackResourceData(data, file);
  return file;
}

Resource* AnimationClipHandler::Unpack(memory::Allocator& allocator,
                                       const ResourceFile& file) {
  auto* clip_resource{
      resource_allocator_.AllocateOneAndPopulate<AnimationClipResource>()};
  UnpackPodResourceDescr<AnimationClipResourceDescr>(file,
                                                     clip_resource->descr);

  Array<u8> data{&allocator};
  UnpackResourceData(file, data);
  auto data_size{sizeof(u8) * data.GetSize()};
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kAnimationIdSize{sizeof(animation::AnimationId)};
  constexpr auto kFramesPerSecondSize{sizeof(animation::FrameIndex)};
  constexpr auto kFrameCountSize{sizeof(animation::FrameIndex)};
  constexpr auto kSampleCountSize{sizeof(usize)};
  constexpr auto kJointPoseCountSize{sizeof(usize)};
  constexpr auto kJointPoseRotationXSize{sizeof(u16)};
  constexpr auto kJointPoseRotationYSize{sizeof(u16)};
  constexpr auto kJointPoseRotationZSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationXSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationYSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationZSize{sizeof(u16)};
  constexpr auto kJointPoseScaleSize{sizeof(u16)};
  constexpr auto kIsLoopSize{sizeof(bool)};

  memory::CopyMemory(&clip_resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&clip_resource->type_id, &buffer[cursor],
                     kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  auto& clip{clip_resource->clip};

  memory::CopyMemory(&clip.animation_id, &buffer[cursor], kAnimationIdSize);
  cursor += kAnimationIdSize;

  memory::CopyMemory(&clip.frames_per_second, &buffer[cursor],
                     kFramesPerSecondSize);
  cursor += kFramesPerSecondSize;

  memory::CopyMemory(&clip.frame_count, &buffer[cursor], kFrameCountSize);
  cursor += kFrameCountSize;

  clip.samples = Array<animation::CompressedAnimationSample>{&allocator};
  usize sample_count;

  memory::CopyMemory(&sample_count, &buffer[cursor], kSampleCountSize);
  cursor += kSampleCountSize;
  clip.samples.Reserve(sample_count);

  for (usize i{0}; i < sample_count; ++i) {
    auto& sample{clip.samples.EmplaceBack()};

    sample.joint_poses = Array<animation::CompressedJointPose>{&allocator};
    usize joint_pose_count;

    memory::CopyMemory(&joint_pose_count, &buffer[cursor], kJointPoseCountSize);
    cursor += kJointPoseCountSize;
    sample.joint_poses.Reserve(sample_count);

    for (usize j{0}; j < joint_pose_count; ++j) {
      auto& pose{sample.joint_poses.EmplaceBack()};

      memory::CopyMemory(&pose.rotation_x, &buffer[cursor],
                         kJointPoseRotationXSize);
      cursor += kJointPoseRotationXSize;

      memory::CopyMemory(&pose.rotation_y, &buffer[cursor],
                         kJointPoseRotationYSize);
      cursor += kJointPoseRotationYSize;

      memory::CopyMemory(&pose.rotation_z, &buffer[cursor],
                         kJointPoseRotationZSize);
      cursor += kJointPoseRotationZSize;

      memory::CopyMemory(&pose.translation_x, &buffer[cursor],
                         kJointPoseTranslationXSize);
      cursor += kJointPoseTranslationXSize;

      memory::CopyMemory(&pose.translation_y, &buffer[cursor],
                         kJointPoseTranslationYSize);
      cursor += kJointPoseTranslationYSize;

      memory::CopyMemory(&pose.translation_z, &buffer[cursor],
                         kJointPoseTranslationZSize);
      cursor += kJointPoseTranslationZSize;

      memory::CopyMemory(&pose.scale, &buffer[cursor], kJointPoseScaleSize);
      cursor += kJointPoseScaleSize;
    }
  }

  memory::CopyMemory(&clip.is_loop, &buffer[cursor], kIsLoopSize);
  cursor += kIsLoopSize;

  return clip_resource;
}
}  // namespace resource
}  // namespace comet
