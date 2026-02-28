// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_buffer.h"
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
  u16 ref_count{0};
  CullMode cull_mode{CullMode::Unknown};
  PrimitiveTopology topology{PrimitiveTopology::Unknown};
  ShaderId id{kInvalidShaderId};
  VertexAttributeStride vertex_attribute_stride{0};
  sptrdiff bound_ubo_offset{0};
  MaterialInstanceId bound_instance_index{kInvalidMaterialInstanceId};
  ShaderUniformData global_uniform_data{};
  ShaderStorageData storage_data{};
  ShaderUniformBufferObjectData global_ubo_data{};
  ShaderUniformBufferObjectData instance_ubo_data{};
  VkDescriptorPool descriptor_pool_handle{VK_NULL_HANDLE};
  const RenderPass* render_pass{nullptr};
  const Pipeline* graphics_pipeline{nullptr};
  const Pipeline* compute_pipeline{nullptr};
  MaterialInstances instances{};
  DescriptorSetLayoutBindings layout_bindings{};
  StaticArray<VkDescriptorSetLayout, kDescriptorSetMaxLayoutCount>
      layout_handles{};
  Array<Buffer> uniform_buffers{};
  Array<VkVertexInputAttributeDescription> vertex_attributes{};
  Array<ShaderUniform> uniforms{};
  Array<ShaderConstant> constants{};
  Array<ShaderStorage> storages{};
  ShaderDescriptorSetIndices descriptor_set_indices{};
  ShaderUniformIndices uniform_indices{};
  ShaderConstantIndices constant_indices{};
  ShaderStorageIndices storage_indices{};
  Array<const ShaderModule*> modules{};
  Array<VkPushConstantRange> push_constant_ranges{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_H_