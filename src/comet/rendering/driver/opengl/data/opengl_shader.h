// Copyright 2025 m4jr0. All Rights Reserved.
// Use of this source code is governed by the MIT
// license that can be found in the LICENSE file.

#ifndef COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_
#define COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_

#include "glad/glad.h"

#include "comet/core/essentials.h"
#include "comet/core/type/array.h"
#include "comet/core/type/string_id.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_data.h"
#include "comet/rendering/driver/opengl/data/opengl_shader_module.h"
#include "comet/rendering/driver/opengl/data/opengl_storage.h"

namespace comet {
namespace rendering {
namespace gl {
using ShaderId = stringid::StringId;
constexpr auto kInvalidShaderId{static_cast<ShaderId>(-1)};

using VertexAttributeHandle = u32;
constexpr auto kInvalidVertexAttributeHandle{0};

using ShaderHandle = u32;
constexpr auto kInvalidShaderHandle{0};

struct Shader {
  bool is_wireframe{false};
  GLenum cull_mode{GL_NONE};
  GLenum topology{GL_NONE};
  ShaderId id{kInvalidShaderId};
  ShaderHandle compute_handle{kInvalidShaderHandle};
  ShaderHandle graphics_handle{kInvalidShaderHandle};
  VertexAttributeHandle vertex_attribute_handle{kInvalidVertexAttributeHandle};
  StorageHandle vertex_buffer_handle{kInvalidStorageHandle};
  StorageHandle index_buffer_handle{kInvalidStorageHandle};
  sptrdiff bound_ubo_offset{0};
  MaterialInstanceId bound_instance_index{kInvalidMaterialInstanceId};
  ShaderUniformData global_uniform_data{};
  ShaderStorageData storage_data{};
  ShaderUniformBufferObjectData global_ubo_data{};
  ShaderUniformBufferObjectData instance_ubo_data{};
  UniformBufferHandle uniform_buffer_handle{kInvalidUniformBufferHandle};
  MaterialInstances instances{};
  Array<ShaderUniform> uniforms{};
  Array<ShaderConstant> constants{};
  Array<ShaderStorage> storages{};
  ShaderUniformBufferIndices uniform_buffer_indices{};
  ShaderUniformIndices uniform_indices{};
  ShaderConstantIndices constant_indices{};
  ShaderStorageIndices storage_indices{};
  Array<const ShaderModule*> modules{};
};

ShaderHandle ResolveHandle(const Shader* shader, ShaderBindType bind_type);
}  // namespace gl
}  // namespace rendering
}  // namespace comet

#endif  // COMET_COMET_RENDERING_DRIVER_OPENGL_DATA_OPENGL_SHADER_H_
