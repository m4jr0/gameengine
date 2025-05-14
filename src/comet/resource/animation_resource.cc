// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "animation_resource.h"

#include "comet/core/c_string.h"
#include "comet/core/generator.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"

namespace comet {
namespace resource {
const ResourceTypeId AnimationClipResource::kResourceTypeId{
    COMET_STRING_ID("animation_clip")};

ResourceId GenerateAnimationClipId(CTStringView file_path,
                                   const schar* animation_name) {
  auto animation_len{GetLength(animation_name)};

  // Add 1 character for "|".
  auto path_len{file_path.GetLength()};
  usize total_len{path_len + 1 + animation_len};

  constexpr auto kMaxStackBufferSize{512};
  schar* buffer = nullptr;
  schar stack_buffer[kMaxStackBufferSize];

  if (total_len + 1 <= kMaxStackBufferSize) {
    buffer = stack_buffer;
  } else {
    buffer = GenerateForOneFrame<schar>(total_len + 1);
  }

  Copy(buffer, file_path.GetCTStr(), path_len);
  buffer[path_len] = '|';
  Copy(buffer, animation_name, animation_len, path_len + 1);
  buffer[total_len] = '\0';
  return COMET_STRING_ID(buffer);
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

  size += sizeof(animation::AnimationClipId);
  size += sizeof(animation::FrameIndex);
  size += sizeof(animation::FrameIndex);

  size += sizeof(usize);

  for (const auto& sample : clip.clip.samples) {
    size += sizeof(usize);

    auto pose_count{sample.joint_poses.GetSize()};

    for (usize i{0}; i < pose_count; ++i) {
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
  constexpr auto kAnimationClipIdSize{sizeof(animation::AnimationClipId)};
  constexpr auto kFramesPerSecondSize{sizeof(animation::FrameIndex)};
  constexpr auto kFrameCountSize{sizeof(animation::FrameIndex)};
  constexpr auto kSampleCountSize{sizeof(usize)};
  constexpr auto kJointPoseCountSize{sizeof(usize)};
#ifndef COMET_COMPRESS_ANIMATIONS
  constexpr auto kJointPoseRotationSize{sizeof(math::Quat)};
  constexpr auto kJointPoseTranslationSize{sizeof(math::Vec3)};
  constexpr auto kJointPoseScaleSize{sizeof(f32)};
#else
  constexpr auto kJointPoseRotationXSize{sizeof(u16)};
  constexpr auto kJointPoseRotationYSize{sizeof(u16)};
  constexpr auto kJointPoseRotationZSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationXSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationYSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationZSize{sizeof(u16)};
  constexpr auto kJointPoseScaleSize{sizeof(u16)};
#endif  // !COMET_COMPRESS_ANIMATIONS
  constexpr auto kIsLoopSize{sizeof(bool)};

  const auto& clip{clip_resource.clip};

  memory::CopyMemory(&buffer[cursor], &clip_resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &clip_resource.type_id,
                     kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  memory::CopyMemory(&buffer[cursor], &clip.id, kAnimationClipIdSize);
  cursor += kAnimationClipIdSize;

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
#ifndef COMET_COMPRESS_ANIMATIONS
      memory::CopyMemory(&buffer[cursor], &pose.rotation,
                         kJointPoseRotationSize);
      cursor += kJointPoseRotationSize;
      memory::CopyMemory(&buffer[cursor], &pose.translation,
                         kJointPoseTranslationSize);
      cursor += kJointPoseTranslationSize;
      memory::CopyMemory(&buffer[cursor], &pose.scale, kJointPoseScaleSize);
      cursor += kJointPoseScaleSize;
#else
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
#endif  // COMET_COMPRESS_ANIMATIONS
    }
  }

  memory::CopyMemory(&buffer[cursor], &clip.is_loop, kIsLoopSize);
  cursor += kIsLoopSize;

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
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(resource::ResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(resource::ResourceTypeId)};
  constexpr auto kAnimationClipIdSize{sizeof(animation::AnimationClipId)};
  constexpr auto kFramesPerSecondSize{sizeof(animation::FrameIndex)};
  constexpr auto kFrameCountSize{sizeof(animation::FrameIndex)};
  constexpr auto kSampleCountSize{sizeof(usize)};
  constexpr auto kJointPoseCountSize{sizeof(usize)};
#ifndef COMET_COMPRESS_ANIMATIONS
  constexpr auto kJointPoseRotationSize{sizeof(math::Quat)};
  constexpr auto kJointPoseTranslationSize{sizeof(math::Vec3)};
  constexpr auto kJointPoseScaleSize{sizeof(f32)};
#else
  constexpr auto kJointPoseRotationXSize{sizeof(u16)};
  constexpr auto kJointPoseRotationYSize{sizeof(u16)};
  constexpr auto kJointPoseRotationZSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationXSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationYSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationZSize{sizeof(u16)};
  constexpr auto kJointPoseScaleSize{sizeof(u16)};
#endif  // !COMET_COMPRESS_ANIMATIONS
  constexpr auto kIsLoopSize{sizeof(bool)};

  memory::CopyMemory(&clip_resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&clip_resource->type_id, &buffer[cursor],
                     kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  auto& clip{clip_resource->clip};

  memory::CopyMemory(&clip.id, &buffer[cursor], kAnimationClipIdSize);
  cursor += kAnimationClipIdSize;

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

#ifndef COMET_COMPRESS_ANIMATIONS
      memory::CopyMemory(&pose.rotation, &buffer[cursor],
                         kJointPoseRotationSize);
      cursor += kJointPoseRotationSize;

      memory::CopyMemory(&pose.translation, &buffer[cursor],
                         kJointPoseTranslationSize);
      cursor += kJointPoseTranslationSize;

      memory::CopyMemory(&pose.scale, &buffer[cursor], kJointPoseScaleSize);
      cursor += kJointPoseScaleSize;
#else
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
#endif  // !COMET_COMPRESS_ANIMATIONS
    }
  }

  memory::CopyMemory(&clip.is_loop, &buffer[cursor], kIsLoopSize);
  cursor += kIsLoopSize;

  return clip_resource;
}
}  // namespace resource
}  // namespace comet
