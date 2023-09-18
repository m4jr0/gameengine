// Copyright 2023 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_

#include "comet_precompile.h"

#include "glad/glad.h"

#include "comet/rendering/driver/opengl/data/opengl_frame.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_module.h"
#include "comet/rendering/driver/opengl/data/opengl_texture_map.h"
#include "comet/rendering/rendering_common.h"

namespace comet {
namespace rendering {
namespace gl {
using UniformBufferHandle = GLuint;
constexpr auto kInvalidUniformBufferHandle{0};

using ShaderUniformLocation = u16;
constexpr auto kInvalidShaderUniformLocation{
    static_cast<ShaderUniformLocation>(-1)};

using ShaderUniformIndex = u16;
constexpr auto kInvalidShaderUniformIndex{static_cast<ShaderUniformIndex>(-1)};

struct ShaderUniform {
  sptrdiff offset{0};
  ShaderUniformSize size{kInvalidShaderUniformSize};
  ShaderUniformLocation location{kInvalidShaderUniformLocation};
  uindex data_index{kInvalidIndex};
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

struct ShaderUniformData {
  std::vector<const TextureMap*> texture_maps{};
  FrameIndex update_frame{kInvalidFrameIndex};
};

constexpr auto kMaxMaterialInstances{1024};

struct ShaderUniformBufferObjectData {
  u32 uniform_count{0};
  u32 sampler_count{0};
  u32 uniform_block_index{GL_INVALID_VALUE};
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

using ShaderId = stringid::StringId;
constexpr auto kInvalidShaderId{static_cast<ShaderId>(-1)};

using ShaderHandle = u32;
constexpr auto kInvalidShaderHandle{0};

struct ShaderDescr {
  TString resource_path{};
};

struct Shader {
  bool is_wireframe{false};
  u8 local_uniform_count{0};
  GLenum cull_mode{GL_INVALID_VALUE};
  UniformBufferHandle ubo_handle{kInvalidUniformBufferHandle};
  ShaderId id{kInvalidShaderId};
  sptrdiff bound_ubo_offset{0};
  MaterialInstanceId bound_instance_index{kInvalidMaterialInstanceId};
  ShaderUniformData global_uniform_data{};
  ShaderUniformBufferObjectData global_ubo_data{};
  ShaderUniformBufferObjectData instance_ubo_data{};
  MaterialInstances instances{};
  ShaderHandle handle{kInvalidShaderHandle};
  std::vector<ShaderUniform> uniforms{};
  ShaderUniformIndices uniform_indices{};
  std::vector<const ShaderModule*> modules{};
};
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_
