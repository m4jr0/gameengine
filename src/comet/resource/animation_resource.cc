// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet_pch.h"

#include "animation_resource.h"

#include "comet/core/c_string.h"
#include "comet/core/generator.h"
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

usize GetAnimationClipSize(const AnimationClipResource& resource) {
  usize size{sizeof(ResourceId) + sizeof(ResourceTypeId)};

  size += sizeof(animation::AnimationClipId);
  size += sizeof(animation::FrameIndex);
  size += sizeof(animation::FrameIndex);

  size += sizeof(usize);

  for (const auto& sample : resource.clip.samples) {
    size += sizeof(usize);

    auto pose_count{sample.joint_poses.GetSize()};

    for (usize i{0}; i < pose_count; ++i) {
      size += sizeof(animation::CompressedJointPose);
    }
  }

  size += sizeof(bool);
  return size;
}
}  // namespace resource
}  // namespace comet
