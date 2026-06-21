// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_SAMPLER_HANDLER_H_
#define COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_SAMPLER_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/runtime/shared_instance_registry.h"
#include "comet/render/driver/opengl/handler/opengl_handler.h"
#include "comet/render/driver/opengl/type/opengl_sampler.h"
#include "comet/render/render_handle.h"

namespace comet {
namespace render {
namespace gl {
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
  Sampler* GenerateSampler(const SamplerDescr& descr);

  memory::FiberFreeListAllocator allocator_{sizeof(Sampler), 256,
                                            kEngineMemoryTagRender};

  memory::PlatformAllocator cache_allocator_{kEngineMemoryTagRender};
  SharedInstanceRegistry<SamplerKey, SamplerTag, Sampler> samplers_{};
};
}  // namespace gl
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_OPENGL_HANDLER_OPENGL_SAMPLER_HANDLER_H_