// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_shader_module.h"
#include "comet/rendering/driver/vulkan/handler/vulkan_handler.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/shader_module_resource.h"

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

  void Shutdown() override;

  const ShaderModule* Generate(const schar* shader_module_path);
  const ShaderModule* Generate(const std::string& shader_module_path);
  const ShaderModule* Get(ShaderModuleId shader_module_id) const;
  const ShaderModule* TryGet(ShaderModuleId shader_module_id) const;
  const ShaderModule* GetOrGenerate(const schar* path);
  const ShaderModule* GetOrGenerate(const std::string& path);
  void Destroy(ShaderModuleId shader_module_id);
  void Destroy(ShaderModule& shader_module);

 private:
  static VkShaderStageFlagBits GetVulkanType(ShaderModuleType module_type);

  ShaderModule* Get(ShaderModuleId shader_module_id);
  ShaderModule* TryGet(ShaderModuleId shader_module_id);
  void Destroy(ShaderModule& shader_module, bool is_destroying_handler);

  std::unordered_map<ShaderModuleId, ShaderModule> shader_modules_{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_
