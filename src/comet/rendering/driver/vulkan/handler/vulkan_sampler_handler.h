// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SAMPLER_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SAMPLER_HANDLER_H_

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/shared_instance_registry.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/type/vulkan_sampler_type.h"
#include "comet/rendering/rendering_handle.h"

namespace comet {
namespace rendering {
namespace vk {
using SamplerHandlerDescr = HandlerDescr;

class SamplerHandler : public Handler {
 public:
  SamplerHandler() = delete;
  explicit SamplerHandler(const SamplerHandlerDescr& descr);
  SamplerHandler(const SamplerHandler&) = delete;
  SamplerHandler(SamplerHandler&&) = delete;
  SamplerHandler& operator=(const SamplerHandler&) = delete;
  SamplerHandler& operator=(SamplerHandler&&) = delete;
  ~SamplerHandler() override = default;

  SamplerHandle GetOrGenerate(const SamplerDescr& descr);
  void Destroy(SamplerHandle handle);

  const Sampler* Get(SamplerHandle handle) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  Sampler* Get(SamplerHandle handle);

  Sampler* GenerateSampler(SamplerKey key, const SamplerDescr& descr);
  void DestroySampler(Sampler* sampler);

  memory::PlatformAllocator cache_allocator_{memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator allocator_{sizeof(Sampler), 256,
                                            memory::kEngineMemoryTagRendering};

  SharedInstanceRegistry<SamplerKey, SamplerTag, Sampler> samplers_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SAMPLER_HANDLER_H_