// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_runtime_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/runtime/resource/animation/animation_resource_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/container/array.h"
#include "comet/core/memory/memory_utils.h"

namespace comet {
namespace resource {
namespace internal {
void WriteCompressedJointPose(u8* buffer, usize& cursor,
                              const animation::CompressedJointPose& pose) {
  WriteValue(buffer, cursor, pose.rotation_x);
  WriteValue(buffer, cursor, pose.rotation_y);
  WriteValue(buffer, cursor, pose.rotation_z);
  WriteValue(buffer, cursor, pose.translation_x);
  WriteValue(buffer, cursor, pose.translation_y);
  WriteValue(buffer, cursor, pose.translation_z);
  WriteValue(buffer, cursor, pose.scale);
}

void ReadCompressedJointPose(const u8* buffer, usize& cursor,
                             animation::CompressedJointPose& pose) {
  ReadValue(buffer, cursor, pose.rotation_x);
  ReadValue(buffer, cursor, pose.rotation_y);
  ReadValue(buffer, cursor, pose.rotation_z);
  ReadValue(buffer, cursor, pose.translation_x);
  ReadValue(buffer, cursor, pose.translation_y);
  ReadValue(buffer, cursor, pose.translation_z);
  ReadValue(buffer, cursor, pose.scale);
}
}  // namespace internal

AnimationClipResourceHandler::AnimationClipResourceHandler(
    const ResourceHandlerDescr& descr)
    : Base{descr} {}

void AnimationClipResourceHandler::OnInitialize() {
  anim_allocator_ = memory::FiberFreeListAllocator{
      kAnimAllocatorElementSize_, kDefaultAllocatorCapacity_,
      kEngineMemoryTagResourceAnimation};

  anim_allocator_.Initialize();
}

void AnimationClipResourceHandler::OnDestroy() { anim_allocator_.Destroy(); }

ResourceFile AnimationClipResourceHandler::Pack(const AnimationClipResource& resource,
    CompressionMode compression_mode) {
  return PackAnimationClipResource(resource, compression_mode, byte_allocator_);
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
      internal::ReadCompressedJointPose(buffer, cursor, pose);
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