// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RESOURCE_HANDLER_ANIMATION_RESOURCE_HANDLER_H_
#define COMET_COMET_RESOURCE_HANDLER_ANIMATION_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/resource/animation_resource.h"
#include "comet/resource/handler/resource_handler.h"
#include "comet/resource/resource.h"

namespace comet {
namespace resource {
class AnimationClipResourceHandler
    : public ResourceHandler<AnimationClipResource> {
 public:
  AnimationClipResourceHandler(const ResourceHandlerDescr& descr);
  AnimationClipResourceHandler(const AnimationClipResourceHandler&) = delete;
  AnimationClipResourceHandler(AnimationClipResourceHandler&&) = delete;
  AnimationClipResourceHandler& operator=(const AnimationClipResourceHandler&) =
      delete;
  AnimationClipResourceHandler& operator=(AnimationClipResourceHandler&&) =
      delete;
  virtual ~AnimationClipResourceHandler() = default;

  void Initialize() override;
  void Destroy() override;

  ResourceFile Pack(const AnimationClipResource& resource,
                    CompressionMode compression_mode) override;
  void Unpack(const ResourceFile& file, ResourceLifeSpan life_span,
              AnimationClipResource* resource) override;

 private:
  inline static constexpr usize kDefaultAllocatorCapacity_{1024};
  memory::FiberFreeListAllocator anim_allocator_{};
};
}  // namespace resource
}  // namespace comet

#endif  // COMET_COMET_RESOURCE_HANDLER_ANIMATION_RESOURCE_HANDLER_H_
