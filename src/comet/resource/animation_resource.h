// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_ANIMATION_RESOURCE_H_
#define COMET_COMET_RESOURCE_ANIMATION_RESOURCE_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/resource/resource.h"
#include "comet/animation/animation_common.h"

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

ResourceId GenerateAnimationClipId(const schar* animation_name);

class AnimationClipHandler : public ResourceHandler {
 public:
  AnimationClipHandler(memory::Allocator* loading_resources_allocator,
                       memory::Allocator* loading_resource_allocator);
  AnimationClipHandler(const AnimationClipHandler&) = delete;
  AnimationClipHandler(AnimationClipHandler&&) = delete;
  AnimationClipHandler& operator=(const AnimationClipHandler&) = delete;
  AnimationClipHandler& operator=(AnimationClipHandler&&) = delete;
  virtual ~AnimationClipHandler() = default;

 protected:
  usize GetAnimationClipSize(const AnimationClipResource& clip) const;
  ResourceFile Pack(memory::Allocator& allocator, const Resource& resource,
                    CompressionMode compression_mode) const override;
  Resource* Unpack(memory::Allocator& allocator,
                   const ResourceFile& file) override;
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_ANIMATION_RESOURCE_H_
