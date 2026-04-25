// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_ANIMATION_ANIMATION_RESOURCE_H_
#define COMET_COMET_RESOURCE_ANIMATION_ANIMATION_RESOURCE_H_

#include "comet/animation/type/animation_clip.h"
#include "comet/core/essentials.h"
#include "comet/resource/resource.h"
#include "comet/resource/runtime/loaded_resource_handle.h"
#include "comet/resource/type/common.h"

namespace comet {
namespace resource {
struct AnimationClipResourceTag {};

using AnimationClipResourceId = ResourceIdT<AnimationClipResourceTag>;
using AnimationClipResourceHandle =
    LoadedResourceHandle<AnimationClipResourceTag>;

struct AnimationClipResourceDescr {
  u8 empty{0};
};

struct AnimationClipResource : Resource {
  using Id = AnimationClipResourceId;
  using Handle = AnimationClipResourceHandle;
  using TypeId = ResourceTypeId;
  using TypeName = ResourceTypeName;

  static constexpr TypeName kResourceTypeName{"animation_clip"};
  static const TypeId kResourceTypeId;

  AnimationClipResourceDescr descr{};
  animation::CompressedAnimationClip clip{};

  Id GetId() const noexcept;
};

usize GetAnimationClipSize(const AnimationClipResource& resource);
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_ANIMATION_ANIMATION_RESOURCE_H_
