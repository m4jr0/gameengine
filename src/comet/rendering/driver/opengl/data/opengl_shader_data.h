// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_DATA_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_DATA_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/core/frame/frame_packet.h"
#include "comet/core/type/array.h"
#include "comet/math/matrix.h"
#include "comet/rendering/driver/opengl/data/opengl_texture_map.h"
#include "comet/rendering/rendering_common.h"
#include "comet/resource/resource.h"

namespace comet {
namespace rendering {
namespace gl {
using UniformBufferHandle = GLuint;
constexpr auto kInvalidUniformBufferHandle{0};

using StorageBufferHandle = GLuint;
constexpr auto kInvalidStorageBufferHandle{0};

using ShaderWord = u32;

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

using ShaderUniformLocation = u16;
constexpr auto kInvalidShaderUniformLocation{
    static_cast<ShaderUniformLocation>(-1)};

using ShaderConstantLocation = ShaderUniformLocation;
constexpr auto kInvalidShaderConstantLocation{kInvalidShaderUniformLocation};

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
};

struct ShaderConstant {
  ShaderConstantLocation location{kInvalidShaderConstantLocation};
  ShaderConstantSize size{kInvalidShaderConstantSize};
  ShaderConstantIndex index{kInvalidShaderConstantIndex};
  ShaderVariableType type{ShaderVariableType::Unknown};
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
  ShaderStorageIndex index{kInvalidShaderStorageIndex};
  usize property_count{0};
  ShaderStorageProperty properties[kMaxShaderStorageLayoutPropertyCount]{};
};

using ShaderUniformBufferIndex = u8;
constexpr auto kInvalidShaderUniformBufferIndex{
    static_cast<ShaderUniformBufferIndex>(-1)};

struct ShaderUniformBufferIndices {
  ShaderUniformBufferIndex global{kInvalidShaderUniformBufferIndex};
  ShaderUniformBufferIndex instance{kInvalidShaderUniformBufferIndex};
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
};

using BindingIndex = u32;
constexpr auto kInvalidBindingIndex{static_cast<BindingIndex>(-1)};

struct ShaderDescr {
  TString resource_path{};
};

constexpr auto kMaxMaterialInstances{1024};

struct ShaderUniformData {
  Array<const TextureMap*> texture_maps{};
  FrameCount update_frame{kInvalidFrameCount};
};

struct ShaderStorageData {
  u32 count{0};
};

struct ShaderUniformBufferObjectData {
  u32 uniform_count{0};
  u32 sampler_count{0};
  u32 uniform_block_index{GL_INVALID_VALUE};
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
  StorageBufferHandle ssbo_proxy_local_datas_handle{
      kInvalidStorageBufferHandle};
  StorageBufferHandle ssbo_proxy_ids_handle{kInvalidStorageBufferHandle};
  StorageBufferHandle ssbo_proxy_instances_handle{kInvalidStorageBufferHandle};
  StorageBufferHandle ssbo_indirect_proxies_handle{kInvalidStorageBufferHandle};
  StorageBufferHandle ssbo_word_indices_handle{kInvalidStorageBufferHandle};
  StorageBufferHandle ssbo_source_words_handle{kInvalidStorageBufferHandle};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_DATA_H_