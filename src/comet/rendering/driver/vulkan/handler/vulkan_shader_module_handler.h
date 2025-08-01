// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_

#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/core/type/map.h"
#include "comet/core/type/tstring.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_module.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
struct ShaderModuleHandlerDescr : HandlerDescr {};

class ShaderModuleHandler : public Handler {
 public:
  ShaderModuleHandler() = delete;
  explicit ShaderModuleHandler(const ShaderModuleHandlerDescr& descr);
  ShaderModuleHandler(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler(ShaderModuleHandler&&) = delete;
  ShaderModuleHandler& operator=(const ShaderModuleHandler&) = delete;
  ShaderModuleHandler& operator=(ShaderModuleHandler&&) = delete;
  virtual ~ShaderModuleHandler() = default;

  void Initialize() override;
  void Shutdown() override;

  const ShaderModule* Generate(CTStringView shader_module_path);
  const ShaderModule* Get(ShaderModuleId shader_module_id) const;
  const ShaderModule* TryGet(ShaderModuleId shader_module_id) const;
  const ShaderModule* GetOrGenerate(CTStringView path);
  void Destroy(ShaderModuleId shader_module_id);
  void Destroy(ShaderModule* shader_module);

 private:
  static VkShaderStageFlagBits GetVulkanType(ShaderModuleType module_type);

  ShaderModule* Get(ShaderModuleId shader_module_id);
  ShaderModule* TryGet(ShaderModuleId shader_module_id);
  void Destroy(ShaderModule* shader_module, bool is_destroying_handler);

  memory::FiberFreeListAllocator allocator_{sizeof(ShaderModule), 256,
                                            memory::kEngineMemoryTagRendering};
  Map<ShaderModuleId, ShaderModule*> shader_modules_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_
