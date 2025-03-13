// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_MODULE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_MODULE_HANDLER_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/allocator.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/opengl/data/opengl_shader.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_module.h"
#include "comet/rendering/driver/opengl/handler/opengl_handler.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace rendering {
namespace gl {
using ShaderModuleHandlerDescr = HandlerDescr;

class ShaderModuleHandler : public Handler {
 public:
  explicit ShaderModuleHandler(const ShaderModuleHandlerDescr& descr);
  ShaderModuleHandler(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler(ShaderModuleHandler&&) = delete;
  ShaderModuleHandler& operator=(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler& operator=(ShaderModuleHandler&&) = delete;
  virtual ~ShaderModuleHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  const ShaderModule* Generate(CTStringView shader_module_path);
  const ShaderModule* Get(ShaderModuleHandle shader_module_handle) const;
  const ShaderModule* TryGet(ShaderModuleHandle shader_module_handle) const;
  const ShaderModule* GetOrGenerate(CTStringView path);
  void Destroy(ShaderModuleHandle shader_module_handle);
  void Destroy(ShaderModule* shader_module);
  void Attach(const Shader* shader, ShaderModuleHandle shader_module_handle);
  void Attach(const Shader* shader, ShaderModule* shader_module);
  void Detach(const Shader* shader, ShaderModuleHandle shader_module_handle);
  void Detach(const Shader* shader, ShaderModule* shader_module);

 private:
  static GLenum GetOpenGlType(ShaderModuleType module_type);

  ShaderModule* Get(ShaderModuleHandle shader_module_handle);
  ShaderModule* TryGet(ShaderModuleHandle shader_module_handle);
  void Destroy(ShaderModule* shader_module, bool is_destroying_handler);
  ShaderModule* CompileShader(const resource::ShaderModuleResource* resource);

  memory::FiberFreeListAllocator allocator_{sizeof(ShaderModule), 256,
                                            memory::kEngineMemoryTagRendering};
  Map<ShaderModuleHandle, ShaderModule*> shader_modules_{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_HANDLER_OPENGL_SHADER_MODULE_HANDLER_H_
