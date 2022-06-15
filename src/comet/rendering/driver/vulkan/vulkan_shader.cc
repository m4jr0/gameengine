// Copyright 2022 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#include "vulkan_shader.h"

#include "spirv_reflect.h"

#include "comet/rendering/driver/vulkan/vulkan_descriptor.h"
#include "comet/rendering/driver/vulkan/vulkan_initializers.h"

namespace comet {
namespace rendering {
namespace vk {
void VulkanShaderPass::AddStage(VulkanShaderModule shader_module,
                                VkShaderStageFlagBits stage_bits) {
  stages.emplace_back(VulkanShaderStage{stage_bits, shader_module});
}

// >:3
struct DescriptorSetLayoutData {
  uint32_t set_number;
  VkDescriptorSetLayoutCreateInfo create_info;
  std::vector<VkDescriptorSetLayoutBinding> bindings;
};

void VulkanShaderPass::ReflectLayout(VkDevice device) {
  std::vector<DescriptorSetLayoutData> set_layouts;
  std::vector<VkPushConstantRange> push_constant_ranges;

  for (auto& stage : stages) {
    SpvReflectShaderModule module{};
    auto& shader_module{stage.shader_module};

    auto result{spvReflectCreateShaderModule(
        shader_module.code_size * sizeof(u32), shader_module.code, &module)};
    COMET_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS,
                 "Could not create SPV reflect shader module!");

    u32 descriptor_count;
    result =
        spvReflectEnumerateDescriptorSets(&module, &descriptor_count, nullptr);
    COMET_ASSERT(
        result == SPV_REFLECT_RESULT_SUCCESS,
        "Could not get descriptor sets count with SPV reflect shader module!");

    std::vector<SpvReflectDescriptorSet*> descriptor_sets{descriptor_count};
    result = spvReflectEnumerateDescriptorSets(&module, &descriptor_count,
                                               descriptor_sets.data());
    COMET_ASSERT(
        result == SPV_REFLECT_RESULT_SUCCESS,
        "Could not enumerate descriptor sets with SPV reflect shader module!");

    for (uindex descriptor_index{0}; descriptor_index < descriptor_sets.size();
         ++descriptor_index) {
      const auto& descriptor_set{*descriptor_sets[descriptor_index]};
      DescriptorSetLayoutData layout_data{};
      const auto binding_count{descriptor_set.binding_count};
      layout_data.bindings.resize(binding_count);

      for (u32 binding_index{0}; binding_index < binding_count;
           ++binding_index) {
        const auto& binding{*descriptor_set.bindings[binding_index]};
        auto& layout_binding{layout_data.bindings[binding_index]};

        layout_binding.binding = binding.binding;
        layout_binding.descriptorType =
            static_cast<VkDescriptorType>(binding.descriptor_type);
        layout_binding.stageFlags =
            static_cast<VkShaderStageFlagBits>(module.shader_stage);
        layout_binding.descriptorCount = 1;

        // >:3 TMP.
        std::array<const char*, 2> override_symbols = {"sceneData",
                                                       "cameraData"};

        std::array<VkDescriptorType, 2> override_descriptor_types = {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC};

        for (u32 override_index{0}; override_index < 2; ++override_index) {
          if (std::strcmp(binding.name, override_symbols[override_index]) ==
              0) {
            layout_binding.descriptorType =
                override_descriptor_types[override_index];
          }
        }
        // >:3 TMP.

        for (u32 dim_index{0}; dim_index < binding.array.dims_count;
             ++dim_index) {
          layout_binding.descriptorCount *= binding.array.dims[dim_index];
        }

        COMET_ASSERT(
            binding.name != nullptr,
            "Binding name is null! Check your shader compilation options.");

        bindings[COMET_STRING_ID(binding.name)] = {
            binding.set, layout_binding.binding, layout_binding.descriptorType};
      }

      layout_data.set_number = descriptor_set.set;
      layout_data.create_info.sType =
          VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      layout_data.create_info.bindingCount = descriptor_set.binding_count;
      layout_data.create_info.pBindings = layout_data.bindings.data();
      set_layouts.push_back(layout_data);
    }

    u32 push_constant_count;
    result = spvReflectEnumeratePushConstantBlocks(
        &module, &push_constant_count, nullptr);
    COMET_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS,
                 "Cound not get push constant block count!");

    if (push_constant_count != 0) {
      std::vector<SpvReflectBlockVariable*> push_constants(push_constant_count);
      result = spvReflectEnumeratePushConstantBlocks(
          &module, &push_constant_count, push_constants.data());
      COMET_ASSERT(result == SPV_REFLECT_RESULT_SUCCESS,
                   "Cound not get push constant blocks!");

      VkPushConstantRange range{
          static_cast<VkShaderStageFlags>(stage.stage_bits),
          push_constants[0]->offset, push_constants[0]->size};

      push_constant_ranges.emplace_back(range);
    }

    spvReflectDestroyShaderModule(&module);
  }

  std::array<DescriptorSetLayoutData, kDescriptorSetLayoutCount>
      merged_set_layouts{};

  for (uindex i{0}; i < kDescriptorSetLayoutCount; ++i) {
    auto& merged_set_layout{merged_set_layouts[i]};

    merged_set_layout.set_number = i;
    merged_set_layout.create_info.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    std::unordered_map<int, VkDescriptorSetLayoutBinding> set_layout_bindings;

    for (auto& set_layout : set_layouts) {
      if (set_layout.set_number != i) {
        continue;
      }

      for (auto& binding : set_layout.bindings) {
        auto it{set_layout_bindings.find(binding.binding)};

        if (it == set_layout_bindings.end()) {
          set_layout_bindings[binding.binding] = binding;
        } else {
          set_layout_bindings[binding.binding].stageFlags |= binding.stageFlags;
        }
      }
    }

    for (auto& [binding_id, binding] : set_layout_bindings) {
      merged_set_layout.bindings.push_back(binding);
    }

    std::sort(
        merged_set_layout.bindings.begin(), merged_set_layout.bindings.end(),
        [](VkDescriptorSetLayoutBinding& a, VkDescriptorSetLayoutBinding& b) {
          return a.binding < b.binding;
        });

    merged_set_layout.create_info.bindingCount =
        static_cast<u32>(merged_set_layout.bindings.size());
    merged_set_layout.create_info.pBindings = merged_set_layout.bindings.data();
    merged_set_layout.create_info.flags = 0;
    merged_set_layout.create_info.pNext = 0;

    if (merged_set_layout.create_info.bindingCount > 0) {
      descriptor_set_hashes[i] = GetVulkanDescriptorSetLayoutCreateInfoHash(
          merged_set_layout.create_info);

      vkCreateDescriptorSetLayout(device, &merged_set_layout.create_info,
                                  VK_NULL_HANDLE, &descriptor_set_layouts[i]);
    } else {
      descriptor_set_hashes[i] = 0;
      descriptor_set_layouts[i] = VK_NULL_HANDLE;
    }
  }

  auto mesh_pipeline_layout_info{init::GetPipelineLayoutCreateInfo()};

  mesh_pipeline_layout_info.pPushConstantRanges = push_constant_ranges.data();
  mesh_pipeline_layout_info.pushConstantRangeCount =
      static_cast<u32>(push_constant_ranges.size());

  std::array<VkDescriptorSetLayout, kDescriptorSetLayoutCount>
      compressed_layouts{};
  uindex layout_count{0};
  for (uindex i{0}; i < kDescriptorSetLayoutCount; ++i) {
    if (descriptor_set_layouts[i] != VK_NULL_HANDLE) {
      compressed_layouts[layout_count++] = descriptor_set_layouts[i];
    }
  }

  mesh_pipeline_layout_info.setLayoutCount = layout_count;
  mesh_pipeline_layout_info.pSetLayouts = compressed_layouts.data();

  vkCreatePipelineLayout(device, &mesh_pipeline_layout_info, VK_NULL_HANDLE,
                         &pipeline_layout);
}

void VulkanShaderHandler::Initialize() {}

void VulkanShaderHandler::Destroy() {}

void VulkanShaderHandler::SetDevice(VkDevice device) noexcept {
  device_ = device;
}

VkDevice VulkanShaderHandler::GetDevice() const noexcept { return device_; }
}  // namespace vk
}  // namespace rendering
}  // namespace comet
