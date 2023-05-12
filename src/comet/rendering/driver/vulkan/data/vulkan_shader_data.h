// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_

#include "comet_precompile.h"

#include "vulkan/vulkan.h"

#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/data/vulkan_texture_map.h"

namespace comet {
namespace rendering {
namespace vk {
// One for global descriptor sets, the other for instance ones.
constexpr auto kDescriptorSetMaxLayoutCount{2};
// One for non-sampler uniforms, the other for samplers.
constexpr auto kDescriptorBindingCount{2};

using ShaderUniformLocation = u16;
constexpr auto kInvalidShaderUniformLocation{
    static_cast<ShaderUniformLocation>(-1)};

using ShaderUniformIndex = u16;
constexpr auto kInvalidShaderUniformIndex{static_cast<ShaderUniformIndex>(-1)};

struct ShaderUniform {
  sptrdiff offset{0};
  ShaderUniformSize size{kInvalidShaderUniformSize};
  ShaderUniformLocation location{kInvalidShaderUniformLocation};
  ShaderUniformIndex index{kInvalidShaderUniformIndex};
  ShaderUniformType type{ShaderUniformType::Unknown};
  ShaderUniformScope scope{ShaderUniformScope::Unknown};
};

struct ShaderUniformIndices {
  ShaderUniformIndex projection{kInvalidShaderUniformIndex};
  ShaderUniformIndex view{kInvalidShaderUniformIndex};
  ShaderUniformIndex ambient_color{kInvalidShaderUniformIndex};
  ShaderUniformIndex view_pos{kInvalidShaderUniformIndex};
  ShaderUniformIndex diffuse_color{kInvalidShaderUniformIndex};
  ShaderUniformIndex diffuse_map{kInvalidShaderUniformIndex};
  ShaderUniformIndex specular_map{kInvalidShaderUniformIndex};
  ShaderUniformIndex normal_map{kInvalidShaderUniformIndex};
  ShaderUniformIndex shininess{kInvalidShaderUniformIndex};
  ShaderUniformIndex model{kInvalidShaderUniformIndex};
};

constexpr auto kShaderDescriptorSetGlobalIndex{0};
constexpr auto kShaderDescriptorSetInstanceIndex{1};

using BindingIndex = u8;
constexpr auto kInvalidBindingIndex{static_cast<BindingIndex>(-1)};

struct DescriptorSetLayoutBinding {
  std::array<VkDescriptorSetLayoutBinding, kDescriptorBindingCount> bindings{};
  BindingIndex sampler_binding_index{kInvalidBindingIndex};
  u32 binding_count{0};
};

struct DescriptorSetLayoutBindings {
  std::array<DescriptorSetLayoutBinding, kDescriptorSetMaxLayoutCount> list{};
  u8 count{0};
};

struct ShaderDescr {
  std::string resource_path{};
  const RenderPass* render_pass{nullptr};
};

constexpr auto kMaxMaterialInstances{1024};

struct ShaderUniformData {
  std::vector<VkDescriptorSet> descriptor_set_handles{};
  std::vector<const TextureMap*> texture_maps{};
  FrameIndex update_frame{kInvalidFrameIndex};
  VkDescriptorPool descriptor_pool_handle{VK_NULL_HANDLE};
};

struct ShaderUniformBufferObjectData {
  u32 uniform_count{0};
  u32 sampler_count{0};
  uindex ubo_size{0};
  sptrdiff ubo_stride{0};
  sptrdiff ubo_offset{0};
};

using MaterialInstanceId = gid::Gid;
constexpr auto kInvalidMaterialInstanceId{gid::kInvalidId};

struct MaterialInstance {
  MaterialInstanceId id{kInvalidMaterialInstanceId};
  sptrdiff offset{0};
  ShaderUniformData uniform_data{};
};

struct MaterialInstances {
  std::vector<MaterialInstance> list{};
  std::vector<MaterialInstanceId> ids{};
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_
