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
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace vk {
using ShaderHandle = u32;
constexpr auto kInvalidShaderHandle{static_cast<ShaderHandle>(-1)};

using VertexAttributeStride = s32;

struct ShaderKey {
  resource::ResourceId shader_id{resource::kInvalidResourceId};
  RenderPassHandle render_pass_handle{kInvalidRenderPassHandle};
};

struct ShaderKeyHashLogic : public MapHashLogic<ShaderKey, ShaderHandle> {
  using EntryPair = typename MapHashLogic<ShaderKey, ShaderHandle>::Value;
  using EntryKey = typename MapHashLogic<ShaderKey, ShaderHandle>::Hashable;

  static const EntryKey& GetHashable(const EntryPair& pair) { return pair.key; }

  static HashValue Hash(const EntryKey& key) {
    HashValue hash{0};
    hash = HashCombine(hash, static_cast<HashValue>(key.shader_id));
    hash = HashCombine(hash, static_cast<HashValue>(key.render_pass_handle));
    return hash;
  }

  static bool AreEqual(const EntryKey& a, const EntryKey& b) {
    return a.shader_id == b.shader_id &&
           a.render_pass_handle == b.render_pass_handle;
  }
};

struct ShaderDescr {
  resource::ResourceId shader_id{resource::kInvalidResourceId};
  RenderPassHandle render_pass_handle{kInvalidRenderPassHandle};
};

struct Shader {
  RasterizerState rasterizer{};
  DepthStencilState depth_stencil{};
  PrimitiveTopology topology{PrimitiveTopology::Unknown};
  ShaderVertexLayout vertex_layout{ShaderVertexLayout::None};

  resource::ResourceId id{resource::kInvalidResourceId};
  ShaderHandle handle{kInvalidShaderHandle};
  u32 ref_count{0};

  VertexAttributeStride vertex_attribute_stride{0};

  sptrdiff bound_global_ubo_offset{0};
  sptrdiff bound_instance_ubo_offset{0};

  RenderPassHandle render_pass_handle{kInvalidRenderPassHandle};
  const Pipeline* graphics_pipeline{nullptr};
  const Pipeline* compute_pipeline{nullptr};

  VkDescriptorPool descriptor_pool_handle{VK_NULL_HANDLE};

  DescriptorSetLayoutBindings layout_bindings{};
  StaticArray<VkDescriptorSetLayout, kDescriptorSetMaxLayoutCount>
      layout_handles{};

  Array<Buffer> uniform_buffers{};
  Array<VkVertexInputAttributeDescription> vertex_attributes{};
  Array<ShaderBinding> bindings{};
  Array<ShaderPushConstantBlock> push_constant_blocks{};
  Array<const ShaderModule*> modules{};
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