// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_MODULE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_MODULE_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/shared_instance_registry.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_module.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using ShaderModuleHandlerDescr = HandlerDescr;

class ShaderModuleHandler : public Handler {
 public:
  ShaderModuleHandler() = delete;
  explicit ShaderModuleHandler(const ShaderModuleHandlerDescr& descr);
  ShaderModuleHandler(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler(ShaderModuleHandler&&) = delete;
  ShaderModuleHandler& operator=(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler& operator=(ShaderModuleHandler&&) = delete;
  ~ShaderModuleHandler() override = default;

  ShaderModuleHandle Generate(
      resource::ShaderModuleResourceId shader_module_resource_id);
  void Destroy(ShaderModuleHandle handle);

  void Attach(const Shader* shader, ShaderModuleHandle handle) const;
  void Detach(const Shader* shader, ShaderModuleHandle handle) const;

  GLenum GetStage(ShaderModuleHandle handle) const;
  ShaderBindType GetBindType(ShaderModuleHandle handle) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  ShaderModule* GenerateShaderModule(
      const resource::ShaderModuleResource* shader_module_resource);
  void DestroyShaderModule(ShaderModule* shader_module);

  ShaderModule* Get(ShaderModuleHandle handle);
  const ShaderModule* Get(ShaderModuleHandle handle) const;

  memory::PlatformAllocator cache_allocator_{memory::kEngineMemoryTagRendering};

  memory::FiberFreeListAllocator allocator_{sizeof(ShaderModule), 256,
                                            memory::kEngineMemoryTagRendering};

  SharedInstanceRegistry<resource::ShaderModuleResourceId, ShaderModuleTag,
                         ShaderModule>
      shader_modules_;
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_MODULE_HANDLER_H_