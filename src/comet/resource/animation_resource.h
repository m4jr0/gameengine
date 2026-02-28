// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_ANIMATION_RESOURCE_H_
#define COMET_COMET_RESOURCE_ANIMATION_RESOURCE_H_

#include "comet/animation/animation_common.h"
#include "comet/core/essentials.h"
#include "comet/core/type/tstring.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
struct AnimationClipResourceDescr {
  // TODO(m4jr0): Add description.
  u8 empty{0};
};

struct AnimationClipResource : Resource {
  static const ResourceTypeId kResourceTypeId;

  AnimationClipResourceDescr descr{};
  animation::CompressedAnimationClip clip{};
};

ResourceId GenerateAnimationClipId(CTStringView file_path,
                                   const schar* animation_name);

usize GetAnimationClipSize(const AnimationClipResource& resource);
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_ANIMATION_RESOURCE_H_
