// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RUNTIME_RESOURCE_ANIMATION_ANIMATION_RESOURCE_HANDLER_H_
#define COMET_RUNTIME_RESOURCE_ANIMATION_ANIMATION_RESOURCE_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/data/animation/animation_id.h"
#include "comet/data/resource/animation/animation_resource.h"
#include "comet/data/resource/common.h"
#include "comet/data/resource/resource.h"
#include "comet/data/resource/resource_file.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/runtime/resource/animation/animation_resource_handle.h"
#include "comet/runtime/resource/resource_handler.h"

namespace comet {
namespace resource {
namespace internal {
template <typename T>
void WriteValue(u8* buffer, usize& cursor, const T& value) {
  static_assert(std::is_trivially_copyable_v<T>);

  memory::CopyMemory(&buffer[cursor], &value, sizeof(T));
  cursor += sizeof(T);
}

template <typename T>
void ReadValue(const u8* buffer, usize& cursor, T& value) {
  static_assert(std::is_trivially_copyable_v<T>);

  memory::CopyMemory(&value, &buffer[cursor], sizeof(T));
  cursor += sizeof(T);
}
}  // namespace internal

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

#endif  // COMET_RUNTIME_RESOURCE_ANIMATION_ANIMATION_RESOURCE_HANDLER_H_