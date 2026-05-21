// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_SHADER_H_
#define COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_SHADER_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_container.h"
#include "comet/core/hash.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/type/vulkan_buffer.h"
#include "comet/rendering/driver/vulkan/type/vulkan_descriptor.h"
#include "comet/rendering/driver/vulkan/type/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/type/vulkan_pipeline.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/pipeline.h"
#include "comet/rendering/type/shader.h"
#include "comet/resource/material/material_resource.h"

namespace comet {
namespace rendering {
namespace vk {
namespace shaderconsts {
constexpr u32 kGlobalSet{0};
constexpr u32 kMaterialSet{1};
constexpr u32 kPassSet{2};
}  // namespace shaderconsts

using ShaderWord = u32;

constexpr auto kMaxShaderBindings{64};
constexpr auto kMaxShaderFieldsPerBlock{64};
constexpr auto kMaxShaderPushConstantBlocks{8};

using ShaderFieldIndex = u16;
constexpr auto kInvalidShaderFieldIndex{static_cast<ShaderFieldIndex>(-1)};

using ShaderBindingIndex = u16;
constexpr auto kInvalidShaderBindingIndex{static_cast<ShaderBindingIndex>(-1)};

using ShaderPushConstantIndex = u16;
constexpr auto kInvalidShaderPushConstantIndex{
    static_cast<ShaderPushConstantIndex>(-1)};

using ShaderOffset = sptrdiff;

struct ShaderFieldLayoutInfo {
  Alignment alignment{kInvalidAlignment};
  ShaderVariableSize element_size{kInvalidShaderVariableSize};
  ShaderVariableSize aligned_size{kInvalidShaderVariableSize};
  ShaderVariableSize stride{0};
  ShaderVariableSize total_size{kInvalidShaderVariableSize};
};

struct ShaderField {
  ShaderVariableType type{ShaderVariableType::Unknown};

  ShaderOffset offset{0};

  ShaderVariableSize element_size{kInvalidShaderVariableSize};
  ShaderVariableSize aligned_size{kInvalidShaderVariableSize};
  ShaderVariableSize stride{0};
  ShaderVariableSize size{kInvalidShaderVariableSize};

  u32 array_count{1};
};

struct ShaderBinding {
  ShaderBindingType type{ShaderBindingType::Unknown};
  ShaderBindingScope scope{ShaderBindingScope::Unknown};
  ShaderMemoryLayout layout{ShaderMemoryLayout::Unknown};
  VkShaderStageFlags stages{0};

  ShaderBindingIndex index{kInvalidShaderBindingIndex};
  u32 set{0};
  u32 binding{0};
  u32 descriptor_count{1};

  ShaderImageBindingSemantic image_semantic{
      ShaderImageBindingSemantic::Unknown};

  ShaderOffset size{0};
  ShaderOffset stride{0};
  ShaderOffset offset{0};

  Array<ShaderField> fields{};
};

struct ShaderPushConstantBlock {
  ShaderPushConstantIndex index{kInvalidShaderPushConstantIndex};
  VkShaderStageFlags stages{0};
  ShaderOffset offset{0};
  ShaderOffset size{0};
  Array<ShaderField> fields{};
};

using ShaderDescriptorSetIndex = u8;
constexpr auto kInvalidShaderDescriptorSetIndex{
    static_cast<ShaderDescriptorSetIndex>(-1)};

struct DescriptorSetLayoutBinding {
  StaticArray<VkDescriptorSetLayoutBinding, kDescriptorBindingMaxCount>
      bindings{};
  u32 binding_count{0};
};

struct DescriptorSetLayoutBindings {
  StaticArray<DescriptorSetLayoutBinding, kDescriptorSetMaxLayoutCount> list{};
  u32 count{0};
};

struct ShaderImageDescriptor {
  TextureHandle texture_handle{};
  SamplerHandle sampler_handle{};
  VkImageLayout image_layout{VK_IMAGE_LAYOUT_UNDEFINED};
};

struct ShaderBindingImageRuntimeData {
  ShaderBindingIndex binding_index{kInvalidShaderBindingIndex};
  Array<ShaderImageDescriptor> descriptors{};
};

struct ShaderBindingRuntimeData {
  Array<ShaderBindingImageRuntimeData> image_bindings{};
};

struct ShaderUniformBufferObjectData {
  VkShaderStageFlags stages{0};
  ShaderOffset size{0};
  ShaderOffset stride{0};
  ShaderOffset offset{0};
};

struct ShaderDescriptorSetRuntimeData {
  Array<VkDescriptorSet> descriptor_set_handles{};
  VkDescriptorPool descriptor_pool_handle{VK_NULL_HANDLE};
  FrameIndex update_frame{kInvalidFrameIndex};
};

constexpr auto kMaxMaterialInstances{1024};

struct MaterialInstance {
  resource::MaterialResourceId material_resource_id{};
  sptrdiff offset{0};
  ShaderDescriptorSetRuntimeData descriptor_data{};
  ShaderBindingRuntimeData binding_data{};
};

struct MaterialInstances {
  Array<MaterialInstance> list{};
  Map<resource::MaterialResourceId, u32> indices{};
};

struct ShaderBufferFieldUpdate {
  ShaderBindingIndex binding_index{kInvalidShaderBindingIndex};
  ShaderFieldIndex field_index{kInvalidShaderFieldIndex};
  const void* data{nullptr};
  usize size{0};
};

struct ShaderImageBindingUpdate {
  ShaderBindingIndex binding_index{kInvalidShaderBindingIndex};
  const ShaderImageDescriptor* descriptors{nullptr};
  u32 descriptor_count{0};
};

struct ShaderBufferBindingUpdate {
  ShaderBindingIndex binding_index{kInvalidShaderBindingIndex};
  VkBuffer buffer_handle{VK_NULL_HANDLE};
  VkDeviceSize buffer_size{0};
  VkDeviceSize buffer_offset{0};
};

struct ShaderPassUpdate {
  frame::FrameArray<ShaderBufferFieldUpdate>* field_updates{nullptr};
  frame::FrameArray<ShaderImageBindingUpdate>* image_bindings{nullptr};
  frame::FrameArray<ShaderBufferBindingUpdate>* buffer_bindings{nullptr};
};

struct ShaderGlobalUpdate {
  frame::FrameArray<ShaderBufferFieldUpdate>* field_updates{nullptr};
  frame::FrameArray<ShaderImageBindingUpdate>* image_bindings{nullptr};
  frame::FrameArray<ShaderBufferBindingUpdate>* buffer_bindings{nullptr};
};

struct ShaderInstanceUpdate {
  frame::FrameArray<ShaderBufferFieldUpdate>* field_updates{nullptr};
  frame::FrameArray<ShaderImageBindingUpdate>* image_bindings{nullptr};
  frame::FrameArray<ShaderBufferBindingUpdate>* buffer_bindings{nullptr};
};

struct ShaderPushConstantBlockUpdate {
  ShaderPushConstantIndex block_index{kInvalidShaderPushConstantIndex};
  const void* data{nullptr};
  usize size{0};
};

struct ShaderPushConstantsUpdate {
  frame::FrameArray<ShaderPushConstantBlockUpdate>* blocks{nullptr};
};

using VertexAttributeStride = s32;

struct ShaderKey {
  resource::ShaderResourceId shader_resource_id{};
  RenderPassHandle render_pass_handle{};
  PipelineBindType bind_type{PipelineBindType::Graphics};

  friend constexpr bool operator==(const ShaderKey& lhs,
                                   const ShaderKey& rhs) noexcept = default;
};

inline HashValue GenerateHash(const ShaderKey& key) noexcept {
  auto hash{resource::GenerateHash(key.shader_resource_id)};
  hash = HashCombine(hash, GenerateHash(key.render_pass_handle));
  hash = HashCombine(hash, comet::GenerateHash(ToUnderlying(key.bind_type)));
  return hash;
}

struct ShaderDescr {
  resource::ShaderResourceId shader_resource_id{};
  RenderPassHandle render_pass_handle{};
  PipelineBindType bind_type{PipelineBindType::Unknown};
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

  PipelineBindType bind_type{PipelineBindType::Graphics};
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

#endif  // COMET_RENDER_DRIVER_VULKAN_TYPE_VULKAN_SHADER_H_