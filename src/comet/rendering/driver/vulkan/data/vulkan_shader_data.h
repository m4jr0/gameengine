// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_

// External. ///////////////////////////////////////////////////////////////////
#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/type/array.h"
#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/rendering_type.h"
#include "comet/resource/material_resource.h"

namespace comet {
namespace rendering {
namespace vk {
namespace shaderconsts {
constexpr u32 kGlobalSet{0};
constexpr u32 kMaterialSet{1};
constexpr u32 kPassSet{2};
}  // namespace shaderconsts

using ShaderWord = u32;

constexpr auto kDescriptorSetMaxLayoutCount{8};
constexpr auto kDescriptorBindingMaxCount{32};
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
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_