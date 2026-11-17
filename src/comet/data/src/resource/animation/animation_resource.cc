// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_data_pch.h"
#include "comet/core/memory/memory_utils.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/animation/animation_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include <type_traits>

#include "comet/core/container/array.h"
#include "comet/core/id/string_id.h"
#include "comet/core/id/string_id_allocator.h"

namespace comet {
namespace resource {
const AnimationClipResource::TypeId AnimationClipResource::kResourceTypeId{
    COMET_STRING_ID(AnimationClipResource::kResourceTypeName.data())};

AnimationClipResource::Id AnimationClipResource::GetId() const noexcept {
  return Id{id};
}

usize GetAnimationClipSize(const AnimationClipResource& resource) {
  auto size{sizeof(RawResourceId) + sizeof(ResourceTypeId)};

  size += sizeof(animation::AnimationClipId);
  size += sizeof(animation::FrameIndex);
  size += sizeof(animation::FrameIndex);

  size += sizeof(usize);

  for (const auto& sample : resource.clip.samples) {
    size += sizeof(usize);
    size +=
        sample.joint_poses.GetSize() * sizeof(animation::CompressedJointPose);
  }

  size += sizeof(bool);
  return size;
}
namespace packing_internal {
template <typename T>
void WriteValue(u8* buffer, usize& cursor, const T& value) {
  static_assert(std::is_trivially_copyable_v<T>);
  memory::CopyMemory(&buffer[cursor], &value, sizeof(T));
  cursor += sizeof(T);
}

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
}

ResourceFile PackAnimationClipResource(
    const AnimationClipResource& resource, CompressionMode compression_mode, memory::Allocator* allocator) {
  COMET_ASSERT(allocator != nullptr, "resource::PackAnimationClipResource", "allocator is null");
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
  file.descr = Array<u8>{allocator};
  file.data = Array<u8>{allocator};

  Array<u8> data{allocator};
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
  constexpr auto kJointPoseRotationXSize{sizeof(u16)};
  constexpr auto kJointPoseRotationYSize{sizeof(u16)};
  constexpr auto kJointPoseRotationZSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationXSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationYSize{sizeof(u16)};
  constexpr auto kJointPoseTranslationZSize{sizeof(u16)};
  constexpr auto kJointPoseScaleSize{sizeof(u16)};
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
      packing_internal::WriteCompressedJointPose(buffer, cursor, pose);
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

namespace decoding_internal {
template <typename T>
void ReadValue(const u8* buffer, usize& cursor, T& value) {
  static_assert(std::is_trivially_copyable_v<T>);
  memory::CopyMemory(&value, &buffer[cursor], sizeof(T));
  cursor += sizeof(T);
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
}  // namespace decoding_internal

void UnpackAnimationClipResource(const ResourceFile& file,
                                          memory::Allocator* scratch_allocator,
    memory::Allocator* payload_allocator,
                                          AnimationClipResource* resource) {
  COMET_ASSERT(resource != nullptr, "AnimationClipResourceHandler::Unpack",
               "animation clip resource is null");

  UnpackPodResourceDescr<AnimationClipResourceDescr>(file, resource->descr);

  Array<u8> data{scratch_allocator};
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
      payload_allocator};

  usize sample_count{0};
  memory::CopyMemory(&sample_count, &buffer[cursor], kSampleCountSize);
  cursor += kSampleCountSize;

  clip.samples.Reserve(sample_count);

  for (usize i{0}; i < sample_count; ++i) {
    auto& sample{clip.samples.EmplaceLast()};

    sample.joint_poses = Array<animation::CompressedJointPose>{
        payload_allocator};

    usize joint_pose_count{0};
    memory::CopyMemory(&joint_pose_count, &buffer[cursor], kJointPoseCountSize);
    cursor += kJointPoseCountSize;

    sample.joint_poses.Reserve(joint_pose_count);

    for (usize j{0}; j < joint_pose_count; ++j) {
      auto& pose{sample.joint_poses.EmplaceLast()};
      decoding_internal::ReadCompressedJointPose(buffer, cursor, pose);
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