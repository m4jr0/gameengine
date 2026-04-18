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
#include "comet/rendering/driver/vulkan/data/vulkan_shader_data.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/rendering_type.h"

namespace comet {
namespace rendering {
namespace vk {
using VertexAttributeStride = s32;

struct ShaderKey {
  resource::ShaderResourceId shader_resource_id{};
  RenderPassHandle render_pass_handle{};

  friend constexpr bool operator==(const ShaderKey& lhs,
                                   const ShaderKey& rhs) noexcept = default;
};

constexpr HashValue GenerateHash(const ShaderKey& key) noexcept {
  auto hash{resource::GenerateHash(key.shader_resource_id)};
  hash = HashCombine(hash, GenerateHash(key.render_pass_handle));
  return hash;
}

struct ShaderDescr {
  resource::ShaderResourceId shader_resource_id{};
  RenderPassHandle render_pass_handle{};
};

struct Shader {
  ShaderHandle handle{};
  RasterizerState rasterizer{};
  DepthStencilState depth_stencil{};
  PrimitiveTopology topology{PrimitiveTopology::Unknown};
  ShaderVertexLayout vertex_layout{ShaderVertexLayout::None};

  resource::ShaderResourceId id{};
  VertexAttributeStride vertex_attribute_stride{0};

  sptrdiff bound_global_ubo_offset{0};
  sptrdiff bound_instance_ubo_offset{0};

  RenderPassHandle render_pass_handle{};
  PipelineHandle graphics_pipeline{};
  PipelineHandle compute_pipeline{};

  VkDescriptorPool descriptor_pool_handle{VK_NULL_HANDLE};

  DescriptorSetLayoutBindings layout_bindings{};
  StaticArray<VkDescriptorSetLayout, kDescriptorSetMaxLayoutCount>
      layout_handles{};

  Array<Buffer> uniform_buffers{};
  Array<VkVertexInputAttributeDescription> vertex_attributes{};
  Array<ShaderBinding> bindings{};
  Array<ShaderPushConstantBlock> push_constant_blocks{};
  Array<ShaderModuleHandle> module_handles{};
  Array<VkPushConstantRange> push_constant_ranges{};

  ShaderDescriptorSetRuntimeData global_descriptor_data{};
  ShaderDescriptorSetRuntimeData storage_descriptor_data{};

  ShaderUniformBufferObjectData global_ubo_data{};
  ShaderUniformBufferObjectData instance_ubo_data{};

  MaterialInstances instances{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_H_