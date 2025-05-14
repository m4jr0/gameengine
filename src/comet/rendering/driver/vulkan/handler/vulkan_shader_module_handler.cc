// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"

#include "vulkan_shader_module_handler.h"

#include <type_traits>

#include "comet/core/memory/allocator/allocator.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader_module_resource.h"

namespace comet {
namespace rendering {
namespace vk {
ShaderModuleHandler::ShaderModuleHandler(const ShaderModuleHandlerDescr& descr)
    : Handler{descr} {}

void ShaderModuleHandler::Initialize() {
  Handler::Initialize();
  allocator_.Initialize();
  shader_modules_ = Map<ShaderModuleId, ShaderModule*>{&allocator_};
}

void ShaderModuleHandler::Shutdown() {
  for (auto& it : shader_modules_) {
    Destroy(it.value, true);
  }

  shader_modules_.Destroy();
  allocator_.Destroy();
  Handler::Shutdown();
}

const ShaderModule* ShaderModuleHandler::Generate(
    CTStringView shader_module_path) {
  const auto* shader_module_resource{
      resource::ResourceManager::Get().Load<resource::ShaderModuleResource>(
          shader_module_path)};

  COMET_ASSERT(shader_module_resource != nullptr,
               "Shader module resource is null!");

  auto* shader_module{allocator_.AllocateOneAndPopulate<ShaderModule>()};
  shader_module->id = shader_module_resource->id;
  shader_module->code =
      reinterpret_cast<const u32*>(shader_module_resource->data.GetData());
  shader_module->code_size = shader_module_resource->data.GetSize();

  COMET_ASSERT(shader_module->code_size > 0, "Shader module resource #",
               shader_module_resource->id, " is empty!");

  shader_module->type =
      GetVulkanType(shader_module_resource->descr.shader_type);

  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_module->code_size;
  create_info.pCode = shader_module->code;

  COMET_CHECK_VK(vkCreateShaderModule(context_->GetDevice(), &create_info,
                                      nullptr, &shader_module->handle),
                 "Failed to create shader module!");

  return shader_modules_.Emplace(shader_module->id, shader_module).value;
}

const ShaderModule* ShaderModuleHandler::Get(
    ShaderModuleId shader_module_id) const {
  auto* shader_module{TryGet(shader_module_id)};
  COMET_ASSERT(shader_module != nullptr,
               "Requested shader module does not exist: ",
               COMET_STRING_ID_LABEL(shader_module_id), "!");
  return shader_module;
}

const ShaderModule* ShaderModuleHandler::TryGet(
    ShaderModuleId shader_module_id) const {
  auto shader_module{shader_modules_.TryGet(shader_module_id)};

  if (shader_module == nullptr) {
    return nullptr;
  }

  return *shader_module;
}

const ShaderModule* ShaderModuleHandler::GetOrGenerate(CTStringView path) {
  const auto* shader_module{TryGet(COMET_STRING_ID(path))};

  if (shader_module != nullptr) {
    return shader_module;
  }

  return Generate(path);
}

void ShaderModuleHandler::Destroy(ShaderModuleId shader_module_id) {
  Destroy(Get(shader_module_id), false);
}

void ShaderModuleHandler::Destroy(ShaderModule* shader_module) {
  Destroy(shader_module, false);
}

VkShaderStageFlagBits ShaderModuleHandler::GetVulkanType(
    ShaderModuleType module_type) {
  switch (module_type) {
    case ShaderModuleType::Compute:
      return VK_SHADER_STAGE_COMPUTE_BIT;
    case ShaderModuleType::Vertex:
      return VK_SHADER_STAGE_VERTEX_BIT;
    case ShaderModuleType::Fragment:
      return VK_SHADER_STAGE_FRAGMENT_BIT;
    default:
      COMET_ASSERT(
          false, "Unknown shader module type: ",
          static_cast<std::underlying_type_t<ShaderModuleType>>(module_type),
          "!");
  }

  return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}

ShaderModule* ShaderModuleHandler::Get(ShaderModuleId shader_module_id) {
  auto* shader_module{TryGet(shader_module_id)};
  COMET_ASSERT(shader_module != nullptr,
               "Requested shader module does not exist: ",
               COMET_STRING_ID_LABEL(shader_module_id), "!");
  return shader_module;
}

ShaderModule* ShaderModuleHandler::TryGet(ShaderModuleId shader_module_id) {
  auto** shader_module{shader_modules_.TryGet(shader_module_id)};

  if (shader_module == nullptr) {
    return nullptr;
  }

  return *shader_module;
}

void ShaderModuleHandler::Destroy(ShaderModule* shader_module,
                                  bool is_destroying_handler) {
  if (shader_module->handle != VK_NULL_HANDLE) {
    vkDestroyShaderModule(context_->GetDevice(), shader_module->handle,
                          VK_NULL_HANDLE);
  }

  if (!is_destroying_handler) {
    shader_modules_.Remove(shader_module->id);
  }

  allocator_.Deallocate(shader_module);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet
