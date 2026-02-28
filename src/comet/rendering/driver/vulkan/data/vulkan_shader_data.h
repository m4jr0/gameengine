// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_
#define COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_

// External. ///////////////////////////////////////////////////////////////////
#include <string_view>

#include "vulkan/vulkan.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/gid.h"
#include "comet/rendering/driver/vulkan/data/vulkan_frame.h"
#include "comet/rendering/driver/vulkan/data/vulkan_render_pass.h"
#include "comet/rendering/driver/vulkan/data/vulkan_texture_map.h"

namespace comet {
namespace rendering {
namespace vk {
using ShaderWord = u32;

// One for global descriptor sets, one for instances, and one for buffers
// (SSBOs).
constexpr auto kDescriptorSetMaxLayoutCount{3};
// Two bindings are required for uniforms (samplers and non-samplers), while
// SSBOs can have more.
constexpr auto kDescriptorBindingCount{8};

constexpr auto kUniformNameProjection{"projection"sv};
constexpr auto kUniformNameView{"view"sv};
constexpr auto kUniformNameAmbientColor{"ambientColor"sv};
constexpr auto kUniformNameViewPos{"viewPos"sv};
constexpr auto kUniformNameDiffuseColor{"diffuseColor"sv};
constexpr auto kUniformNameDiffuseMap{"diffuseMap"sv};
constexpr auto kUniformNameSpecularMap{"specularMap"sv};
constexpr auto kUniformNameNormalMap{"normalMap"sv};
constexpr auto kUniformNameShininess{"shininess"sv};

constexpr auto kConstantNameDrawCount{"drawCount"sv};
constexpr auto kConstantNameCount{"count"sv};

constexpr auto kStorageNameProxyLocalDatas{"proxyLocalDatas"sv};
constexpr auto kStorageNameProxyIds{"proxyIds"sv};
constexpr auto kStorageNameProxyInstances{"proxyInstances"sv};
constexpr auto kStorageNameIndirectProxies{"indirectProxies"sv};
constexpr auto kStorageNameWordIndices{"wordIndices"sv};
constexpr auto kStorageNameSourceWords{"sourceWords"sv};
constexpr auto kStorageNameDestinationWords{"destinationWords"sv};
constexpr auto kStorageNameMatrixPalettes{"matrixPalettes"sv};

#ifdef COMET_DEBUG_RENDERING
constexpr auto kStorageNameDebugData{"debugData"sv};
#endif  // COMET_DEBUG_RENDERING

#ifdef COMET_DEBUG_CULLING
constexpr auto kStorageNameDebugAabbs{"debugAabbs"sv};
constexpr auto kStorageNameLineVertices{"lineVertices"sv};
#endif  // COMET_DEBUG_CULLING

using ShaderBindingIndex = u32;
constexpr auto kInvalidShaderBindingIndex{static_cast<ShaderBindingIndex>(-1)};

constexpr ShaderBindingIndex kStorageBindingProxyLocalDatas{0};
constexpr ShaderBindingIndex kStorageBindingProxyIds{1};
constexpr ShaderBindingIndex kStorageBindingProxyInstances{2};
constexpr ShaderBindingIndex kStorageBindingIndirectProxies{3};
constexpr ShaderBindingIndex kStorageBindingWordIndices{4};
constexpr ShaderBindingIndex kStorageBindingSourceWords{5};
constexpr ShaderBindingIndex kStorageBindingDestinationWords{6};
constexpr ShaderBindingIndex kStorageBindingMatrixPalettes{7};

constexpr ShaderBindingIndex kStorageBindingDebugOffset{64};
#ifdef COMET_DEBUG_RENDERING
constexpr ShaderBindingIndex kStorageBindingDebugData{
    kStorageBindingDebugOffset};
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
constexpr ShaderBindingIndex kStorageBindingDebugAabbs{
    kStorageBindingDebugOffset + 1};
constexpr ShaderBindingIndex kStorageBindingLineVertices{
    kStorageBindingDebugOffset + 2};
#endif  // COMET_DEBUG_CULLING

using ShaderUniformLocation = u16;
constexpr auto kInvalidShaderUniformLocation{
    static_cast<ShaderUniformLocation>(-1)};

using ShaderUniformIndex = u16;
constexpr auto kInvalidShaderUniformIndex{static_cast<ShaderUniformIndex>(-1)};

using ShaderConstantIndex = u16;
constexpr auto kInvalidShaderConstantIndex{
    static_cast<ShaderConstantIndex>(-1)};

using ShaderStorageIndex = u16;
constexpr auto kInvalidShaderStorageIndex{static_cast<ShaderStorageIndex>(-1)};

using ShaderOffset = s32;

struct ShaderUniform {
  ShaderOffset offset{0};
  ShaderUniformSize size{kInvalidShaderUniformSize};
  ShaderUniformIndex index{kInvalidShaderUniformIndex};
  ShaderUniformLocation location{kInvalidShaderUniformLocation};
  ShaderVariableType type{ShaderVariableType::Unknown};
  ShaderUniformScope scope{ShaderUniformScope::Unknown};
  VkShaderStageFlags stages{0};
};

struct ShaderConstant {
  ShaderOffset offset{0};
  ShaderConstantSize size{kInvalidShaderConstantSize};
  ShaderConstantIndex index{kInvalidShaderConstantIndex};
  ShaderVariableType type{ShaderVariableType::Unknown};
  VkShaderStageFlags stages{0};
};

using ShaderStoragePropertySize = u32;
constexpr auto kInvalidShaderStoragePropertySize{
    static_cast<ShaderStoragePropertySize>(-1)};

struct ShaderStorageProperty {
  ShaderStoragePropertySize size{kInvalidShaderStoragePropertySize};
  ShaderVariableType type{ShaderVariableType::Unknown};
};

constexpr usize kMaxShaderStorageLayoutPropertyCount{5};

struct ShaderStorage {
  VkShaderStageFlags stages{0};
  ShaderStorageIndex index{kInvalidShaderStorageIndex};
  ShaderBindingIndex binding{kInvalidShaderBindingIndex};
  usize property_count{0};
  ShaderStorageProperty properties[kMaxShaderStorageLayoutPropertyCount]{};
};

using ShaderDescriptorSetIndex = u8;
constexpr auto kInvalidShaderDescriptorSetIndex{
    static_cast<ShaderDescriptorSetIndex>(-1)};

struct ShaderDescriptorSetIndices {
  ShaderDescriptorSetIndex global{kInvalidShaderDescriptorSetIndex};
  ShaderDescriptorSetIndex instance{kInvalidShaderDescriptorSetIndex};
  ShaderDescriptorSetIndex storage{kInvalidShaderDescriptorSetIndex};
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
};

struct ShaderConstantIndices {
  ShaderConstantIndex draw_count{kInvalidShaderConstantIndex};
  ShaderConstantIndex count{kInvalidShaderConstantIndex};
};

struct ShaderStorageIndices {
  ShaderStorageIndex proxy_local_datas{kInvalidShaderStorageIndex};
  ShaderStorageIndex proxy_ids{kInvalidShaderStorageIndex};
  ShaderStorageIndex proxy_instances{kInvalidShaderStorageIndex};
  ShaderStorageIndex indirect_proxies{kInvalidShaderStorageIndex};
  ShaderStorageIndex word_indices{kInvalidShaderStorageIndex};
  ShaderStorageIndex source_words{kInvalidShaderStorageIndex};
  ShaderStorageIndex destination_words{kInvalidShaderStorageIndex};
  ShaderStorageIndex matrix_palettes{kInvalidShaderStorageIndex};
#ifdef COMET_DEBUG_RENDERING
  ShaderStorageIndex debug_data{kInvalidShaderStorageIndex};
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
  ShaderStorageIndex debug_aabbs{kInvalidShaderStorageIndex};
  ShaderStorageIndex line_vertices{kInvalidShaderStorageIndex};
#endif  // COMET_DEBUG_CULLING
};

struct DescriptorSetLayoutBinding {
  StaticArray<VkDescriptorSetLayoutBinding, kDescriptorBindingCount> bindings{};
  ShaderBindingIndex sampler_binding_index{kInvalidShaderBindingIndex};
  u32 binding_count{0};
};

struct DescriptorSetLayoutBindings {
  StaticArray<DescriptorSetLayoutBinding, kDescriptorSetMaxLayoutCount> list{};
  u8 count{0};
};

struct ShaderDescr {
  TString resource_path{};
  const RenderPass* render_pass{nullptr};
};

constexpr auto kMaxMaterialInstances{1024};

struct ShaderUniformData {
  Array<VkDescriptorSet> descriptor_set_handles{};
  Array<const TextureMap*> texture_maps{};
  FrameIndex update_frame{kInvalidFrameIndex};
  VkDescriptorPool descriptor_pool_handle{VK_NULL_HANDLE};
};

struct ShaderStorageData {
  VkShaderStageFlags stages{0};
  u32 count{0};
  Array<VkDescriptorSet> descriptor_set_handles{};
  VkDescriptorPool descriptor_pool_handle{VK_NULL_HANDLE};
};

struct ShaderUniformBufferObjectData {
  VkShaderStageFlags stages{0};
  u32 uniform_count{0};
  u32 sampler_count{0};
  usize ubo_size{0};
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
  Array<MaterialInstance> list{};
  Array<MaterialInstanceId> ids{};
};

struct ShaderGlobalsUpdate {
  const math::Mat4* projection_matrix{nullptr};
  const math::Mat4* view_matrix{nullptr};
};

struct ShaderConstantsUpdate {
  const u32* draw_count{nullptr};
  const u32* count{nullptr};
};

struct ShaderStoragesUpdate {
  VkBuffer ssbo_proxy_local_datas_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_proxy_ids_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_proxy_instances_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_indirect_proxies_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_word_indices_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_source_words_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_destination_words_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_matrix_palettes_handle{VK_NULL_HANDLE};
#ifdef COMET_DEBUG_RENDERING
  VkBuffer ssbo_debug_data_handle{VK_NULL_HANDLE};
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
  VkBuffer ssbo_debug_aabbs_handle{VK_NULL_HANDLE};
  VkBuffer ssbo_debug_lines_handle{VK_NULL_HANDLE};
#endif  // COMET_DEBUG_CULLING
  VkDeviceSize ssbo_proxy_local_datas_size{0};
  VkDeviceSize ssbo_proxy_ids_size{0};
  VkDeviceSize ssbo_proxy_instances_size{0};
  VkDeviceSize ssbo_indirect_proxies_size{0};
  VkDeviceSize ssbo_word_indices_size{0};
  VkDeviceSize ssbo_source_words_size{0};
  VkDeviceSize ssbo_destination_words_size{0};
  VkDeviceSize ssbo_matrix_palettes_size{0};
#ifdef COMET_DEBUG_RENDERING
  VkDeviceSize ssbo_debug_data_size{0};
#endif  // COMET_DEBUG_RENDERING
#ifdef COMET_DEBUG_CULLING
  VkDeviceSize ssbo_debug_aabbs_size{0};
  VkDeviceSize ssbo_debug_lines_size{0};
#endif  // COMET_DEBUG_CULLING
};
}  // namespace vk
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_VULKAN_DATA_VULKAN_SHADER_DATA_H_
