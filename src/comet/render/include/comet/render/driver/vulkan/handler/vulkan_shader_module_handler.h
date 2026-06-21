// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_
#define COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/runtime/memory/allocator/free_list_allocator.h"
#include "comet/core/memory/memory.h"
#include "comet/runtime/memory/memory_tag.h"
#include "comet/runtime/shared_instance_registry.h"
#include "comet/render/driver/vulkan/handler/vulkan_handler.h"
#include "comet/render/driver/vulkan/type/vulkan_shader_module.h"
#include "comet/render/render_handle.h"
#include "comet/data/resource/shader/shader_module_resource.h"

namespace comet {
namespace render {
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
  ~ShaderModuleHandler() override = default;

  ShaderModuleHandle GetOrGenerate(
      resource::ShaderModuleResourceId shader_module_resource_id);
  void Destroy(ShaderModuleHandle handle);

  VkShaderModule GetNativeHandle(ShaderModuleHandle handle) const;
  VkShaderStageFlagBits GetStage(ShaderModuleHandle handle) const;

 protected:
  void OnInitialize() override;
  void OnShutdown() override;

 private:
  ShaderModule* GenerateShaderModule(
      const resource::ShaderModuleResource* shader_module_resource);
  void DestroyShaderModule(ShaderModule* shader_module);

  ShaderModule* Get(ShaderModuleHandle handle);
  const ShaderModule* Get(ShaderModuleHandle handle) const;

  memory::PlatformAllocator cache_allocator_{kEngineMemoryTagRender};

  memory::FiberFreeListAllocator allocator_{sizeof(ShaderModule), 256,
                                            kEngineMemoryTagRender};

  SharedInstanceRegistry<resource::ShaderModuleResourceId, ShaderModuleTag,
                         ShaderModule>
      shader_modules_;
};
}  // namespace vk
}  // namespace render
}  // namespace comet

#endif  // COMET_RENDER_DRIVER_VULKAN_HANDLER_VULKAN_SHADER_MODULE_HANDLER_H_