// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SHADER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SHADER_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/core/engine.h"
#include "comet/rendering/driver/vulkan/vulkan_debug.h"
#include "comet/resource/resource_manager.h"
#include "comet/resource/shader_resource.h"

namespace comet {
namespace rendering {
namespace vk {
struct VulkanShaderModule {
  uindex code_size{0};
  const u32* code{nullptr};
  VkShaderModule handle{VK_NULL_HANDLE};
};

struct VulkanShaderModuleDescr {
  VkShaderStageFlagBits stage_bits{VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};
  std::string shader_path;
};

struct VulkanShaderData {};

template <typename ShaderResourcePath>
VulkanShaderModule CreateShaderModule(VkDevice device,
                                      ShaderResourcePath&& shader_path) {
  const auto* shader_resource{
      Engine::Get().GetResourceManager().Load<resource::ShaderResource>(
          std::forward<ShaderResourcePath>(shader_path))};

  VulkanShaderModule shader_module{
      shader_resource->data.size(),
      reinterpret_cast<const u32*>(shader_resource->data.data())};

  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_module.code_size;
  create_info.pCode = shader_module.code;

  COMET_CHECK_VK(vkCreateShaderModule(device, &create_info, nullptr,
                                      &shader_module.handle),
                 "Failed to create shader module!");
  return shader_module;
}

struct VulkanShaderStage {
  VkShaderStageFlagBits stage_bits{VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM};
  VulkanShaderModule shader_module{};
};

struct VulkanResolvedBinding {
  u32 set{0};
  u32 binding{0};
  VkDescriptorType type{VK_DESCRIPTOR_TYPE_MAX_ENUM};
};

constexpr auto kDescriptorSetLayoutCount{4};

struct VulkanShaderPass {
  VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
  VkPipeline pipeline{VK_NULL_HANDLE};
  std::array<VkDescriptorSetLayout, kDescriptorSetLayoutCount>
      descriptor_set_layouts;
  std::array<stringid::StringId, kDescriptorSetLayoutCount>
      descriptor_set_hashes;
  std::vector<VulkanShaderStage> stages;
  std::unordered_map<stringid::StringId, VulkanResolvedBinding> bindings;

  void AddStage(VulkanShaderModule shader_module,
                VkShaderStageFlagBits stage_bits);
  void ReflectLayout(VkDevice device);
};

class VulkanShaderHandler {
 public:
  VulkanShaderHandler() = default;
  VulkanShaderHandler(const VulkanShaderHandler&) = delete;
  VulkanShaderHandler(VulkanShaderHandler&&) = delete;
  VulkanShaderHandler& operator=(const VulkanShaderHandler&) = delete;
  VulkanShaderHandler& operator=(VulkanShaderHandler&&) = delete;
  ~VulkanShaderHandler() = default;

  void Initialize();
  void Destroy();

  template <typename ShaderResourcePath>
  VulkanShaderModule Get(ShaderResourcePath&& path) {
    const auto it{shader_module_cache_.find(path)};

    if (it != shader_module_cache_.end()) {
      return it->second;
    }

    const auto result{shader_module_cache_.emplace(
        path,
        CreateShaderModule(device_, std::forward<ShaderResourcePath>(path)))};

    COMET_ASSERT(result.second, "Could not create shader module!");
    return result.first->second;
  }

  void SetDevice(VkDevice device) noexcept;
  VkDevice GetDevice() const noexcept;

 private:
  std::unordered_map<std::string, VulkanShaderModule> shader_module_cache_;
  VkDevice device_{VK_NULL_HANDLE};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_VULKAN_SHADER_H_
