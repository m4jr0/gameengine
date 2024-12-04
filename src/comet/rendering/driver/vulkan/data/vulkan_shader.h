// Copyright 2024 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_H_

#include "vma/vk_mem_alloc.h"
#include "vulkan/vulkan.h"

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/data/vulkan_pipeline.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/driver/vulkan/data/vulkan_shader_module.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace vk {
using ShaderId = stringid::StringId;
constexpr auto kInvalidShaderId{static_cast<ShaderId>(-1)};

using VertexAttributeStride = s32;

struct Shader {
  bool is_wireframe{false};
  CullMode cull_mode{CullMode::Unknown};
  ShaderId id{kInvalidShaderId};
  VertexAttributeStride vertex_attribute_stride{0};
  sptrdiff bound_ubo_offset{0};
  MaterialInstanceId bound_instance_index{kInvalidMaterialInstanceId};
  ShaderUniformData global_uniform_data{};
  ShaderUniformBufferObjectData global_ubo_data{};
  ShaderUniformBufferObjectData instance_ubo_data{};
  Buffer uniform_buffer{};
  VkDescriptorPool descriptor_pool_handle{VK_NULL_HANDLE};
  const RenderPass* render_pass{nullptr};
  const Pipeline* pipeline{nullptr};
  MaterialInstances instances{};
  DescriptorSetLayoutBindings layout_bindings{};
  // One per frame
  std::vector<VkVertexInputAttributeDescription> vertex_attributes{};
  StaticArray<VkDescriptorSetLayout, kDescriptorSetMaxLayoutCount>
      layout_handles{};
  std::vector<ShaderUniform> uniforms{};
  ShaderUniformIndices uniform_indices{};
  std::vector<const ShaderModule*> modules{};
  std::vector<VkPushConstantRange> push_constant_ranges{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_H_