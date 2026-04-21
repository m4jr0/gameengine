// Copyright 2026 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SHADER_TYPE_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SHADER_TYPE_H_

// External. ///////////////////////////////////////////////////////////////////
#include "glad/glad.h"
////////////////////////////////////////////////////////////////////////////////

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_utils.h"
#include "comet/core/type/array.h"
#include "comet/core/type/map.h"
#include "comet/rendering/driver/opengl/type/opengl_descriptor_type.h"
#include "comet/rendering/driver/opengl/type/opengl_pipeline_type.h"
#include "comet/rendering/rendering_handle.h"
#include "comet/rendering/type/rendering_common_type.h"
#include "comet/rendering/type/rendering_shader_type.h"
#include "comet/resource/material/material_resource.h"
#include "comet/resource/shader/shader_resource.h"

namespace comet {
namespace rendering {
namespace gl {
namespace shaderconsts {
constexpr u32 kGlobalSet{0};
constexpr u32 kMaterialSet{1};
constexpr u32 kPassSet{2};

constexpr u32 kGlobalSetOffset{0};
constexpr u32 kMaterialSetOffset{10};
constexpr u32 kPassSetOffset{20};

// OpenGL bridge for Vulkan push constants.
constexpr u32 kPushConstantBindingOffset{50};
}  // namespace shaderconsts

using GlNativeUniformBufferHandle = GLuint;
constexpr auto kInvalidGlNativeUniformBufferHandle{0};

using StorageBufferHandle = GLuint;
constexpr auto kInvalidStorageBufferHandle{0};

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
  ShaderStageFlags stages{kShaderStageFlagBitsNone};

  ShaderBindingIndex index{kInvalidShaderBindingIndex};
  u32 set{0};          // Logical set.
  u32 binding{0};      // Logical binding.
  u32 api_binding{0};  // Flattened OpenGL binding.
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
  ShaderStageFlags stages{kShaderStageFlagBitsNone};
  ShaderOffset offset{0};
  ShaderOffset size{0};
  Array<ShaderField> fields{};
};

using ShaderDescriptorSetIndex = u8;
constexpr auto kInvalidShaderDescriptorSetIndex{
    static_cast<ShaderDescriptorSetIndex>(-1)};

struct DescriptorSetLayoutBinding {
  StaticArray<ShaderBindingIndex, kDescriptorBindingMaxCount> binding_indices{};
  u32 binding_count{0};
};

struct DescriptorSetLayoutBindings {
  StaticArray<DescriptorSetLayoutBinding, kDescriptorSetMaxLayoutCount> list{};
  u32 count{0};
};

struct ShaderImageDescriptor {
  TextureHandle texture_handle{};
  SamplerHandle sampler_handle{};
};

struct ShaderBindingImageRuntimeData {
  ShaderBindingIndex binding_index{kInvalidShaderBindingIndex};
  Array<ShaderImageDescriptor> descriptors{};
};

struct ShaderBindingRuntimeData {
  Array<ShaderBindingImageRuntimeData> image_bindings{};
};

struct ShaderUniformBufferObjectData {
  ShaderStageFlags stages{kShaderStageFlagBitsNone};
  ShaderOffset size{0};
  ShaderOffset stride{0};
  ShaderOffset offset{0};
};

struct ShaderDescriptorSetRuntimeData {
  FrameCount update_frame{kInvalidFrameCount};

  // Logical runtime data for OpenGL.
  // There is no Vulkan-style descriptor set object here, but we keep a parallel
  // abstraction so the higher-level engine model stays consistent.
  Array<ShaderBindingIndex> binding_indices{};
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
  GLuint buffer_handle{kInvalidStorageBufferHandle};
  usize buffer_size{0};
  usize buffer_offset{0};
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

using GlNativeProgramHandle = GLuint;
constexpr auto kInvalidGlNativeProgramHandle{
    static_cast<GlNativeProgramHandle>(0)};

using GlNativeVertexAttributeHandle = GLuint;
constexpr auto kInvalidGlNativeVertexAttributeHandle{0};

struct ShaderKey {
  resource::ShaderResourceId shader_resource_id{};
  RenderPassHandle render_pass_handle{};

  friend constexpr bool operator==(const ShaderKey& lhs,
                                   const ShaderKey& rhs) noexcept = default;
};

using VertexAttributeStride = s32;

struct VertexAttribute {
  GLuint index{0};
  GLint component_count{0};
  GLenum component_type{0};
  GLboolean is_normalized{GL_FALSE};
  GLsizei stride{0};
  const void* offset{nullptr};
};

struct ShaderDescr {
  resource::ShaderResourceId shader_resource_id{};
};

struct Shader {
  ShaderHandle handle{};
  RasterizerState rasterizer{};
  DepthStencilState depth_stencil{};
  GLenum topology{0};
  ShaderVertexLayout vertex_layout{ShaderVertexLayout::None};

  resource::ShaderResourceId id{};
  bool has_vertex_source_binding{false};
  u64 vertex_source_id{0};

  VertexAttributeStride vertex_attribute_stride{0};

  sptrdiff bound_global_ubo_offset{0};
  sptrdiff bound_instance_ubo_offset{0};

  GlNativeProgramHandle compute_program_native_handle{
      kInvalidGlNativeProgramHandle};
  GlNativeProgramHandle graphics_program_native_handle{
      kInvalidGlNativeProgramHandle};

  GlNativeVertexAttributeHandle vertex_attribute_native_handle{
      kInvalidGlNativeVertexAttributeHandle};
  GlNativeUniformBufferHandle uniform_buffer_native_handle{
      kInvalidGlNativeUniformBufferHandle};

  Array<VertexAttribute> vertex_attributes{};
  Array<ShaderBinding> bindings{};
  Array<ShaderPushConstantBlock> push_constant_blocks{};
  Array<ShaderModuleHandle> module_handles{};

  ShaderUniformBufferObjectData global_ubo_data{};
  ShaderUniformBufferObjectData instance_ubo_data{};

  ShaderDescriptorSetRuntimeData global_descriptor_data{};
  ShaderDescriptorSetRuntimeData storage_descriptor_data{};

  MaterialInstances instances{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_TYPE_OPENGL_SHADER_TYPE_H_