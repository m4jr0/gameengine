// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "comet/data/resource/animation/animation_resource.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/type/array.h"
#include "comet/core/type/string_id.h"

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
}  // namespace resource
}  // namespace comet