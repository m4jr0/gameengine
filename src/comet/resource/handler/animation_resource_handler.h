// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_ANIMATION_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_HANDLER_ANIMATION_RESOURCE_HANDLER_H_

#include "comet/animation/animation_id.h"
#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/resource/animation_resource.h"
#include "comet/resource/handler/resource_handler.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
class AnimationClipResourceHandler
    : public ResourceHandler<animation::AnimationClipTag,
                             AnimationClipResource> {
 public:
  using Base = ResourceHandler;

  explicit AnimationClipResourceHandler(const ResourceHandlerDescr& descr);
  AnimationClipResourceHandler(const AnimationClipResourceHandler&) = delete;
  AnimationClipResourceHandler(AnimationClipResourceHandler&&) = delete;
  AnimationClipResourceHandler& operator=(const AnimationClipResourceHandler&) =
      delete;
  AnimationClipResourceHandler& operator=(AnimationClipResourceHandler&&) =
      delete;
  ~AnimationClipResourceHandler() override = default;

  ResourceFile Pack(const AnimationClipResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              AnimationClipResource* resource) override;

 protected:
  void OnInitialize() override;
  void OnDestroy() override;

 private:
  inline static constexpr usize kDefaultAllocatorCapacity_{1024};
  inline static constexpr usize kAnimAllocatorElementSize_{
      math::Max(sizeof(animation::CompressedAnimationSample),
                sizeof(animation::CompressedJointPose))};
  memory::FiberFreeListAllocator anim_allocator_{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_HANDLER_ANIMATION_RESOURCE_HANDLER_H_