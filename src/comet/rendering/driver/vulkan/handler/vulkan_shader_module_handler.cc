// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

// Precompiled. ////////////////////////////////////////////////////////////////
#include "comet/rendering/comet_rendering_pch.h"
#include "comet_pch.h"
////////////////////////////////////////////////////////////////////////////////

// Header. /////////////////////////////////////////////////////////////////////
#include "vulkan_shader_module_handler.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/memory/allocator/allocator.h"
#include "comet/rendering/driver/vulkan/utils/vulkan_shader_utils.h"
#include "comet/rendering/driver/vulkan/vulkan_context.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader/shader_module_resource.h"

namespace comet {
namespace rendering {
namespace vk {
ShaderModuleHandler::ShaderModuleHandler(const ShaderModuleHandlerDescr& descr)
    : Handler{descr}, shader_modules_{&cache_allocator_, 256} {}

ShaderModuleHandle ShaderModuleHandler::GetOrGenerate(
    resource::ShaderModuleResourceId shader_module_resource_id) {
  COMET_ASSERT(shader_module_resource_id.IsValid(),
               "ShaderModuleHandler::GetOrGenerate",
               "shader module resource id is invalid");

  if (const auto handle{shader_modules_.TryAcquire(shader_module_resource_id)};
      handle) {
    return handle;
  }

  ShaderModuleHandle generated_handle{ShaderModuleHandle::Invalid()};

  auto* shader_module_resource_handler{
      resource::ResourceManager::Get().GetShaderModules()};

  const auto is_loaded{shader_module_resource_handler->WithTemporaryLoad(
      shader_module_resource_id,
      [this, &generated_handle, shader_module_resource_id](
          const resource::ShaderModuleResource* shader_module_resource) {
        auto* shader_module{GenerateShaderModule(shader_module_resource)};
        COMET_ASSERT(shader_module != nullptr,
                     "ShaderModuleHandler::GetOrGenerate",
                     "generated shader module is null",
                     "shader_module_resource_id", shader_module_resource_id);

        generated_handle =
            shader_modules_.Create(shader_module->id, shader_module);
        COMET_ASSERT(generated_handle, "ShaderModuleHandler::GetOrGenerate",
                     "shader module instance creation failed",
                     "shader_module_resource_id", shader_module_resource_id);

        shader_module->handle = generated_handle;
      })};

  return is_loaded ? generated_handle : ShaderModuleHandle::Invalid();
}

void ShaderModuleHandler::Destroy(ShaderModuleHandle handle) {
  auto* shader_module{shader_modules_.Get(handle)};

  if (!shader_modules_.Release(handle)) {
    return;
  }

  DestroyShaderModule(shader_module);
  shader_modules_.Remove(handle);
}

VkShaderModule ShaderModuleHandler::GetNativeHandle(
    ShaderModuleHandle handle) const {
  const auto* shader_module{Get(handle)};
  COMET_ASSERT(shader_module->native_handle != VK_NULL_HANDLE,
               "ShaderModuleHandler::GetNativeHandle",
               "shader module native handle is invalid", "shader_module_handle",
               handle);
  return shader_module->native_handle;
}

VkShaderStageFlagBits ShaderModuleHandler::GetStage(
    ShaderModuleHandle handle) const {
  return Get(handle)->stage;
}

ShaderModule* ShaderModuleHandler::Get(ShaderModuleHandle handle) {
  auto* shader_module{shader_modules_.TryGet(handle)};
  COMET_ASSERT(shader_module != nullptr, "ShaderModuleHandler::Get",
               "shader module not found", "shader_module_handle", handle);
  return shader_module;
}

const ShaderModule* ShaderModuleHandler::Get(ShaderModuleHandle handle) const {
  const auto* shader_module{shader_modules_.TryGet(handle)};
  COMET_ASSERT(shader_module != nullptr, "ShaderModuleHandler::Get",
               "shader module not found", "shader_module_handle", handle);
  return shader_module;
}

void ShaderModuleHandler::OnInitialize() {
  allocator_.Initialize();
  shader_modules_.Initialize();
}

void ShaderModuleHandler::OnShutdown() {
  memory::PlatformAllocator tmp_allocator{memory::kEngineMemoryTagRendering};
  auto handles_to_destroy{Array<ShaderModuleHandle>::WithCapacity(
      &tmp_allocator, shader_modules_.GetLiveCount())};

  shader_modules_.ForEachLive(
      [&handles_to_destroy](ShaderModuleHandle handle, const ShaderModule*) {
        handles_to_destroy.PushLast(handle);
      });

  for (const auto handle : handles_to_destroy) {
    const auto ref_count{shader_modules_.GetRefCount(handle)};

    if (ref_count > 0) {
      COMET_LOG_WARNING(
          LoggerType::Rendering, "ShaderModuleHandler::OnShutdown",
          "forcing shader module destruction", "shader_module_handle", handle,
          "ref_count", ref_count, "shader_module_resource_id",
          shader_modules_.Get(handle)->id);
    }

    auto* shader_module{shader_modules_.Drain(handle)};

    if (shader_module == nullptr) {
      continue;
    }

    COMET_ASSERT(shader_module->handle == handle,
                 "ShaderModuleHandler::OnShutdown",
                 "shader module handle mismatch", "expected_handle", handle,
                 "actual_handle", shader_module->handle,
                 "shader_module_resource_id", shader_module->id);

    DestroyShaderModule(shader_module);
  }

  shader_modules_.Destroy();
  allocator_.Destroy();
}

ShaderModule* ShaderModuleHandler::GenerateShaderModule(
    const resource::ShaderModuleResource* shader_module_resource) {
  COMET_ASSERT(shader_module_resource != nullptr,
               "ShaderModuleHandler::GenerateShaderModule",
               "shader module resource is null");
  COMET_ASSERT(shader_module_resource->data.GetSize() % sizeof(u32) == 0,
               "ShaderModuleHandler::GenerateShaderModule",
               "spir-v bytecode size is not aligned",
               "shader_module_resource_id", shader_module_resource->id,
               "bytecode_size", shader_module_resource->data.GetSize(),
               "alignment", sizeof(u32));

  auto* shader_module{allocator_.AllocateOneAndPopulate<ShaderModule>()};
  shader_module->handle = ShaderModuleHandle::Invalid();
  shader_module->id = shader_module_resource->GetId();
  shader_module->code =
      reinterpret_cast<const u32*>(shader_module_resource->data.GetData());
  shader_module->code_size = shader_module_resource->data.GetSize();

  COMET_ASSERT(shader_module->code_size > 0,
               "ShaderModuleHandler::GenerateShaderModule",
               "shader module resource is empty", "shader_module_resource_id",
               shader_module_resource->id);

  shader_module->stage = GetVkStage(shader_module_resource->descr.stage);

  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.pNext = nullptr;
  create_info.flags = 0;
  create_info.codeSize = shader_module->code_size;
  create_info.pCode = shader_module->code;

  COMET_CHECK_VK(vkCreateShaderModule(context_->GetDevice(), &create_info,
                                      nullptr, &shader_module->native_handle),
                 "ShaderModuleHandler::GenerateShaderModule",
                 "shader module creation failed", "shader_module_resource_id",
                 shader_module->id);

  return shader_module;
}

void ShaderModuleHandler::DestroyShaderModule(ShaderModule* shader_module) {
  COMET_ASSERT(shader_module != nullptr,
               "ShaderModuleHandler::DestroyShaderModule",
               "shader module is null");

  if (shader_module->native_handle != VK_NULL_HANDLE) {
    vkDestroyShaderModule(context_->GetDevice(), shader_module->native_handle,
                          nullptr);
    shader_module->native_handle = VK_NULL_HANDLE;
  }

  shader_module->handle.Invalidate();
  allocator_.Deallocate(shader_module);
}
}  // namespace vk
}  // namespace rendering
}  // namespace comet