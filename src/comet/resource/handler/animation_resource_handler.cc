// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "animation_resource_handler.h"

#include "comet/animation/animation_common.h"
#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"

namespace comet {
namespace resource {
AnimationClipResourceHandler::AnimationClipResourceHandler(
    const ResourceHandlerDescr& descr)
    : ResourceHandler<AnimationClipResource>{descr} {}

void AnimationClipResourceHandler::Initialize() {
  ResourceHandler::Initialize();

  anim_allocator_ = memory::FiberFreeListAllocator{
      math::Max(sizeof(animation::AnimationSample),
                sizeof(animation::JointPose)),
      kDefaultAllocatorCapacity_, memory::kEngineMemoryTagResource};

  anim_allocator_.Initialize();
}

void AnimationClipResourceHandler::Destroy() {
  ResourceHandler::Destroy();
  anim_allocator_.Destroy();
}

ResourceFile AnimationClipResourceHandler::Pack(
    const AnimationClipResource& resource, CompressionMode compression_mode) {
  ResourceFile file{};
  file.resource_id = resource.id;
  file.resource_type_id = AnimationClipResource::kResourceTypeId;
  file.compression_mode = compression_mode;
  file.descr = Array<u8>{byte_allocator_};
  file.data = Array<u8>{byte_allocator_};

  Array<u8> data{byte_allocator_};
  data.Resize(GetAnimationClipSize(resource));
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

  const auto& clip{resource.clip};

  memory::CopyMemory(&buffer[cursor], &resource.id, kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&buffer[cursor], &resource.type_id, kResourceTypeIdSize);
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

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void AnimationClipResourceHandler::Unpack(const ResourceFile& file,
                                          ResourceLifeSpan life_span,
                                          AnimationClipResource* resource) {
  UnpackPodResourceDescr<AnimationClipResourceDescr>(file, resource->descr);

  Array<u8> data{byte_allocator_};
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

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;

  auto& clip{resource->clip};

  memory::CopyMemory(&clip.id, &buffer[cursor], kAnimationClipIdSize);
  cursor += kAnimationClipIdSize;

  memory::CopyMemory(&clip.frames_per_second, &buffer[cursor],
                     kFramesPerSecondSize);
  cursor += kFramesPerSecondSize;

  memory::CopyMemory(&clip.frame_count, &buffer[cursor], kFrameCountSize);
  cursor += kFrameCountSize;

  clip.samples = Array<animation::CompressedAnimationSample>{
      ResolveAllocator(&anim_allocator_, life_span)};
  usize sample_count;

  memory::CopyMemory(&sample_count, &buffer[cursor], kSampleCountSize);
  cursor += kSampleCountSize;
  clip.samples.Reserve(sample_count);

  for (usize i{0}; i < sample_count; ++i) {
    auto& sample{clip.samples.EmplaceBack()};

    sample.joint_poses = Array<animation::CompressedJointPose>{
        ResolveAllocator(&anim_allocator_, life_span)};
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
  resource->life_span = life_span;
}
}  // namespace resource
}  // namespace comet
