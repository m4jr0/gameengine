// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/animation/animation_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/memory_utils.h"
#include "comet/core/type/array.h"

namespace comet {
namespace resource {
AnimationClipResourceHandler::AnimationClipResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

void AnimationClipResourceHandler::OnInitialize() {
  anim_allocator_ = memory::FiberFreeListAllocator{
      kAnimAllocatorElementSize_, kDefaultAllocatorCapacity_,
      memory::kEngineMemoryTagResourceAnimation};

  anim_allocator_.Initialize();
}

void AnimationClipResourceHandler::OnDestroy() { anim_allocator_.Destroy(); }

ResourceFile AnimationClipResourceHandler::Pack(
    const AnimationClipResource& resource, CompressionMode compression_mode) {
  COMET_ASSERT(resource.type_id == AnimationClipResource::kResourceTypeId,
               "AnimationClipResourceHandler::Pack",
               "animation clip resource type id is invalid", "resource_type_id",
               resource.type_id, "expected_type_id",
               AnimationClipResource::kResourceTypeId);
  COMET_ASSERT(resource.clip.frames_per_second > 0,
               "AnimationClipResourceHandler::Pack",
               "animation clip frames per second is invalid",
               "frames_per_second", resource.clip.frames_per_second);

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

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
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
#endif  // COMET_COMPRESS_ANIMATIONS
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

  const auto sample_count{clip.samples.GetSize()};
  memory::CopyMemory(&buffer[cursor], &sample_count, kSampleCountSize);
  cursor += kSampleCountSize;

  for (const auto& sample : clip.samples) {
    const auto joint_pose_count{sample.joint_poses.GetSize()};
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

  COMET_ASSERT(cursor == data.GetSize(), "AnimationClipResourceHandler::Pack",
               "packed animation clip size mismatch", "cursor", cursor,
               "data_size", data.GetSize());

  PackPodResourceDescr(resource.descr, file);
  PackResourceData(data, file);
  return file;
}

void AnimationClipResourceHandler::Unpack(const ResourceFile& file,
                                          ResourceLifeSpan life_span,
                                          AnimationClipResource* resource) {
  COMET_ASSERT(resource != nullptr, "AnimationClipResourceHandler::Unpack",
               "animation clip resource is null");

  UnpackPodResourceDescr<AnimationClipResourceDescr>(file, resource->descr);

  Array<u8> data{byte_allocator_};
  UnpackResourceData(file, data);
  const auto* buffer{data.GetData()};
  usize cursor{0};

  constexpr auto kResourceIdSize{sizeof(RawResourceId)};
  constexpr auto kResourceTypeIdSize{sizeof(ResourceTypeId)};
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
#endif  // COMET_COMPRESS_ANIMATIONS
  constexpr auto kIsLoopSize{sizeof(bool)};

  memory::CopyMemory(&resource->id, &buffer[cursor], kResourceIdSize);
  cursor += kResourceIdSize;

  memory::CopyMemory(&resource->type_id, &buffer[cursor], kResourceTypeIdSize);
  cursor += kResourceTypeIdSize;
  COMET_ASSERT(resource->type_id == AnimationClipResource::kResourceTypeId,
               "AnimationClipResourceHandler::Unpack",
               "animation clip resource type id is invalid", "resource_type_id",
               resource->type_id, "expected_type_id",
               AnimationClipResource::kResourceTypeId);

  resource->life_span = life_span;

  auto& clip{resource->clip};

  memory::CopyMemory(&clip.id, &buffer[cursor], kAnimationClipIdSize);
  cursor += kAnimationClipIdSize;

  memory::CopyMemory(&clip.frames_per_second, &buffer[cursor],
                     kFramesPerSecondSize);
  cursor += kFramesPerSecondSize;
  COMET_ASSERT(clip.frames_per_second > 0,
               "AnimationClipResourceHandler::Unpack",
               "animation clip frames per second is invalid",
               "frames_per_second", clip.frames_per_second);

  memory::CopyMemory(&clip.frame_count, &buffer[cursor], kFrameCountSize);
  cursor += kFrameCountSize;

  clip.samples = Array<animation::CompressedAnimationSample>{
      ResolveAllocator(&anim_allocator_, life_span)};

  usize sample_count{0};
  memory::CopyMemory(&sample_count, &buffer[cursor], kSampleCountSize);
  cursor += kSampleCountSize;

  clip.samples.Reserve(sample_count);

  for (usize i{0}; i < sample_count; ++i) {
    auto& sample{clip.samples.EmplaceLast()};

    sample.joint_poses = Array<animation::CompressedJointPose>{
        ResolveAllocator(&anim_allocator_, life_span)};

    usize joint_pose_count{0};
    memory::CopyMemory(&joint_pose_count, &buffer[cursor], kJointPoseCountSize);
    cursor += kJointPoseCountSize;

    sample.joint_poses.Reserve(joint_pose_count);

    for (usize j{0}; j < joint_pose_count; ++j) {
      auto& pose{sample.joint_poses.EmplaceLast()};

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
#endif  // COMET_COMPRESS_ANIMATIONS
    }
  }

  memory::CopyMemory(&clip.is_loop, &buffer[cursor], kIsLoopSize);
  cursor += kIsLoopSize;

  COMET_ASSERT(cursor == data.GetSize(), "AnimationClipResourceHandler::Unpack",
               "unpacked animation clip size mismatch", "cursor", cursor,
               "data_size", data.GetSize());
}
}  // namespace resource
}  // namespace comet